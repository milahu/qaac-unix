#ifndef _WIN32UTIL_H
#define _WIN32UTIL_H

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <unistd.h> // realpath
#include <fcntl.h>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <limits.h> // PATH_MAX
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma warning(push)
#pragma warning(disable: 4091)
#pragma warning(pop)
#include "util.h"

namespace win32 {

    // Function to get the number of milliseconds since system startup
    unsigned long GetTickCount()
    {
        struct timespec now;
        if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
            // Handle error
            return 0;
        }
        // Calculate milliseconds since system startup
        return now.tv_sec * 1000 + now.tv_nsec / 1000000;
    }

    class Timer {
        uint32_t m_ticks;
    public:
        Timer() { m_ticks = GetTickCount(); };
        double ellapsed() {
            return (static_cast<double>(GetTickCount()) - m_ticks) / 1000.0;
        }
    };

    void throw_error(const std::string& msg, uint32_t error);

    inline void throw_error(const std::string& msg, uint32_t error)
    {
        throw_error(strutil::us2w(msg), error);
    }

    inline std::string GetFullPathName(const std::string &path) {
        char resolved_path[PATH_MAX];
        if (realpath(path.c_str(), resolved_path) != nullptr) {
            return std::string(resolved_path);
        } else {
            // Handle error, e.g., file not found
            return "";
        }
    }

    inline std::string GetFullPathNameX(const std::string &path)
    {
        return GetFullPathName(path);
    }

    inline std::string PathReplaceExtension(const std::string &path, const std::string &newExtension) {
        // Find the last dot in the path
        size_t dotPos = path.find_last_of('.');
        if (dotPos != std::string::npos) {
            // Replace the substring starting from the dot position with the new extension
            return path.substr(0, dotPos) + "." + newExtension;
        }
        // If no dot is found, just append the new extension
        return path + "." + newExtension;
    }

    inline std::string PathCombineX(const std::string &basedir, const std::string &filename) {
        if (basedir.empty() || basedir.back() == '/') {
            // If basedir is empty or already ends with '/', just concatenate
            return basedir + filename;
        } else {
            // If basedir does not end with '/', add a '/' before concatenating
            return basedir + "/" + filename;
        }
    }

    std::string GetModuleFileNameX(void* module) {
        // the "module" paramter is not supported
        // always return name of this exe
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer));
        if (len == -1) {
            // Error handling: unable to read the symbolic link
            return "";
        }
        buffer[len] = '\0'; // Null-terminate the string
        return std::string(buffer);
    }

    inline bool MakeSureDirectoryPathExistsX(const std::string &path)
    {
        // SHCreateDirectoryEx() doesn't work with relative path
        std::string fullpath = GetFullPathNameX(path);
        std::vector<char> buf(fullpath.begin(), fullpath.end());
        buf.push_back(0);
        char *pos = PathFindFileNameW(buf.data());
        *pos = 0;
        int rc = SHCreateDirectoryExW(nullptr, buf.data(), nullptr);
        return rc == ERROR_SUCCESS;
    }

    inline std::string get_module_directory(HMODULE module=0)
    {
        std::string path = GetModuleFileNameX(module);
        const char *fpos = PathFindFileNameW(path.c_str());
        return path.substr(0, fpos - path.c_str());
    }

    inline std::string prefixed_path(const char *path)
    {
        std::string fullpath = GetFullPathNameX(path);
        if (fullpath.size() < 256)
            return fullpath;
        if (fullpath.size() > 2 && fullpath.substr(0, 2) == "\\\\")
            fullpath.insert(2, "?\\UNC\\");
        else
            fullpath.insert(0, "\\\\?\\");
        return fullpath;
    }

    inline FILE *wfopenx(const char *path, const char *mode)
    {
        std::string fullpath = win32::prefixed_path(path);
        int share = _SH_DENYRW;
        if (std::wcschr(mode, L'r') && !std::wcschr(mode, L'+'))
            share = _SH_DENYWR;
        FILE *fp = _wfsopen(fullpath.c_str(), mode, share);
        if (!fp) {
            if (_doserrno) throw_error(fullpath.c_str(), _doserrno);
            util::throw_crt_error(fullpath);
        }
        return fp;
    }
    inline std::shared_ptr<FILE> fopen(const std::string &path,
                                       const char *mode)
    {
        auto noop_close = [](FILE *){};
        if (path != "-")
            return std::shared_ptr<FILE>(wfopenx(path.c_str(), mode),
                                         std::fclose);
        else if (std::wcschr(mode, L'r'))
            return std::shared_ptr<FILE>(stdin, noop_close);
        else
            return std::shared_ptr<FILE>(stdout, noop_close);
    }

    inline HANDLE get_handle(int fd)
    {
        return reinterpret_cast<HANDLE>(_get_osfhandle(fd));
    }
    inline bool is_seekable(HANDLE fh)
    {
        return GetFileType(fh) == FILE_TYPE_DISK;
    }
    inline bool is_seekable(int fd)
    {
        return is_seekable(get_handle(fd));
    }

    FILE *tmpfile(const char *prefix);

    char *load_with_mmap(const char *path, uint64_t *size);

    int create_named_pipe(const char *path);

    std::string get_dll_version_for_locale(HMODULE hDll, WORD langid);

    bool is_same_file(HANDLE ha, HANDLE hb);

    inline bool is_same_file(int fda, int fdb)
    {
        return is_same_file(get_handle(fda), get_handle(fdb));
    }
}
#endif

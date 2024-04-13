#ifndef _WIN32UTIL_H
#define _WIN32UTIL_H

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <unistd.h> // realpath, access, F_OK, open, close
#include <fcntl.h> // for open, O_CREAT, O_WRONLY, O_RDWR, O_RDONLY
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <limits.h> // PATH_MAX
#include <cstdio>
#include <cerrno> // for errno
#include <sys/file.h> // flock

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

    std::string GetModuleFileNameX(void* module = nullptr) {
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

    inline bool MakeSureDirectoryPathExistsX(const std::string &path) {
        std::string fullpath = path;

        // If the path is relative, convert it to an absolute path
        if (path.front() != '/') {
            fullpath = GetFullPathNameX(path);
        }

        // Find the last directory separator in the path
        size_t lastSlash = fullpath.find_last_of('/');
        if (lastSlash != std::string::npos) {
            // Create the parent directories if they don't exist
            std::string parentPath = fullpath.substr(0, lastSlash);
            if (mkdir(parentPath.c_str(), 0777) == -1 && errno != EEXIST) {
                // Error handling: failed to create parent directories
                return false;
            }
        }

        // Create the final directory if it doesn't exist
        if (mkdir(fullpath.c_str(), 0777) == -1 && errno != EEXIST) {
            // Error handling: failed to create the final directory
            return false;
        }

        return true;
    }

    inline std::string get_module_directory(void* module = nullptr) {
        std::string path = GetModuleFileNameX(module);

        // Find the last directory separator in the path
        const char* lastSlash = std::strrchr(path.c_str(), '/');
        if (lastSlash != nullptr) {
            // Extract the directory path up to the last slash
            return path.substr(0, lastSlash - path.c_str());
        }

        // If no slash is found, return an empty string
        return "";
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

    inline FILE *wfopenx(const char *path, const char *mode) {
        std::string fullpath = path; // No need for any prefix in Unix/Linux
        int share = 0;

        // Determine file access mode and file sharing mode
        if (strchr(mode, 'r') && !strchr(mode, '+')) {
            // Read-only mode
            share = LOCK_SH;
        } else {
            // Write or read/write mode
            share = LOCK_EX;
        }

        // Check if the file exists before opening it
        if (access(fullpath.c_str(), F_OK) == -1) {
            // File does not exist, create it if mode includes write access
            if (strchr(mode, 'w') || strchr(mode, 'a')) {
                int fd = open(fullpath.c_str(), O_CREAT, 0644);
                if (fd == -1) {
                    // Error creating the file
                    // You may want to handle the error differently
                    perror("Error creating file");
                    return nullptr;
                }
                close(fd);
            } else {
                // File does not exist and mode doesn't include write access
                // Return nullptr
                return nullptr;
            }
        }

        // Open the file with the specified mode
        int fd = open(fullpath.c_str(), O_RDWR);
        if (fd == -1) {
            // Error opening the file
            // You may want to handle the error differently
            perror("Error opening file");
            return nullptr;
        }

        // Apply file locking based on the determined sharing mode
        if (flock(fd, share | LOCK_NB) == -1) {
            // Error applying file lock
            // You may want to handle the error differently
            perror("Error applying file lock");
            close(fd);
            return nullptr;
        }

        // Convert file descriptor to FILE pointer
        FILE *fp = fdopen(fd, mode);
        if (!fp) {
            // Error converting file descriptor to FILE pointer
            // You may want to handle the error differently
            perror("Error converting file descriptor to FILE pointer");
            close(fd);
            return nullptr;
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
        else if (std::strchr(mode, 'r'))
            return std::shared_ptr<FILE>(stdin, noop_close);
        else
            return std::shared_ptr<FILE>(stdout, noop_close);
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

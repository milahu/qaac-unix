#ifndef _WIN32UTIL_H
#define _WIN32UTIL_H

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma warning(push)
#pragma warning(disable: 4091)
#include <shlobj.h>
#pragma warning(pop)
#include "util.h"

#define HR(expr) (void)(win32::throwIfError((expr), #expr))

namespace win32 {
    class Timer {
        DWORD m_ticks;
    public:
        Timer() { m_ticks = GetTickCount(); };
        double ellapsed() {
            return (static_cast<double>(GetTickCount()) - m_ticks) / 1000.0;
        }
    };

    void throw_error(const std::wstring& msg, DWORD error);

    inline void throw_error(const std::string& msg, DWORD error)
    {
        throw_error(strutil::us2w(msg), error);
    }

    inline void throwIfError(HRESULT expr, const char *msg)
    {
        if (FAILED(expr)) throw_error(msg, expr);
    }

    inline std::wstring GetFullPathNameX(const std::wstring &path)
    {
        DWORD length = GetFullPathNameW(path.c_str(), 0, 0, 0);
        std::vector<char8_t> vec(length);
        length = GetFullPathNameW(path.c_str(), static_cast<DWORD>(vec.size()),
                                  &vec[0], 0);
        return std::wstring(&vec[0], &vec[length]);
    }

    inline std::wstring PathReplaceExtension(const std::wstring &path,
                                             const char8_t *ext)
    {
        const char8_t *beg = path.c_str();
        const char8_t *end = PathFindExtensionW(beg);
        std::wstring s(beg, end);
        //if (ext[0] != L'.') s.push_back(L'.');
        s += ext;
        return s;
    }

    // XXX: limited to MAX_PATH
    inline std::wstring PathCombineX(const std::wstring &basedir,
                                     const std::wstring &filename)
    {
        char8_t buffer[MAX_PATH];
        PathCombineW(buffer, basedir.c_str(), filename.c_str());
        return buffer;
    }

    inline std::wstring GetModuleFileNameX(HMODULE module)
    {
        std::vector<char8_t> buffer(32);
        DWORD cclen = GetModuleFileNameW(module, &buffer[0],
                                         static_cast<DWORD>(buffer.size()));
        while (cclen >= buffer.size() - 1) {
            buffer.resize(buffer.size() * 2);
            cclen = GetModuleFileNameW(module, &buffer[0],
                                       static_cast<DWORD>(buffer.size()));
        }
        return std::wstring(&buffer[0], &buffer[cclen]);
    }

    inline bool MakeSureDirectoryPathExistsX(const std::wstring &path)
    {
        // SHCreateDirectoryEx() doesn't work with relative path
        std::wstring fullpath = GetFullPathNameX(path);
        std::vector<char8_t> buf(fullpath.begin(), fullpath.end());
        buf.push_back(0);
        char8_t *pos = PathFindFileNameW(buf.data());
        *pos = 0;
        int rc = SHCreateDirectoryExW(nullptr, buf.data(), nullptr);
        return rc == ERROR_SUCCESS;
    }

    inline std::wstring get_module_directory(HMODULE module=0)
    {
        std::wstring path = GetModuleFileNameX(module);
        const char8_t *fpos = PathFindFileNameW(path.c_str());
        return path.substr(0, fpos - path.c_str());
    }

    inline std::wstring prefixed_path(const char8_t *path)
    {
        std::wstring fullpath = GetFullPathNameX(path);
        if (fullpath.size() < 256)
            return fullpath;
        if (fullpath.size() > 2 && fullpath.substr(0, 2) == L"\\\\")
            fullpath.insert(2, L"?\\UNC\\");
        else
            fullpath.insert(0, L"\\\\?\\");
        return fullpath;
    }

    inline FILE *wfopenx(const char8_t *path, const char8_t *mode)
    {
        std::wstring fullpath = win32::prefixed_path(path);
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
    inline std::shared_ptr<FILE> fopen(const std::wstring &path,
                                       const char8_t *mode)
    {
        auto noop_close = [](FILE *){};
        if (path != L"-")
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

    FILE *tmpfile(const char8_t *prefix);

    char *load_with_mmap(const char8_t *path, uint64_t *size);

    int create_named_pipe(const char8_t *path);

    std::string get_dll_version_for_locale(HMODULE hDll, WORD langid);

    bool is_same_file(HANDLE ha, HANDLE hb);

    inline bool is_same_file(int fda, int fdb)
    {
        return is_same_file(get_handle(fda), get_handle(fdb));
    }
}
#endif

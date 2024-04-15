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
#include <sys/stat.h>

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

    unsigned long GetTickCount();

    class Timer {
        uint32_t m_ticks;
    public:
        Timer() { m_ticks = GetTickCount(); };
        double ellapsed() {
            return (static_cast<double>(GetTickCount()) - m_ticks) / 1000.0;
        }
    };

    void throw_error(const std::string& msg, uint32_t error);

    std::string GetFullPathName(const std::string &path);

    std::string GetFullPathNameX(const std::string &path);

    std::string PathReplaceExtension(const std::string &path, const std::string &newExtension);

    std::string PathCombineX(const std::string &basedir, const std::string &filename);

    std::string GetModuleFileNameX(void* module);

    bool MakeSureDirectoryPathExistsX(const std::string &path);

    std::string get_module_directory(void* module);

    std::string prefixed_path(const char *path);

    FILE *wfopenx(const char *path, const char *mode);

    std::shared_ptr<FILE> fopen(const std::string &path,
                                       const char *mode);

    bool is_seekable(int fd);

    int64_t filelengthi64(int fd);

    FILE *tmpfile(const char *prefix);

    char *load_with_mmap(const char *path, uint64_t *size);

    int create_named_pipe(const char *path);

    bool is_same_file(int fda, int fdb);

}
#endif

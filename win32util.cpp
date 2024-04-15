#include "win32util.h"
#include "util.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include "strutil.h"

namespace win32 {

    // get the number of milliseconds since system startup
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

    void throw_error(const std::string &msg, uint32_t code)
    {
        std::string ss;
        ss = strutil::format("%d: %s", code, msg.c_str());
        throw std::runtime_error((ss));
    }

    FILE *tmpfile(const char *prefix)
    {

        std::string template_name =
            strutil::format("%s.%d.XXXXXX", prefix, getpid());

        int fd = mkstemp((char*)template_name.c_str());
        if (fd == -1) {
            util::throw_crt_error("win32::tmpfile: mkstemp()");
        }

        FILE *fp = fdopen(fd, "w+");
        if (!fp) {
            close(fd);
            util::throw_crt_error("win32::tmpfile: _fdopen()");
        }

        return fp;
    }

    char *load_with_mmap(const char *path, uint64_t *size)
    {
        // Open the file
        int fd = open(path, O_RDONLY);
        if (fd == -1) {
            throw std::runtime_error(strutil::format("Failed to open file: %s", strerror(errno)));
        }

        // Get the file size
        struct stat st;
        if (fstat(fd, &st) == -1) {
            close(fd);
            throw std::runtime_error(strutil::format("Failed to get file size: %s", strerror(errno)));
        }
        *size = st.st_size;

        // Map the file into memory
        char* view = static_cast<char*>(mmap(nullptr, *size, PROT_READ, MAP_PRIVATE, fd, 0));
        if (view == MAP_FAILED) {
            close(fd);
            throw std::runtime_error(strutil::format("Failed to mmap file: %s", strerror(errno)));
        }

        // Close the file descriptor
        close(fd);

        return view;
    }

    int create_named_pipe(const char *path)
    {
        // Create the named pipe
        if (mkfifo(path, 0666) == -1) {
            throw std::runtime_error(strutil::format("Failed to create named pipe: %s", strerror(errno)));
        }

        // Open the named pipe for writing
        int fd = open(path, O_WRONLY);
        if (fd == -1) {
            throw std::runtime_error(strutil::format("Failed to open named pipe: %s", strerror(errno)));
        }

        return fd;
    }

    // Check if two file descriptors refer to the same file (Unix implementation)
    inline bool is_same_file(int fda, int fdb)
    {
        // Retrieve file information for the first file descriptor
        struct stat statA;
        if (fstat(fda, &statA) == -1) {
            // Error occurred while retrieving file information for the first file descriptor
            return false;
        }

        // Retrieve file information for the second file descriptor
        struct stat statB;
        if (fstat(fdb, &statB) == -1) {
            // Error occurred while retrieving file information for the second file descriptor
            return false;
        }

        // Compare inode numbers and device IDs to determine if the files are the same
        return statA.st_dev == statB.st_dev && statA.st_ino == statB.st_ino;
    }

}

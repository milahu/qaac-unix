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
    bool is_same_file(int fda, int fdb)
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

    std::string GetFullPathName(const std::string &path) {
        char resolved_path[PATH_MAX];
        if (realpath(path.c_str(), resolved_path) != nullptr) {
            return std::string(resolved_path);
        } else {
            // Handle error, e.g., file not found
            return "";
        }
    }

    std::string GetFullPathNameX(const std::string &path)
    {
        return GetFullPathName(path);
    }

    std::string PathReplaceExtension(const std::string &path, const std::string &newExtension) {
        // Find the last dot in the path
        size_t dotPos = path.find_last_of('.');
        if (dotPos != std::string::npos) {
            // Replace the substring starting from the dot position with the new extension
            return path.substr(0, dotPos) + "." + newExtension;
        }
        // If no dot is found, just append the new extension
        return path + "." + newExtension;
    }

    std::string PathCombineX(const std::string &basedir, const std::string &filename) {
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

    bool MakeSureDirectoryPathExistsX(const std::string &path) {
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

    std::string get_module_directory(void* module = nullptr) {
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

    std::string prefixed_path(const char *path)
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

    FILE *wfopenx(const char *path, const char *mode) {
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

    std::shared_ptr<FILE> fopen(const std::string &path,
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

    // Check if a given file descriptor represents a seekable file (Unix implementation)
    bool is_seekable(int fd) {
        // Attempt to seek to the current position (0 bytes from the current position)
        off_t currentPosition = lseek(fd, 0, SEEK_CUR);
        // If lseek succeeds, the file descriptor is seekable
        return currentPosition != -1;
    }

    // Function to determine the length of a file given its file descriptor
    int64_t filelengthi64(int fd) {
        // Save the current position to restore later
        off_t current_pos = lseek(fd, 0, SEEK_CUR);
        if (current_pos == -1) return -1;

        // Seek to the end of the file to get the size
        off_t end_pos = lseek(fd, 0, SEEK_END);
        if (end_pos == -1) return -1;

        // Restore the original position
        lseek(fd, current_pos, SEEK_SET);

        // Return the file length in bytes
        return end_pos;
    }

}

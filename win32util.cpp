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

    std::string get_dll_version_for_locale(HMODULE hDll, WORD langid)
    {
        HRSRC hRes = FindResourceExW(hDll,
                                     RT_VERSION,
                                     MAKEINTRESOURCEW(VS_VERSION_INFO),
                                     langid);
        if (!hRes)
            win32::throw_error("FindResourceExW", GetLastError());
        std::string data;
        {
            uint32_t cbres = SizeofResource(hDll, hRes);
            HGLOBAL hMem = LoadResource(hDll, hRes);
            if (hMem) {
                char *pc = static_cast<char*>(LockResource(hMem));
                if (cbres && pc)
                    data.assign(pc, cbres);
                FreeResource(hMem);
            }
        }
        // find dwSignature of VS_FIXEDFILEINFO
        size_t pos = data.find("\xbd\x04\xef\xfe");
        if (pos != std::string::npos) {
            VS_FIXEDFILEINFO vfi;
            std::memcpy(&vfi, data.c_str() + pos, sizeof vfi);
            WORD v[4];
            v[0] = HIWORD(vfi.dwFileVersionMS);
            v[1] = LOWORD(vfi.dwFileVersionMS);
            v[2] = HIWORD(vfi.dwFileVersionLS);
            v[3] = LOWORD(vfi.dwFileVersionLS);
            return strutil::format("%u.%u.%u.%u", v[0],v[1],v[2],v[3]);
        }
        return "";
    }

    bool is_same_file(HANDLE ha, HANDLE hb)
    {
        BY_HANDLE_FILE_INFORMATION bhfia, bhfib;

        if (!GetFileInformationByHandle(ha, &bhfia)) return false;
        if (!GetFileInformationByHandle(hb, &bhfib)) return false;
        return bhfia.dwVolumeSerialNumber == bhfib.dwVolumeSerialNumber
            && bhfia.nFileIndexHigh == bhfib.nFileIndexHigh
            && bhfia.nFileIndexLow == bhfib.nFileIndexLow;
    }
}

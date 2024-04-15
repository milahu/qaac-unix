#include "win32util.h"
#include "util.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
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
        std::string fullpath = prefixed_path(path);
        HANDLE hFile = CreateFileW(fullpath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if (hFile == INVALID_HANDLE_VALUE)
            throw_error(fullpath, GetLastError());
        uint32_t sizeHi, sizeLo;
        sizeLo = GetFileSize(hFile, &sizeHi);
        *size = (static_cast<uint64_t>(sizeHi) << 32) | sizeLo;
        HANDLE hMap = CreateFileMappingW(hFile, 0, PAGE_READONLY, 0, 0, 0);
        uint32_t err = GetLastError();
        CloseHandle(hFile);
        if (!hMap) throw_error("CreateFileMapping", err);
        char *view =
            reinterpret_cast<char*>( MapViewOfFile(hMap, FILE_MAP_READ,
                                                   0, 0, 0));
        CloseHandle(hMap);
        return view;
    }

    int create_named_pipe(const char *path)
    {
        HANDLE fh = CreateNamedPipeW(path,
                                     PIPE_ACCESS_OUTBOUND,
                                     PIPE_TYPE_BYTE | PIPE_WAIT,
                                     1, 0, 0, 0, 0);
        if (fh == INVALID_HANDLE_VALUE)
            throw_error(path, GetLastError());
        ConnectNamedPipe(fh, 0);
        return _open_osfhandle(reinterpret_cast<intptr_t>(fh),
                               _O_WRONLY | _O_BINARY);
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

#include "InputFactory.h"
#include <map>
#include <memory>
#include <stdexcept>
#include <cstring> // strrchr
#include <fcntl.h> // open
#include <unistd.h> // lseek
#include "win32util.h"
#ifdef QAAC
#include "ExtAFSource.h"
#endif
#include "FLACSource.h"
#include "LibSndfileSource.h"
#include "RawSource.h"
#include "WaveSource.h"
#include "WavpackSource.h"
#include "MP4Source.h"

std::shared_ptr<ISeekableSource> InputFactory::open(const char *path)
{
    std::map<std::string, std::shared_ptr<ISeekableSource> >::iterator
        pos = m_sources.find(path);
    if (pos != m_sources.end())
        return pos->second;

    const char *ext = strrchr(path, '.');
    if (ext != nullptr) {
        // Move pointer to the position after the dot
        ++ext;
    } else {
        // Handle case when no extension is found
        ext = "";
    }

    int fd = ::open(path, O_RDONLY);
    if (fd == -1) {
        // Handle file open error
        throw std::runtime_error("Failed to open file");
    }
    std::shared_ptr<FILE> fp(fdopen(fd, "rb"));
    if (!fp) {
        // Handle file stream creation error
        ::close(fd);
        throw std::runtime_error("Failed to create file stream");
    }

    if (m_is_raw) {
        std::shared_ptr<RawSource> src =
            std::make_shared<RawSource>(fp, m_raw_format);
        m_sources[path] = src;
        return src;
    }

    if (strcasecmp(ext, "avs") == 0)
        throw std::runtime_error("not implemented: AvisynthSource");

#define TRY_MAKE_SHARED(type, ...) \
    do { \
        try { \
            std::shared_ptr<type> src = \
                std::make_shared<type>(__VA_ARGS__); \
            m_sources[path] = src; \
            return src; \
        } catch (...) { \
            lseek(fd, 0, SEEK_SET); \
        } \
    } while (0)

    TRY_MAKE_SHARED(WaveSource, fd, m_ignore_length);
    // Check if the file is seekable
    if (lseek(fd, 0, SEEK_CUR) == -1) {
        ::close(fd);
        throw std::runtime_error("Not available input file format");
    }

    TRY_MAKE_SHARED(MP4Source, fd);
#ifdef QAAC
    TRY_MAKE_SHARED(ExtAFSource, fd);
#endif
    TRY_MAKE_SHARED(FLACSource, fd);
    TRY_MAKE_SHARED(WavpackSource, path);
    TRY_MAKE_SHARED(LibSndfileSource, fd);

    ::close(fd);
    throw std::runtime_error("Not available input file format");
}

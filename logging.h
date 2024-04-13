#include <cstdio> // stderr, FILE
#include <cstdarg>
#include <vector>
#include <cstdlib> // std::setbuf
#include <fcntl.h> // open
#include <unistd.h> // fileno
#include "win32util.h"

class Log {
    std::vector<std::shared_ptr<FILE>> m_streams;
public:
    static Log &instance()
    {
        static Log self;
        return self;
    }
    bool is_enabled() { return m_streams.size() != 0; }
    void enable_stderr()
    {
        m_streams.push_back(std::shared_ptr<FILE>(stderr, [](FILE*){}));
    }
    void enable_file(const char *filename)
    {
        try {
            FILE *fp = std::fopen(filename, "w");
            if (fp == nullptr) {
                throw std::runtime_error("Log::enable_file(): failed to open file for writing");
            }

            // Set file stream buffer to unbuffered
            std::setbuf(fp, nullptr);

            // Convert FILE pointer to file descriptor
            int fd = fileno(fp);
            if (fd == -1) {
                std::fclose(fp);
                throw std::runtime_error("Log::enable_file(): failed to get fileno");
            }

            // Set file descriptor to text mode
            if (fcntl(fd, F_SETMODE, O_TEXT) == -1) {
                std::fclose(fp);
                throw std::runtime_error("Log::enable_file(): failed to set text mode");
            }

            m_streams.push_back(std::shared_ptr<FILE>(fp, std::fclose));
        } catch (const std::runtime_error& e) {
            throw;
        }
    }
    void vwprintf(const char *fmt, va_list args)
    {
        int rc = _vscwprintf(fmt, args);
        std::vector<char> buffer(rc + 1);
        rc = _vsnwprintf(buffer.data(), buffer.size(), fmt, args);

        OutputDebugStringW(buffer.data());
        for (size_t i = 0; i < m_streams.size(); ++i)
            std::fputws(buffer.data(), m_streams[i].get());
    }
    void wprintf(const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        vwprintf(fmt, ap);
        va_end(ap);
    }
private:
    Log() {}
    Log(const Log&);
    Log& operator=(const Log&);
};

#define LOG(fmt, ...) Log::instance().wprintf(fmt, __VA_ARGS__)

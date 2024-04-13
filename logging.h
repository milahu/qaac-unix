#include <cstdio> // stderr, FILE, vsnprintf
#include <cstdarg> // va_list
#include <vector> // std::vector
#include <cstdlib> // std::setbuf
#include <fcntl.h> // open
#include <unistd.h> // fileno
#include <iostream> // std::cout
#include <memory> // std::shared_ptr
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

            m_streams.push_back(std::shared_ptr<FILE>(fp, std::fclose));
        } catch (const std::runtime_error& e) {
            throw;
        }
    }
    void vprintf(const char *fmt, va_list args)
    {
        va_list args_copy;
        va_copy(args_copy, args);
        int length = vsnprintf(nullptr, 0, fmt, args_copy);
        va_end(args_copy);

        if (length < 0) {
            // Handle error
            return;
        }

        std::vector<char> buffer(length + 1);
        vsnprintf(buffer.data(), buffer.size(), fmt, args);

        // OutputDebugStringW(buffer.data()); // OutputDebugStringW is Windows-specific
        std::cout << buffer.data(); // Print to console instead

        // Assuming m_streams is a vector of shared_ptr<FILE>
        for (size_t i = 0; i < m_streams.size(); ++i) {
            std::fputs(buffer.data(), m_streams[i].get());
        }
    }
    void printf(const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
private:
    Log() {}
    Log(const Log&);
    Log& operator=(const Log&);
};

#define LOG(fmt, ...) Log::instance().printf(fmt, ##__VA_ARGS__)

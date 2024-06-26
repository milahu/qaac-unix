#include <stdarg.h>
#include "strutil.h"

namespace strutil {
    template<> char *strsep(char **strp, const char *sep)
    {
        char *tok, *s;

        if (!strp || !(tok = *strp))
            return 0;
        if ((s = std::strpbrk(tok, sep))) {
            *s = 0;
            *strp = s + 1;
        } else
            *strp = 0;
        return tok;
    }

    std::string &m2w(std::string &dst, const char *src, size_t srclen,
            const std::codecvt<char, char, std::mbstate_t> &cvt)
    {
        typedef std::codecvt<char, char, std::mbstate_t> cvt_t;
        char buffer[0x100];
        std::mbstate_t state = { 0 };
        const char *pend = src + srclen, *pnext = src;
        char *pwbegin = buffer,
                *pwend = buffer + sizeof(buffer)/sizeof(buffer[0]),
                *pwnext = pwbegin;
        dst.clear();
        while (src < pend) {
            if (cvt.in(state, src, pend, pnext,
                        pwbegin, pwend, pwnext) == cvt_t::error)
                throw std::runtime_error("conversion failed");
            dst.append(pwbegin, pwnext - pwbegin);
            pwnext = pwbegin;
            src = pnext;
        }
        return dst;
    }

    std::string &w2m(std::string &dst, const char *src, size_t srclen,
                   const std::codecvt<char, char, std::mbstate_t> &cvt)
    {
        typedef std::codecvt<char, char, std::mbstate_t> cvt_t;
        char buffer[0x100];
        std::mbstate_t state = { 0 };
        const char *pwend = src + srclen, *pwnext = src;
        char *pbegin = buffer,
             *pend = buffer + sizeof(buffer),
             *pnext = pbegin;
        dst.clear();
        while (src < pwend) {
            if (cvt.out(state, src, pwend, pwnext,
                        pbegin, pend, pnext) == cvt_t::error)
                throw std::runtime_error("conversion failed");
            dst.append(pbegin, pnext - pbegin);
            pnext = pbegin;
            src = pwnext;
        }
        return dst;
    }

#if defined(_MSC_VER) || defined(__MINGW32__)
#ifndef vsnprintf
#define vsnprintf _vsnprintf
#endif
#endif
    std::string format(const char *fmt, ...)
    {
        va_list args;
        std::vector<char> buffer(128);

        va_start(args, fmt);
        int rc = vsnprintf(&buffer[0], buffer.size(), fmt, args);
        va_end(args);
        if (rc >= 0 && rc < static_cast<int>(buffer.size()))
            return std::string(&buffer[0], &buffer[rc]);
#if defined(_MSC_VER) || defined(__MINGW32__) 
        va_start(args, fmt);
        rc = _vscprintf(fmt, args);
        va_end(args);
        if (rc < 0) {
            // format failed
            return "";
        }
#endif
        buffer.resize(rc + 1);
        va_start(args, fmt);
        rc = vsnprintf(&buffer[0], buffer.size(), fmt, args);
        va_end(args);
        return std::string(&buffer[0], &buffer[rc]);
    }

#if defined(_MSC_VER) || defined(__MINGW32__) 
    std::string format(const char *fmt, ...)
    {
        va_list args;

        va_start(args, fmt);
        int rc = _vscwprintf(fmt, args);
        va_end(args);

        std::vector<char> buffer(rc + 1);

        va_start(args, fmt);
        rc = _vsnwprintf(&buffer[0], buffer.size(), fmt, args);
        va_end(args);

        return std::string(&buffer[0], &buffer[rc]);
    }
#endif

    /*
     * NUMBER ::= [0-9]+
     * TERM ::= NUMBER | NUMBER"-"NUMBER
     * RANGES ::= TERM | RANGES","TERM
     */
    bool parse_numeric_ranges(const char *s, std::vector<int> *nums,
                              int vmin, int vmax)
    {
        enum { NUMBER, TERM };
        char *end;
        std::vector<int> result;
        int n, state = NUMBER;

        do {
            n = std::strtoul(s, &end, 10);
            if (end == s || n < vmin || n > vmax)
                return false;
            if (state == NUMBER)
                result.push_back(n);
            else if (result.back() > n)
                return false;
            else {
                /* XXX: can consume HUGE memory depending on vmin and vmax */
                for (int k = result.back() + 1; k <= n; ++k)
                    result.push_back(k);
            }
            if (*end == ',')
                state = NUMBER;
            else if (*end == '-') {
                if (state == TERM)
                    return false;
                else
                    state = TERM;
            } else if (*end)
                return false;
        } while (*end && (s = end + 1));

        nums->swap(result);
        return true;
    }
}

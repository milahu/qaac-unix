#ifndef STRUTIL_HPP_INCLUDED
#define STRUTIL_HPP_INCLUDED

#include <cwchar>
#include <string>
#include <cstring>
#include <vector>
#include <locale>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <codecvt>

#if defined _MSC_VER
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#endif

#if defined _MSC_VER
typedef intptr_t ssize_t;
#endif

namespace strutil {
    template<typename T> T *strsep(T **strp, const T *sep);
    template<> char *strsep(char **strp, const char *sep);
    template<> char *strsep(char **strp, const char *sep);

    std::string &m2w(std::string &dst, const char *src, size_t srclen,
            const std::codecvt<char, char, std::mbstate_t> &cvt);

    std::string &w2m(std::string &dst, const char *src, size_t srclen,
                   const std::codecvt<char, char, std::mbstate_t> &cvt);

    template <typename T, typename Conv>
    inline
    std::basic_string<T> strtransform(const std::basic_string<T> &s, Conv conv)
    {
        std::basic_string<T> result;
        std::transform(s.begin(), s.end(), std::back_inserter(result), conv);
        return result;
    }
    inline
    std::string slower(const std::string &s)
    {
        return strtransform(s, tolower);
    }
    inline
    std::string wslower(const std::string &s)
    {
        return strtransform(s, towlower);
    }
    inline
    std::string supper(const std::string &s)
    {
        return strtransform(s, toupper);
    }
    inline
    std::string wsupper(const std::string &s)
    {
        return strtransform(s, towupper);
    }
    inline ssize_t strindex(const char *s, int ch)
    {
        const char *p = std::strchr(s, ch);
        return p ? p - s : -1;
    }
    template <typename T>
    void squeeze(T *str, const T *charset)
    {
        T *q = str;
        for (T *p = str; *p; ++p)
            if (strindex(charset, *p) == -1)
                *q++ = *p;
        *q = 0;
    }
    template <typename T>
    std::basic_string<T> squeeze(const std::basic_string<T> &str,
                                 const T *charset)
    {
        std::basic_string<T> result;
        std::copy_if(str.begin(), str.end(), std::back_inserter(result),
                     [&](T c) -> bool {
                        return strindex(charset, c) == -1;
                     });
        return result;
    }

    inline
    std::string m2w(const std::string &src,
                     const std::codecvt<char, char, std::mbstate_t> &cvt)
    {
        std::string result;
        return m2w(result, src.c_str(), src.size(), cvt);
    }
    inline
    std::string m2w(const std::string &src)
    {
        typedef std::codecvt<char, char, std::mbstate_t> cvt_t;
        std::locale loc("");
        return m2w(src, std::use_facet<cvt_t>(loc));
    }
    inline
    std::string us2w(const std::string &src)
    {
        return src;
    }
    inline
    std::string w2m(const std::string& src,
                    const std::codecvt<char, char, std::mbstate_t> &cvt)
    {
        std::string result;
        return w2m(result, src.c_str(), src.size(), cvt);
    }
    inline
    std::string w2m(const std::string &src)
    {
        typedef std::codecvt<char, char, std::mbstate_t> cvt_t;
        std::locale loc("");
        return w2m(src, std::use_facet<cvt_t>(loc));
    }
    inline
    std::string w2us(const std::string &src)
    {
        return src;
    }

    std::string format(const char *fmt, ...);
    std::string format(const char *fmt, ...);

    template <typename T>
    std::basic_string<T> normalize_crlf(const T *s, const T *eol)
    {
        std::basic_string<T> result;
        T c;
        while ((c = *s++)) {
            if (c == '\r') {
                result.append(eol);
                if (*s == '\n')
                    ++s;
            }
            else if (c == '\n')
                result.append(eol);
            else
                result.push_back(c);
        }
        return result;
    }

    template<typename T>
    class Tokenizer {
        std::vector<T> m_buffer;
        const T *m_sep;
        T *m_tok;
    public:
        Tokenizer(const std::basic_string<T> &s, const T *sep)
            : m_sep(sep)
        {
            std::copy(s.begin(), s.end(), std::back_inserter(m_buffer));
            m_buffer.push_back(0);
            m_tok = &m_buffer[0];
        }
        T *next()
        {
            return strsep(&m_tok, m_sep);
        }
        T *rest()
        {
            return m_tok;
        }
    };

    bool parse_numeric_ranges(const char *s, std::vector<int> *nums,
                              int vmin=0, int vmax=99);
}

#endif

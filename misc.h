#ifndef MISC_H
#define MISC_H

#include <stdint.h>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <complex>

namespace misc {
    typedef std::pair<std::wstring, double> chapter_t;
    typedef std::complex<float> complex_t;

    std::wstring loadTextFile(const std::wstring &path, int codepage=0);

    std::wstring generateFileName(const std::wstring &spec,
                                  const std::map<std::string, std::string>&tag);

    // the resulting chapter_t is <name, absolute timestamp>
    std::vector<chapter_t> loadChapterFile(const char8_t *file,
                                           uint32_t codepage);

    // convert <name, absolute timestamp> to <name, timedelta>
    std::vector<chapter_t>
        convertChaptersToQT(const std::vector<chapter_t> &chapters,
                            double total_duration);

    std::shared_ptr<FILE> openConfigFile(const char8_t *file);

    std::vector<std::vector<complex_t>>
    loadRemixerMatrixFromFile(const char8_t *path);

    std::vector<std::vector<complex_t>>
    loadRemixerMatrixFromPreset(const char8_t *preset_name);
}

#endif

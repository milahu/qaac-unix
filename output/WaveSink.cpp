#include <cstdio>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include "WaveSource.h"
#include "WaveSink.h"

namespace {
    inline uint16_t get_wave_format(const AudioStreamBasicDescription &asbd)
    {
        return (asbd.mChannelsPerFrame > 2
                || asbd.mBitsPerChannel > 16
                || (asbd.mBitsPerChannel & 7)
                || (asbd.mFormatFlags & kAudioFormatFlagIsFloat)) ? 0xfffe : 1;
    }
}

WaveSink::WaveSink(const std::shared_ptr<FILE> &fp,
                   uint64_t duration,
                   const AudioStreamBasicDescription &asbd,
                   uint32_t chanmask)
        : m_file(fp), m_closed(false), m_seekable(false), m_fact(false),
          m_chanmask(chanmask), m_bytes_written(0), m_asbd(asbd)
{
    m_seekable = win32::is_seekable(fileno(fp.get()));
    std::string header = buildHeader();

    uint32_t hdrsize = header.size();
    uint32_t riffsize = ~0, datasize = ~0;
    m_rf64 = m_seekable;
    if (duration != ~0ULL) {
        uint64_t datasize64 = duration * m_bytes_per_frame;
        uint64_t riffsize64 = hdrsize + datasize64 + 20;
        if (riffsize64 >> 32 == 0) {
            datasize = static_cast<uint32_t>(datasize64);
            riffsize = static_cast<uint32_t>(riffsize64);
            m_rf64 = false;
        }
    }
    write("RIFF", 4);
    write(&riffsize, 4);
    write("WAVE", 4);
    if (m_rf64) {
        write("JUNK", 4);
        static const char filler[32] = { 0x1c, 0 };
        write(filler, 32);
    }
    write("fmt ", 4);
    write(&hdrsize, 4);
    write(header.c_str(), hdrsize);
    if (m_seekable && get_wave_format(m_asbd) == 0xfffe) {
        m_fact = true;
        write("JUNK\004\0\0\0\0\0\0\0", 12);
    }
    write("data", 4);
    write(&datasize, 4);
    m_data_pos = 28 + hdrsize + (m_fact ? 12 : 0) + (m_rf64 ? 36 : 0);
}

std::string WaveSink::buildHeader()
{
    std::ostringstream oss;
    std::stringbuf *os = oss.rdbuf();

    int bpc = ((m_asbd.mBitsPerChannel + 7) & ~7) / 8;
    m_bytes_per_frame = bpc * m_asbd.mChannelsPerFrame;
    
    // wFormatTag
    uint16_t fmt = get_wave_format(m_asbd);
    put(os, fmt);
    // nChannels
    put(os, static_cast<uint16_t>(m_asbd.mChannelsPerFrame));
    // nSamplesPerSec
    put(os, static_cast<uint32_t>(m_asbd.mSampleRate));

    // nAvgBytesPerSec
    put(os, static_cast<uint32_t>(m_asbd.mSampleRate * m_bytes_per_frame));
    // nBlockAlign
    put(os, static_cast<uint16_t>(m_bytes_per_frame));
    // wBitsPerSample
    put(os, static_cast<uint16_t>(bpc << 3));

    // cbSize
    if (fmt == 0xfffe) {
        // WAVEFORMATEXTENSIBLE
        put(os, static_cast<uint16_t>(22));
        // Samples
        put(os, static_cast<uint16_t>(m_asbd.mBitsPerChannel));
        // dwChannelMask
        put(os, m_chanmask);
        // SubFormat
        if (m_asbd.mFormatFlags & kAudioFormatFlagIsFloat)
            put(os, wave::ksFormatSubTypeFloat);
        else
            put(os, wave::ksFormatSubTypePCM);
    }
    return oss.str();
}

/*
 * XXX: This method rewrites const data arg
 */
void WaveSink::writeSamples(const void *data, size_t length, size_t nsamples)
{
    void *bp = const_cast<void *>(data);
    if (m_asbd.mBitsPerChannel <= 8) {
        util::convert_sign(static_cast<uint32_t *>(bp),
                           nsamples * m_asbd.mChannelsPerFrame);
    }
    if (m_bytes_per_frame < m_asbd.mBytesPerFrame) {
        unsigned obpc = m_asbd.mBytesPerFrame / m_asbd.mChannelsPerFrame;
        unsigned nbpc = m_bytes_per_frame / m_asbd.mChannelsPerFrame;
        util::pack(bp, &length, obpc, nbpc);
    }
    write(bp, length);
    m_bytes_written += length;
    if (!m_seekable) std::fflush(m_file.get());
}

void WaveSink::finishWrite()
{
    if (m_closed)
        return;
    m_closed = true;
    FILE *fp = m_file.get();
    if (m_bytes_written & 1) write("\0", 1);
    if (!m_seekable) return;
    uint64_t datasize64 = m_bytes_written;
    uint64_t riffsize64 = datasize64 + m_data_pos - 8;
    uint64_t nsamples = m_bytes_written / m_bytes_per_frame;
    if (riffsize64 >> 32 == 0) {
        if (std::fseek(fp, m_data_pos - 4, SEEK_SET) == 0) {
            uint32_t size32 = static_cast<uint32_t>(datasize64);
            write(&size32, 4);
            if (std::fseek(fp, 4, SEEK_SET) == 0) {
                size32 = static_cast<uint32_t>(riffsize64);
                write(&size32, 4);
            }
        }
        if (m_fact) {
            uint32_t samples32 = static_cast<uint32_t>(nsamples);
            if (std::fseek(fp, m_data_pos - 20, SEEK_SET) == 0) {
                write("fact\004\0\0\0", 8);
                write(&samples32, 4);
            }
        }
    } else if (m_rf64) {
        std::rewind(fp);
        write("RF64", 4);
        std::fseek(fp, 8, SEEK_CUR);
        write("ds64", 4);
        std::fseek(fp, 4, SEEK_CUR);
        write(&riffsize64, 8);
        write(&datasize64, 8);
        write(&nsamples, 8);
    }
}

#ifndef CAFSINK_H
#define CAFSINK_H

#include <map>
#include "ISink.h"
#include "win32util.h"
#ifndef NO_COREAUDIO
#include "AudioFile.h"
#endif
#include "ALAC/EndianPortable.h" // BSWAP16 BSWAP32 BSWAP64
#include "CoreAudio/CoreAudioTypes.h"
#include "CoreAudio/AudioFile.h"

class CAFSink : public ISink, public ITagStore {
    std::shared_ptr<FILE> m_file;
    bool m_seekable;
    uint32_t m_data_pos;
    uint64_t m_bytes_written;
    uint64_t m_frames_written;
    uint32_t m_channel_layout;
    std::vector<uint8_t > m_magic_cookie;
    std::map<std::string, std::string> m_tags;
    std::vector<uint32_t> m_packet_table;
    AudioStreamBasicDescription m_asbd;
public:
    CAFSink(const std::string &filename,
            const AudioStreamBasicDescription &asbd,
            uint32_t channel_layout,
            const std::vector<uint8_t> &cookie)
    {
        init(win32::fopen(filename, "wb"), asbd, channel_layout, cookie);
    }
    CAFSink(const std::shared_ptr<FILE> &file,
            const AudioStreamBasicDescription &asbd,
            uint32_t channel_layout,
            const std::vector<uint8_t> &cookie)
    {
        init(file, asbd, channel_layout, cookie);
    }
    void setTag(const std::string &key, const std::string &value)
    {
        if (key.find("iTunes:") != 0)
            m_tags[key] = value;
    }
    void beginWrite();
    void writeSamples(const void *data, size_t length, size_t nsamples);
    void finishWrite(const AudioFilePacketTableInfo &info);
private:
    void init(const std::shared_ptr<FILE> &file,
              const AudioStreamBasicDescription &asbd,
              uint32_t channel_layout,
              const std::vector<uint8_t> &cookie);
    uint32_t packedBytesPerFrame()
    {
        if (m_asbd.mBytesPerFrame == 0)
            return 0;
        unsigned bytesPerChannel = ((m_asbd.mBitsPerChannel + 7) & ~7) / 8;
        return bytesPerChannel * m_asbd.mChannelsPerFrame;
    }
    void write(const void *data, size_t length) {
        FILE *fp = m_file.get();
        if (fwrite(data, 1, length, fp) < length) {
            throw std::runtime_error("write failed");
        }
    }
    void write32(uint32_t x)
    {
        x = BSWAP32(x);
        write(&x, 4);
    }
    void write64(uint64_t x)
    {
        x = BSWAP64(x);
        write(&x, 8);
    }
    void writef64(double x)
    {
        union { double d; uint64_t i; } d2i;
        d2i.d = x;
        write64(d2i.i);
    }
    void writeBER(uint32_t x);

    void writeASBD(uint32_t format);
    void desc();
    void chan();
    void kuki();
    void ldsc();
    void info();
    void data();
    void pakt(const AudioFilePacketTableInfo &info);
};

#endif

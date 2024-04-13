#ifndef METADATA_H
#define METADATA_H

#include <iterator>
#include "mp4v2wrapper.h"
#include "cautil.h"
#include "CoreAudio/AudioFile.h"

namespace Tag {
    const uint32_t kTitle = FOURCC(*(int32_t*)"\xa9",'n','a','m');
    const uint32_t kArtist = FOURCC(*(int32_t*)"\xa9",'A','R','T');
    const uint32_t kAlbumArtist = *(int32_t*)"aART";
    const uint32_t kAlbum = FOURCC(*(int32_t*)"\xa9",'a','l','b');
    const uint32_t kGrouping = FOURCC(*(int32_t*)"\xa9",'g','r','p');
    const uint32_t kComposer = FOURCC(*(int32_t*)"\xa9",'w','r','t');
    const uint32_t kComment = FOURCC(*(int32_t*)"\xa9",'c','m','t');
    const uint32_t kGenre = FOURCC(*(int32_t*)"\xa9",'g','e','n');
    const uint32_t kGenreID3 = *(int32_t*)"gnre";
    const uint32_t kDate = FOURCC(*(int32_t*)"\xa9",'d','a','y');
    const uint32_t kTrack = *(int32_t*)"trkn";
    const uint32_t kDisk = *(int32_t*)"disk";
    const uint32_t kTempo = *(int32_t*)"tmpo";
    const uint32_t kDescription = *(int32_t*)"desc";
    const uint32_t kLongDescription = *(int32_t*)"ldes";
    const uint32_t kLyrics = FOURCC(*(int32_t*)"\xa9",'l','y','r');
    const uint32_t kCopyright = *(int32_t*)"cprt";
    const uint32_t kCompilation = *(int32_t*)"cpil";
    const uint32_t kTool = FOURCC(*(int32_t*)"\xa9",'t','o','o');
    const uint32_t kArtwork = *(int32_t*)"covr";

    const uint32_t kTvSeason = *(int32_t*)"tvsn";
    const uint32_t kTvEpisode = *(int32_t*)"tves";
    const uint32_t kPodcast = *(int32_t*)"pcst";
    const uint32_t kHDVideo = *(int32_t*)"hdvd";
    const uint32_t kMediaType = *(int32_t*)"stik";
    const uint32_t kContentRating = *(int32_t*)"rtng";
    const uint32_t kGapless = *(int32_t*)"pgap";
    const uint32_t kiTunesAccountType = *(int32_t*)"akID";
    const uint32_t kiTunesCountry = *(int32_t*)"sfID";
    const uint32_t kcontentID = *(int32_t*)"cnID";
    const uint32_t kartistID = *(int32_t*)"atID";
    const uint32_t kplaylistID = *(int32_t*)"plID";
    const uint32_t kgenreID = *(int32_t*)"geID";
    const uint32_t kcomposerID = *(int32_t*)"cmID";
}

namespace TextBasedTag {
    std::string normalizeTagName(const char *name);
    std::map<std::string, std::string>
        normalizeTags(const std::map<std::string, std::string> &src);
}

namespace ID3 {
    std::map<std::string, std::string> fetchAiffID3Tags(int fd);
    std::map<std::string, std::string> fetchMPEGID3Tags(int fd);
}

namespace M4A {
    const char *getTagNameFromFourCC(uint32_t fcc);

    void convertToM4ATags(const std::map<std::string, std::string> &src,
                          std::map<uint32_t, std::string> *shortTags,
                          std::map<std::string, std::string> *longTags);

    std::map<std::string, std::string> fetchTags(MP4FileX &file);
}

namespace CAF {
    std::map<std::string, std::string>
        fetchTags(const std::vector<uint8_t> &info);
    std::map<std::string, std::string> fetchTags(int fd);
}

const char * const iTunSMPB_template = " 00000000 %08X %08X %08X%08X "
"00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000";

#endif

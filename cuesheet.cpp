#include <sstream>
#include <clocale>
#include <locale>
#include <regex>
#include "cuesheet.h"
#include "win32util.h"
#include "metadata.h"
#include "CompositeSource.h"
#include "NullSource.h"
#include "TrimmedSource.h"
#include "InputFactory.h"

static inline
unsigned msf2frames(unsigned mm, unsigned ss, unsigned ff)
{
    return (mm * 60 + ss) * 75 + ff;
}

static inline
uint64_t frame2sample(double sampling_rate, uint32_t nframe)
{
    return static_cast<uint64_t>(nframe / 75.0 * sampling_rate + 0.5);
}

template <typename CharT>
bool CueTokenizer<CharT>::nextline()
{
    m_fields.clear();
    int_type c;
    std::basic_string<CharT> field;
    while (traits_type::not_eof(c = m_sb->sbumpc())) {
        if (c == '"') {
            // eat until closing quote
            while (traits_type::not_eof(c = m_sb->sbumpc())) {
                if (c == '\n')
                    throw std::runtime_error(strutil::format(
                        "cuesheet: runaway string at line %u",
                        static_cast<uint32_t>(m_lineno) + 1));
                else if (c != '"')
                    field.push_back(c);
                else if (m_sb->sgetc() != '"') // closing quote
                    break;
                else { // escaped quote
                    m_sb->snextc();
                    field.push_back(c);
                }
            }
        }
        else if (c == '\n') {
            ++m_lineno;
            break;
        }
        else if (isWS(c)) {
            if (field.size()) {
                m_fields.push_back(field);
                field.clear();
            }
            while (isWS(m_sb->sgetc()))
                m_sb->snextc();
        }
        else
            field.push_back(c);
    }
    if (field.size()) m_fields.push_back(field);
    return m_fields.size() > 0 || c == '\n';
}

template struct CueTokenizer<char>;
template struct CueTokenizer<char>;

void CueTrack::addSegment(const CueSegment &seg)
{
    if (m_segments.size()) {
        CueSegment &last = m_segments.back();
        if (last.m_index >= seg.m_index) {
            if (last.m_index == 0x7fffffff) {
                std::string msg =
                    strutil::format("cuesheet: conflicting use of "
                                    "INDEX00 or PREGAP found on "
                                    "track %u-%u", m_number, m_number + 1);
                throw std::runtime_error(msg);
            }
            std::string msg =
                strutil::format("cuesheet: INDEX must be in strictly "
                                "ascending order: track %u", m_number);
            throw std::runtime_error(msg);
        }
        if (last.m_filename == seg.m_filename && last.m_end == seg.m_begin)
        {
            last.m_end = seg.m_end;
            return;
        }
    }
    m_segments.push_back(seg);
}

std::map<std::string, std::string> CueTrack::getTags() const
{
    auto result = m_cuesheet->getTags();
    std::for_each(m_meta.begin(), m_meta.end(),
                  [&](decltype(*m_meta.begin()) &tag) {
                      auto key = (tag.first);
                      if (key == "PERFORMER")
                          key = "artist";
                      result[key] = (tag.second);
                  });
    result["track number"]
        = strutil::format("%u/%u", number(), m_cuesheet->count());
    return TextBasedTag::normalizeTags(result);
}

void CueSheet::parse(std::wstreambuf *src)
{
    static struct handler_t {
        const char *cmd;
        void (CueSheet::*mf)(const std::string *args);
        size_t nargs;
    } handlers[] = {
        { "FILE", &CueSheet::parseFile, 3 },
        { "TRACK", &CueSheet::parseTrack, 3 },
        { "INDEX", &CueSheet::parseIndex, 3 },
        { "POSTGAP", &CueSheet::parsePostgap, 2 },
        { "PREGAP", &CueSheet::parsePregap, 2 },
        { "REM", &CueSheet::parseRem, 3 },
        { "CATALOG", &CueSheet::parseMeta, 2 },
        { "ISRC", &CueSheet::parseMeta, 2 },
        { "PERFORMER", &CueSheet::parseMeta, 2 },
        { "SONGWRITER", &CueSheet::parseMeta, 2 },
        { "TITLE", &CueSheet::parseMeta, 2 },
        { 0, 0, 0 }
    };

    CueTokenizer<char> tokenizer(src);
    while (tokenizer.nextline()) {
        if (!tokenizer.m_fields.size())
            continue;
        m_lineno = tokenizer.m_lineno;
        std::string cmd = tokenizer.m_fields[0];
        for (handler_t *p = handlers; p->cmd; ++p) {
            if (cmd != p->cmd)
                continue;
            if (tokenizer.m_fields.size() == p->nargs)
                (this->*p->mf)(&tokenizer.m_fields[0]);
            else if (cmd != "REM")
                die(strutil::format("wrong num args for %ls command", p->cmd));
            break;
        }
        // if (!p->cmd) die("Unknown command");
    }
    validate();
}

std::vector<std::shared_ptr<ISeekableSource>>
CueSheet::loadTracks(bool is_embedded, const std::string &path,
                     const std::vector<int> &selection)
{
    std::vector<std::shared_ptr<ISeekableSource>> tracks;
    for (auto track = begin(); track != end(); ++track) {
        if (selection.size()) {
            if (std::find(selection.begin(), selection.end(),
                          track->number()) == selection.end())
                continue;
        }
        std::shared_ptr<CompositeSource> track_source(new CompositeSource());
        std::for_each(track->begin(), track->end(), [&](const CueSegment &seg) {
            std::shared_ptr<ISeekableSource> src;
            if (seg.m_filename == "__GAP__") {
                if (tracks.size())
                    src.reset(new NullSource(src->getSampleFormat()));
            } else if (is_embedded) {
                src = InputFactory::instance().open(path.c_str());
            } else {
                std::string ifilename =
                    win32::PathCombineX(path, seg.m_filename);
                src = InputFactory::instance().open(ifilename.c_str());
            }
            if (src.get()) {
                double rate = src->getSampleFormat().mSampleRate;
                uint64_t begin = frame2sample(rate, seg.m_begin);
                uint64_t duration = ~0ULL;
                if (seg.m_end != ~0U)
                    duration = frame2sample(rate, seg.m_end) - begin;
                src.reset(new TrimmedSource(src, begin, duration));
                track_source->addSource(src);
            }
        });
        auto &stags = track_source->getTags();
        std::regex re("^CUE_TRACK(\\d+)_(.*)$", std::regex_constants::icase);
        std::smatch match;
        std::map<std::string, std::string> tags;
        for (auto it = stags.begin(); it != stags.end(); ++it) {
            if (std::regex_match(it->first, match, re)) {
                int tn = atoi(match[1].str().c_str());
                if (tn == track->number())
                    tags[match[2].str()] = it->second;
            } else
                tags[it->first] = it->second;
        }
        auto track_tags = track->getTags();
        for (auto it = track_tags.begin(); it != track_tags.end(); ++it)
            tags[it->first] = it->second;
        track_source->setTags(tags);
        tracks.push_back(track_source);
    }
    return tracks;
}

std::map<std::string, std::string> CueSheet::getTags() const
{
    std::map<std::string, std::string> result;
    std::for_each(m_meta.begin(), m_meta.end(),
                  [&](decltype(*m_meta.begin()) &tag) {
        std::string key = (tag.first);
        std::string val = (tag.second);

        if (key == "PERFORMER") {
            result["artist"] = val;
            result["ALBUM ARTIST"] = val;
        } else if (key == "TITLE")
            result["album"] = val;
        else
            result[key] = val;
    });
    return TextBasedTag::normalizeTags(result);
}

void CueSheet::validate()
{
    auto index1_missing =
        [](const CueTrack &track) {
            return !std::count_if(track.begin(), track.end(),
                                  [](const CueSegment &seg) {
                                      return seg.m_index == 1;
                                  });
        };
    auto track = std::find_if(begin(), end(), index1_missing);
    if (track != end()) {
        std::string msg =
            strutil::format("cuesheet: INDEX01 not found on track %u",
                            track->number());
        throw std::runtime_error(msg);
    };
}

void CueSheet::parseFile(const std::string *args)
{
    if (!m_cur_file.empty() && m_cur_file != args[1])
        this->m_has_multiple_files = true;
    m_cur_file = args[1];
}
void CueSheet::parseTrack(const std::string *args)
{
    if (args[2] == "AUDIO") {
        unsigned no;
        if (std::swscanf(args[1].c_str(), "%d", &no) != 1)
            die("Invalid TRACK number");
        m_tracks.push_back(CueTrack(this, no));
    }
}
void CueSheet::parseIndex(const std::string *args)
{
    if (!m_tracks.size())
        die("INDEX command before TRACK");
    if (m_cur_file.empty())
        die("INDEX command before FILE");
    unsigned no, mm, ss, ff, nframes;
    if (std::swscanf(args[1].c_str(), "%u", &no) != 1)
        die("Invalid INDEX number");
    if (std::swscanf(args[2].c_str(), "%u:%u:%u", &mm, &ss, &ff) != 3)
        die("Invalid INDEX time format");
    if (ss > 59 || ff > 74)
        die("Invalid INDEX time format");
    nframes = msf2frames(mm, ss, ff);
    CueSegment *lastseg = lastSegment();
    if (lastseg && lastseg->m_filename == m_cur_file) {
        lastseg->m_end = nframes;
        if (lastseg->m_begin >= nframes)
            die("INDEX time must be in ascending order");
    }
    CueSegment segment(m_cur_file, no);
    segment.m_begin = nframes;
    if (no > 0)
        m_tracks.back().addSegment(segment);
    else {
        if (m_tracks.size() == 1) {
            /* HTOA */
            m_tracks.insert(m_tracks.begin(), CueTrack(this, 0));
            m_tracks[0].setMeta("title", "(HTOA)");
            segment.m_index = 1;
        } else
            segment.m_index = 0x7fffffff;
        m_tracks[m_tracks.size() - 2].addSegment(segment);
    }
}
void CueSheet::parsePostgap(const std::string *args)
{
    if (!m_tracks.size())
        die("POSTGAP command before TRACK");
    unsigned mm, ss, ff;
    if (std::swscanf(args[1].c_str(), "%u:%u:%u", &mm, &ss, &ff) != 3)
        die("Invalid POSTGAP time format");
    CueSegment segment(std::string("__GAP__"), 0x7ffffffe);
    segment.m_end = msf2frames(mm, ss, ff);
    m_tracks.back().addSegment(segment);
}
void CueSheet::parsePregap(const std::string *args)
{
    if (!m_tracks.size())
        die("PREGAP command before TRACK");
    unsigned mm, ss, ff;
    if (std::swscanf(args[1].c_str(), "%u:%u:%u", &mm, &ss, &ff) != 3)
        die("Invalid PREGAP time format");
    CueSegment segment(std::string("__GAP__"), 0x7fffffff);
    segment.m_end = msf2frames(mm, ss, ff);
    if (m_tracks.size() > 1)
        m_tracks[m_tracks.size() - 2].addSegment(segment);
}
void CueSheet::parseMeta(const std::string *args)
{
    if (m_tracks.size())
        m_tracks.back().setMeta(args[0], args[1]);
    else
        m_meta[args[0]] = args[1];
}

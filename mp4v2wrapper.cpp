#include "util.h"
#include "mp4v2wrapper.h"
#include "strutil.h"
#include "win32util.h"
#undef FindAtom // XXX: conflicts with kernel32 function macro

using mp4v2::impl::MP4File;
using mp4v2::impl::MP4Track;
using mp4v2::impl::MP4Atom;
using mp4v2::impl::MP4RootAtom;
using mp4v2::impl::MP4DataAtom;
using mp4v2::impl::MP4NameAtom;
using mp4v2::impl::MP4MeanAtom;
using mp4v2::impl::MP4Property;
using mp4v2::impl::MP4Integer16Property;
using mp4v2::impl::MP4Integer32Property;
using mp4v2::impl::MP4Integer64Property;
using mp4v2::impl::MP4StringProperty;
using mp4v2::impl::MP4BytesProperty;
using mp4v2::impl::MP4TableProperty;
using mp4v2::platform::io::File;
namespace itmf = mp4v2::impl::itmf;

std::string format_mp4error(const mp4v2::impl::Exception &e)
{
    return strutil::format("libmp4v2: %s", e.msg().c_str());
}

class MP4AlacAtom: public MP4Atom {
public:
    MP4AlacAtom(MP4File &file, const char *id): MP4Atom(file, id)
    {
        AddVersionAndFlags();
        AddProperty(new MP4BytesProperty(*this, "decoderConfig"));
    }
};

class MP4ChanAtom: public MP4Atom {
public:
    MP4ChanAtom(MP4File &file, const char *id): MP4Atom(file, id)
    {
        AddVersionAndFlags();
        AddProperty(new MP4BytesProperty(*this, "channelLayout"));
    }
};

class MP4SbgpAtom: public MP4Atom {
public:
    MP4SbgpAtom(MP4File &file, const char *id): MP4Atom(file, id)
    {
        AddVersionAndFlags();
        AddProperty(new MP4Integer32Property(*this, "groupingType"));
        MP4Integer32Property *count =
            new MP4Integer32Property(*this, "entryCount");
        AddProperty(count);
        MP4TableProperty *table = new MP4TableProperty(*this, "entries", count);
        AddProperty(table);
        table->AddProperty(new MP4Integer32Property(table->GetParentAtom(),
                                                    "sampleCount"));
        table->AddProperty(new MP4Integer32Property(table->GetParentAtom(),
                                                    "groupDescriptionIndex"));
    }
};

class MP4SgpdAtom: public MP4Atom {
public:
    MP4SgpdAtom(MP4File &file, const char *id): MP4Atom(file, id)
    {
        AddVersionAndFlags();
        SetVersion(1);
        AddProperty(new MP4Integer32Property(*this, "groupingType"));
        MP4Integer32Property *defaultLength =
            new MP4Integer32Property(*this, "defaultLength");
        defaultLength->SetValue(2);
        AddProperty(defaultLength);
        MP4Integer32Property *count =
            new MP4Integer32Property(*this, "entryCount");
        AddProperty(count);
        MP4TableProperty *table = new MP4TableProperty(*this, "entries", count);
        AddProperty(table);
        table->AddProperty(new MP4Integer16Property(table->GetParentAtom(),
                                                    "rollDistance"));
    }
};

MP4TrackId
MP4FileX::AddAlacAudioTrack(const uint8_t *alac, const uint8_t *chan)
{
    uint32_t bitsPerSample = alac[5];
    uint32_t nchannels = alac[9];
    uint32_t timeScale;
    std::memcpy(&timeScale, alac + 20, 4);
    timeScale = util::b2host32(timeScale);

    SetTimeScale(timeScale);
    MP4TrackId track = AddTrack(MP4_AUDIO_TRACK_TYPE, timeScale);
    AddTrackToOd(track);

    SetTrackFloatProperty(track, "tkhd.volume", 1.0);

    InsertChildAtom(MakeTrackName(track, "mdia.minf"), "smhd", 0);
    AddChildAtom(MakeTrackName(track, "mdia.minf.stbl.stsd"), "alac");

    MP4Atom *atom = FindTrackAtom(track, "mdia.minf.stbl.stsd");
    MP4Property *pProp;
    atom->FindProperty("stsd.entryCount", &pProp);
    dynamic_cast<MP4Integer32Property*>(pProp)->IncrementValue();

    atom = atom->FindChildAtom("alac");

    /* XXX
       Would overflow when samplerate >= 65536, so we shift down here.
       Anyway, iTunes seems to always set 44100 to stsd samplerate for ALAC.
     */
    while (timeScale & 0xffff0000)
        timeScale >>= 1;
    atom->FindProperty("alac.timeScale", &pProp);
    dynamic_cast<MP4Integer32Property*>(pProp)->SetValue(timeScale<<16);
    atom->FindProperty("alac.sampleSize", &pProp);
    dynamic_cast<MP4Integer16Property*>(pProp)->SetValue(bitsPerSample);
    atom->FindProperty("alac.channels", &pProp);
    dynamic_cast<MP4Integer16Property*>(pProp)->SetValue(nchannels);

    MP4AlacAtom *alacAtom = new MP4AlacAtom(*this, "alac");
    pProp = alacAtom->GetProperty(2);
    dynamic_cast<MP4BytesProperty*>(pProp)->SetValue(alac, 24, 0);
    atom->AddChildAtom(alacAtom);

    if (chan) {
        MP4ChanAtom *chanAtom = new MP4ChanAtom(*this, "chan");
        pProp = chanAtom->GetProperty(2);
        dynamic_cast<MP4BytesProperty*>(pProp)->SetValue(chan, 12, 0);
        atom->AddChildAtom(chanAtom);
    }
    return track;
}

void
MP4FileX::CreateAudioSampleGroupDescription(MP4TrackId trackId,
                                            uint32_t sampleCount)
{
    MP4Atom *stbl = FindTrackAtom(trackId, "mdia.minf.stbl");
    MP4Property *prop;

    MP4SbgpAtom * sbgp = new MP4SbgpAtom(*this, "sbgp");
    stbl->AddChildAtom(sbgp);
    sbgp->Generate();
    sbgp->FindProperty("sbgp.groupingType", &prop);
    dynamic_cast<MP4Integer32Property*>(prop)->SetValue(*(int32_t*)"roll");
    sbgp->FindProperty("sbgp.entries.sampleCount", &prop);
    dynamic_cast<MP4Integer32Property*>(prop)->AddValue(sampleCount);
    sbgp->FindProperty("sbgp.entries.groupDescriptionIndex", &prop);
    dynamic_cast<MP4Integer32Property*>(prop)->AddValue(1);
    sbgp->FindProperty("sbgp.entryCount", &prop);
    dynamic_cast<MP4Integer32Property*>(prop)->IncrementValue();

    MP4SgpdAtom * sgpd = new MP4SgpdAtom(*this, "sgpd");
    stbl->AddChildAtom(sgpd);
    sgpd->Generate();
    sgpd->FindProperty("sgpd.groupingType", &prop);
    dynamic_cast<MP4Integer32Property*>(prop)->SetValue(*(int32_t*)"roll");
    sgpd->FindProperty("sgpd.entries.rollDistance", &prop);
    dynamic_cast<MP4Integer16Property*>(prop)->AddValue(-1);
    sgpd->FindProperty("sgpd.entryCount", &prop);
    dynamic_cast<MP4Integer32Property*>(prop)->IncrementValue();
}

MP4DataAtom *
MP4FileX::CreateMetadataAtom(const char *name, itmf::BasicType typeCode)
{
    MP4Atom *pAtom = AddDescendantAtoms("moov",
            strutil::format("udta.meta.ilst.%s", name).c_str());
    if (!pAtom) return 0;
    MP4DataAtom *pDataAtom = AddChildAtomT(pAtom, "data");
    pDataAtom->typeCode.SetValue(typeCode);

    MP4Atom *pHdlrAtom = FindAtom("moov.udta.meta.hdlr");
    MP4Property *pProp;
    pHdlrAtom->FindProperty("hdlr.reserved2", &pProp);
    static const uint8_t val[12] = { 0x61, 0x70, 0x70, 0x6c, 0 };
    dynamic_cast<MP4BytesProperty*>(pProp)->SetValue(val, 12);

    return pDataAtom;
}

MP4DataAtom *MP4FileX::FindOrCreateMetadataAtom(
        const char *atom, itmf::BasicType typeCode)
{
    std::string atomstring =
        strutil::format("moov.udta.meta.ilst.%s.data", atom);
    MP4DataAtom *pAtom = FindAtomT(atomstring.c_str());
    if (!pAtom)
        pAtom = CreateMetadataAtom(atom, typeCode);
    return pAtom;
}

bool MP4FileX::SetMetadataString (const char *atom, const char *value)
{
    MP4DataAtom *pAtom = FindOrCreateMetadataAtom(atom, itmf::BT_UTF8);
    if (pAtom)
        pAtom->metadata.SetValue(reinterpret_cast<const uint8_t*>(value),
                std::strlen(value));
    return pAtom != 0;
}

bool MP4FileX::SetMetadataTrack(uint16_t track, uint16_t totalTracks)
{
    MP4DataAtom *pAtom = FindOrCreateMetadataAtom("trkn", itmf::BT_IMPLICIT);
    if (pAtom) {
        uint8_t v[8] = { 0 };
        v[2] = track >> 8;
        v[3] = track & 0xff;
        v[4] = totalTracks >> 8;
        v[5] = totalTracks & 0xff;
        pAtom->metadata.SetValue(v, 8);
    }
    return pAtom != 0;
}

bool MP4FileX::SetMetadataDisk(uint16_t disk, uint16_t totalDisks)
{
    MP4DataAtom *pAtom = FindOrCreateMetadataAtom("disk", itmf::BT_IMPLICIT);
    if (pAtom) {
        uint8_t v[6] = { 0 };
        v[2] = disk >> 8;
        v[3] = disk & 0xff;
        v[4] = totalDisks >> 8;
        v[5] = totalDisks & 0xff;
        pAtom->metadata.SetValue(v, 6);
    }
    return pAtom != 0;
}

bool MP4FileX::SetMetadataUint8(const char *atom, uint8_t value)
{
    MP4DataAtom *pAtom = FindOrCreateMetadataAtom(atom, itmf::BT_INTEGER);
    if (pAtom)
        pAtom->metadata.SetValue(&value, 1);
    return pAtom != 0;
}

bool MP4FileX::SetMetadataUint16(const char *atom, uint16_t value)
{
    MP4DataAtom *pAtom = FindOrCreateMetadataAtom(atom, itmf::BT_INTEGER);
    if (pAtom) {
        uint8_t v[2];
        v[0] = value >> 8;
        v[1] = value & 0xff;
        pAtom->metadata.SetValue(v, 2);
    }
    return pAtom != 0;
}

bool MP4FileX::SetMetadataUint32(const char *atom, uint32_t value)
{
    MP4DataAtom *pAtom = FindOrCreateMetadataAtom(atom, itmf::BT_INTEGER);
    if (pAtom) {
        uint8_t v[4];
        v[0] = value >> 24;
        v[1] = (value >> 16) & 0xff;
        v[2] = (value >> 8) & 0xff;
        v[3] = value & 0xff;
        pAtom->metadata.SetValue(v, 4);
    }
    return pAtom != 0;
}

bool MP4FileX::SetMetadataUint64(const char *atom, uint64_t value)
{
    MP4DataAtom *pAtom = FindOrCreateMetadataAtom(atom, itmf::BT_INTEGER);
    if (pAtom) {
        uint8_t v[8];
        v[0] = value >> 56;
        v[1] = (value >> 48) & 0xff;
        v[2] = (value >> 40) & 0xff;
        v[3] = (value >> 32) & 0xff;
        v[4] = (value >> 24) & 0xff;
        v[5] = (value >> 16) & 0xff;
        v[6] = (value >> 8) & 0xff;
        v[7] = value & 0xff;
        pAtom->metadata.SetValue(v, 8);
    }
    return pAtom != 0;
}

bool MP4FileX::SetMetadataGenre(const char *atom, uint16_t value)
{
    MP4DataAtom *pAtom = FindOrCreateMetadataAtom(atom, itmf::BT_IMPLICIT);
    if (pAtom) {
        uint8_t v[2];
        v[0] = value >> 8;
        v[1] = value & 0xff;
        pAtom->metadata.SetValue(v, 2);
    }
    return pAtom != 0;
}

bool MP4FileX::SetMetadataArtwork(const char *atom,
        const char *data, size_t size, itmf::BasicType typeCode)
{
    MP4Atom *covr = FindAtom("moov.udta.meta.ilst.covr");
    if (!covr) {
        AddDescendantAtoms("moov", "udta.meta.ilst.covr");
        covr = FindAtom("moov.udta.meta.ilst.covr");
        if (!covr) return false;
    }
    MP4DataAtom *pDataAtom = AddChildAtomT(covr, "data");
    if (typeCode == itmf::BT_UNDEFINED)
        typeCode = itmf::computeBasicType(data, size);
    pDataAtom->typeCode.SetValue(typeCode);
    pDataAtom->metadata.SetValue(
        reinterpret_cast<const uint8_t *>(data), size);
    return true;
}

bool MP4FileX::SetMetadataFreeForm(const char *name, const char *mean,
      const uint8_t* pValue, uint32_t valueSize, itmf::BasicType typeCode)
{
    MP4DataAtom *pDataAtom = 0;
    std::string tagname;
    for (int i = 0;; ++i) {
        tagname = strutil::format("moov.udta.meta.ilst.----[%d]", i);
        MP4Atom *pTagAtom = FindAtom(tagname.c_str());
        if (!pTagAtom)  
            break;
        MP4NameAtom *pNameAtom = FindChildAtomT(pTagAtom, "name");
        if (!pNameAtom || pNameAtom->value.CompareToString(name))
            continue;
        MP4MeanAtom *pMeanAtom = FindChildAtomT(pTagAtom, "mean");
        if (!pMeanAtom || pMeanAtom->value.CompareToString(mean))
            continue;
        MP4DataAtom *pDataAtom = FindChildAtomT(pTagAtom, "data");
        if (!pDataAtom)
            pDataAtom = AddChildAtomT(pTagAtom, "data");
        break;
    }
    if (!pDataAtom) {
        MP4Atom *pTagAtom = AddDescendantAtoms("moov", tagname.c_str() + 5);
        if (!pTagAtom) return false;

        MP4MeanAtom *pMeanAtom = AddChildAtomT(pTagAtom, "mean");
        pMeanAtom->value.SetValue(
                reinterpret_cast<const uint8_t*>(mean), std::strlen(mean));

        MP4NameAtom *pNameAtom = AddChildAtomT(pTagAtom, "name");
        pNameAtom->value.SetValue(
                reinterpret_cast<const uint8_t*>(name), std::strlen(name));

        pDataAtom = AddChildAtomT(pTagAtom, "data");
    }
    pDataAtom->typeCode.SetValue(typeCode);
    pDataAtom->metadata.SetValue(pValue, valueSize);
    return true;
}

MP4FileCopy::MP4FileCopy(MP4File *file)
        : m_mp4file(reinterpret_cast<MP4FileX*>(file)),
          m_src(reinterpret_cast<MP4FileX*>(file)->m_file),
          m_dst(0)
{
    m_nchunks = 0;
    size_t numTracks = file->GetNumberOfTracks();
    for (size_t i = 0; i < numTracks; ++i) {
        ChunkInfo ci;
        ci.current = 1;
        ci.final = m_mp4file->m_pTracks[i]->GetNumberOfChunks();
        ci.time = MP4_INVALID_TIMESTAMP;
        m_state.push_back(ci);
        m_nchunks += ci.final;
    }
}

void MP4FileCopy::start(const char *path)
{
    m_mp4file->m_file = 0;
    try {
        m_fp = std::shared_ptr<FILE>(win32::wfopenx(strutil::us2w(path).c_str(), L"wb"), fclose);
        static MP4StdIOCallbacks callbacks;
        m_mp4file->Open(path, File::MODE_CREATE, nullptr, &callbacks, m_fp.get());
    } catch (...) {
        m_mp4file->ResetFile();
        throw;
    }
    m_dst = m_mp4file->m_file;
    m_mp4file->SetIntegerProperty("moov.mvhd.modificationTime",
        mp4v2::impl::MP4GetAbsTimestamp());
    dynamic_cast<MP4RootAtom*>(m_mp4file->m_pRootAtom)->BeginOptimalWrite();
}

void MP4FileCopy::finish()
{
    if (!m_dst)
        return;
    try {
        MP4RootAtom *root = dynamic_cast<MP4RootAtom*>(m_mp4file->m_pRootAtom);
        root->FinishOptimalWrite();
    } catch (...) {
        delete m_src;
        delete m_dst;
        m_dst = 0;
        m_mp4file->m_file = 0;
        throw;
    }
    delete m_src;
    delete m_dst;
    m_dst = 0;
    m_mp4file->m_file = 0;
}

bool MP4FileCopy::copyNextChunk()
{
    uint32_t nextTrack = 0xffffffff;
    MP4Timestamp nextTime = MP4_INVALID_TIMESTAMP;
    size_t numTracks = m_mp4file->GetNumberOfTracks();
    for (size_t i = 0; i < numTracks; ++i) {
        MP4Track *track = m_mp4file->m_pTracks[i];
        if (m_state[i].current > m_state[i].final)
            continue;
        if (m_state[i].time == MP4_INVALID_TIMESTAMP) {
            MP4Timestamp time = track->GetChunkTime(m_state[i].current);
            m_state[i].time = mp4v2::impl::MP4ConvertTime(time,
                    track->GetTimeScale(), m_mp4file->GetTimeScale());
        }
        if (m_state[i].time > nextTime)
            continue;
        if (m_state[i].time == nextTime &&
                std::strcmp(track->GetType(), MP4_HINT_TRACK_TYPE))
            continue;
        nextTime = m_state[i].time;
        nextTrack = i;
    }
    if (nextTrack == 0xffffffff) return false;
    MP4Track *track = m_mp4file->m_pTracks[nextTrack];
    m_mp4file->m_file = m_src;
    uint8_t *chunk;
    uint32_t size;
    track->ReadChunk(m_state[nextTrack].current, &chunk, &size);
    m_mp4file->m_file = m_dst;
    track->RewriteChunk(m_state[nextTrack].current, chunk, size);
    MP4Free(chunk);
    m_state[nextTrack].current++;
    m_state[nextTrack].time = MP4_INVALID_TIMESTAMP;
    return true;
}
 
bool MP4FileX::GetQTChapters(std::vector<misc::chapter_t> *chapterList)
{
    MP4TrackId trackId = FindChapterTrack();
    if (trackId == MP4_INVALID_TRACK_ID)
        return false;

    std::vector<misc::chapter_t> chapters;
    MP4Track *track = GetTrack(trackId);
    uint32_t nsamples = track->GetNumberOfSamples();
    double timescale = track->GetTimeScale();
    MP4Timestamp start = 0;
    MP4Duration duration = 0;
    std::vector<uint8_t> sample;
    for (uint32_t i = 0; i < nsamples; ++i) {
        MP4SampleId sid =
            track->GetSampleIdFromTime(start + duration, true);
        uint32_t size = GetSampleSize(trackId, sid);
        if (size > sample.size()) sample.resize(size);
        uint8_t *sp = &sample[0];
        track->ReadSample(sid, &sp, &size, &start, &duration);
        const char * title = reinterpret_cast<const char *>(sp + 2);
        int titleLen = std::min((sp[0]<<8)|sp[1], MP4V2_CHAPTER_TITLE_MAX);
        std::string stitle(title, title + titleLen);
        chapters.push_back(std::make_pair(strutil::us2w(stitle),
                                          duration / timescale));
    }
    chapterList->swap(chapters);
    return chapterList->size() > 0;
}

bool MP4FileX::GetNeroChapters(std::vector<misc::chapter_t> *chapterList,
                               double *first_off)
{
    MP4Atom *chpl = FindAtom("moov.udta.chpl");
    if (!chpl)
        return false;
    MP4Property *prop;
    if (!chpl->FindProperty("chpl.chaptercount", &prop))
        return false;
    uint32_t count = dynamic_cast<MP4Integer32Property*>(prop)->GetValue();
    if (!count || !chpl->FindProperty("chpl.chapters", &prop))
        return false;
    MP4TableProperty *pTable = dynamic_cast<MP4TableProperty*>(prop);
    MP4Integer64Property *pStartTime =
        dynamic_cast<MP4Integer64Property*>(pTable->GetProperty(0));
    MP4StringProperty *pName =
        dynamic_cast<MP4StringProperty*>(pTable->GetProperty(1));
    if (!pStartTime || !pName)
        return false;
    std::vector<misc::chapter_t> chapters;
    int64_t prev = pStartTime->GetValue(0);
    double scale = 10000000.0;
    if (first_off) *first_off = prev / scale;
    const char *name = pName->GetValue(0);
    for (uint32_t i = 1; i < count; ++i) {
        int64_t start = pStartTime->GetValue(i);
        chapters.push_back(std::make_pair(strutil::us2w(name),
                                          (start - prev) / scale));
        name = pName->GetValue(i);
        prev = start;
    }
    int64_t end =
        static_cast<double>(GetDuration()) / GetTimeScale() * scale + 0.5;
    chapters.push_back(std::make_pair(strutil::us2w(name),
                                      (end - prev) / scale));
    chapterList->swap(chapters);
    return chapterList->size() > 0;
}

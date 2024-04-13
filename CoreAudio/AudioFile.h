#ifndef AudioFile_H
#define AudioFile_H

#include "CoreFoundation.h"
#include "CoreAudioTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct OpaqueAudioFileID;
typedef struct OpaqueAudioFileID *AudioFileID;

typedef UInt32 AudioFileTypeID;
typedef UInt32 AudioFilePropertyID;

struct AudioFile_SMPTE_Time {
   SInt8   mHours;
   UInt8   mMinutes;
   UInt8   mSeconds;
   UInt8   mFrames;
   UInt32  mSubFrameSampleOffset;
};
typedef struct AudioFile_SMPTE_Time AudioFile_SMPTE_Time;

struct AudioFileMarker {
   Float64 mFramePosition;
   CFStringRef           mName;
   SInt32                mMarkerID;
   AudioFile_SMPTE_Time  mSMPTETime;
   UInt32                mType;
   UInt16                mReserved;
   UInt16                mChannel;
};
typedef struct AudioFileMarker AudioFileMarker;

struct AudioFileMarkerList {
   UInt32           mSMPTE_TimeType;
   UInt32           mNumberMarkers;
   AudioFileMarker  mMarkers[1];
};
typedef struct AudioFileMarkerList AudioFileMarkerList;

struct AudioFileRegion {
   UInt32            mRegionID;
   CFStringRef       mName;
   UInt32            mFlags;
   UInt32            mNumberMarkers;
   AudioFileMarker   mMarkers[1];
};
typedef struct AudioFileRegion AudioFileRegion;

struct AudioFileRegionList {
   UInt32            mSMPTE_TimeType;
   UInt32            mNumberRegions;
   AudioFileRegion   mRegions[1];
};
typedef struct AudioFileRegionList AudioFileRegionList;

struct AudioFramePacketTranslation {
   SInt64  mFrame;
   SInt64  mPacket;
   UInt32  mFrameOffsetInPacket;
};
typedef struct AudioFramePacketTranslation AudioFramePacketTranslation;

struct AudioBytePacketTranslation {
   SInt64 mByte;
   SInt64 mPacket;
   UInt32 mByteOffsetInPacket;
   UInt32 mFlags;
};
typedef struct AudioBytePacketTranslation AudioBytePacketTranslation;

struct AudioFilePacketTableInfo {
   SInt64  mNumberValidFrames;
   SInt32  mPrimingFrames;
   SInt32  mRemainderFrames;
};
typedef struct AudioFilePacketTableInfo AudioFilePacketTableInfo;

struct AudioFileTypeAndFormatID {
   AudioFileTypeID  mFileType;
   UInt32           mFormatID;
};
typedef struct AudioFileTypeAndFormatID AudioFileTypeAndFormatID;

enum {
   kAudioFileAIFFType            = *(int32_t*)"AIFF",
   kAudioFileAIFCType            = *(int32_t*)"AIFC",
   kAudioFileWAVEType            = *(int32_t*)"WAVE",
   kAudioFileSoundDesigner2Type  = *(int32_t*)"Sd2f",
   kAudioFileNextType            = *(int32_t*)"NeXT",
   kAudioFileMP3Type             = *(int32_t*)"MPG3",
   kAudioFileMP2Type             = *(int32_t*)"MPG2",
   kAudioFileMP1Type             = *(int32_t*)"MPG1",
   kAudioFileAC3Type             = *(int32_t*)"ac-3",
   kAudioFileAAC_ADTSType        = *(int32_t*)"adts",
   kAudioFileMPEG4Type           = *(int32_t*)"mp4f",
   kAudioFileM4AType             = *(int32_t*)"m4af",
   kAudioFileCAFType             = *(int32_t*)"caff",
   kAudioFile3GPType             = *(int32_t*)"3gpp",
   kAudioFile3GP2Type            = *(int32_t*)"3gp2",
   kAudioFileAMRType             = *(int32_t*)"amrf"
};

enum {
   kAudioFilePropertyFileFormat            = *(int32_t*)"ffmt",
   kAudioFilePropertyDataFormat            = *(int32_t*)"dfmt",
   kAudioFilePropertyIsOptimized           = *(int32_t*)"optm",
   kAudioFilePropertyMagicCookieData       = *(int32_t*)"mgic",
   kAudioFilePropertyAudioDataByteCount    = *(int32_t*)"bcnt",
   kAudioFilePropertyAudioDataPacketCount  = *(int32_t*)"pcnt",
   kAudioFilePropertyMaximumPacketSize     = *(int32_t*)"psze",
   kAudioFilePropertyDataOffset            = *(int32_t*)"doff",
   kAudioFilePropertyChannelLayout         = *(int32_t*)"cmap",
   kAudioFilePropertyDeferSizeUpdates      = *(int32_t*)"dszu",
   kAudioFilePropertyDataFormatName        = *(int32_t*)"fnme",
   kAudioFilePropertyMarkerList            = *(int32_t*)"mkls",
   kAudioFilePropertyRegionList            = *(int32_t*)"rgls",
   kAudioFilePropertyPacketToFrame         = *(int32_t*)"pkfr",
   kAudioFilePropertyFrameToPacket         = *(int32_t*)"frpk",
   kAudioFilePropertyPacketToByte          = *(int32_t*)"pkby",
   kAudioFilePropertyByteToPacket          = *(int32_t*)"bypk",
   kAudioFilePropertyChunkIDs              = *(int32_t*)"chid",
   kAudioFilePropertyInfoDictionary        = *(int32_t*)"info",
   kAudioFilePropertyPacketTableInfo       = *(int32_t*)"pnfo",
   kAudioFilePropertyFormatList            = *(int32_t*)"flst",
   kAudioFilePropertyPacketSizeUpperBound  = *(int32_t*)"pkub",
   kAudioFilePropertyReserveDuration       = *(int32_t*)"rsrv",
   kAudioFilePropertyEstimatedDuration     = *(int32_t*)"edur",
   kAudioFilePropertyBitRate               = *(int32_t*)"brat",
   kAudioFilePropertyID3Tag                = *(int32_t*)"id3t",
   kAudioFilePropertySourceBitDepth        = *(int32_t*)"sbtd",
   kAudioFilePropertyAlbumArtwork          = *(int32_t*)"aart"
};

enum
{
   kAudioFileGlobalInfo_ReadableTypes                            = *(int32_t*)"afrf",
   kAudioFileGlobalInfo_WritableTypes                            = *(int32_t*)"afwf",
   kAudioFileGlobalInfo_FileTypeName                             = *(int32_t*)"ftnm",
   kAudioFileGlobalInfo_AvailableStreamDescriptionsForFormat     = *(int32_t*)"sdid",
   kAudioFileGlobalInfo_AvailableFormatIDs                       = *(int32_t*)"fmid",
   
   kAudioFileGlobalInfo_AllExtensions                            = *(int32_t*)"alxt",
   kAudioFileGlobalInfo_AllHFSTypeCodes                          = *(int32_t*)"ahfs",
   kAudioFileGlobalInfo_AllUTIs                                  = *(int32_t*)"auti",
   kAudioFileGlobalInfo_AllMIMETypes                             = *(int32_t*)"amim",
   
   kAudioFileGlobalInfo_ExtensionsForType                        = *(int32_t*)"fext",
   kAudioFileGlobalInfo_HFSTypeCodesForType                      = *(int32_t*)"fhfs",
   kAudioFileGlobalInfo_UTIsForType                              = *(int32_t*)"futi",
   kAudioFileGlobalInfo_MIMETypesForType                         = *(int32_t*)"fmim",
   
   kAudioFileGlobalInfo_TypesForMIMEType                        = *(int32_t*)"tmim",
   kAudioFileGlobalInfo_TypesForUTI                             = *(int32_t*)"tuti",
   kAudioFileGlobalInfo_TypesForHFSTypeCode                     = *(int32_t*)"thfs",
   kAudioFileGlobalInfo_TypesForExtension                       = *(int32_t*)"text"
};

enum {
   kAudioFileFlags_EraseFile                = 1,
   kAudioFileFlags_DontPageAlignAudioData   = 2
};

enum {
   kAudioFileReadPermission      = 0x01,
   kAudioFileWritePermission     = 0x02,
   kAudioFileReadWritePermission = 0x03
};

enum {
   kAudioFileLoopDirection_NoLooping = 0,
   kAudioFileLoopDirection_Forward = 1,
   kAudioFileLoopDirection_ForwardAndBackward = 2,
   kAudioFileLoopDirection_Backward = 3
};

enum {
   kAudioFileMarkerType_Generic = 0,
};

enum {
   kAudioFileRegionFlag_LoopEnable = 1,
   kAudioFileRegionFlag_PlayForward = 2,
   kAudioFileRegionFlag_PlayBackward = 4
};

enum {
   kBytePacketTranslationFlag_IsEstimate = 1
};

#define kAFInfoDictionary_Artist "artist"
#define kAFInfoDictionary_Album "album"
#define kAFInfoDictionary_Tempo "tempo"
#define kAFInfoDictionary_KeySignature "key signature"
#define kAFInfoDictionary_TimeSignature "time signature"
#define kAFInfoDictionary_TrackNumber "track number"
#define kAFInfoDictionary_Year "year"
#define kAFInfoDictionary_Composer "composer"
#define kAFInfoDictionary_Lyricist "lyricist"
#define kAFInfoDictionary_Genre "genre"
#define kAFInfoDictionary_Title "title"
#define kAFInfoDictionary_RecordedDate "recorded date"
#define kAFInfoDictionary_Comments "comments"
#define kAFInfoDictionary_Copyright "copyright"
#define kAFInfoDictionary_SourceEncoder "source encoder"
#define kAFInfoDictionary_EncodingApplication "encoding application"
#define kAFInfoDictionary_NominalBitRate "nominal bit rate"
#define kAFInfoDictionary_ChannelLayout "channel layout"
#define kAFInfoDictionary_ApproximateDurationInSeconds "approximate duration in seconds"
#define kAFInfoDictionary_SourceBitDepth "source bit depth"
#define kAFInfoDictionary_ISRC "ISRC"
#define kAFInfoDictionary_SubTitle "subtitle"

typedef SInt64 (*AudioFile_GetSizeProc)(
   void  *inClientData
);

typedef OSStatus (*AudioFile_ReadProc) (
   void     *inClientData,
   SInt64   inPosition,
   UInt32   requestCount,
   void     *buffer,
   UInt32   *actualCount
);

typedef OSStatus (*AudioFile_SetSizeProc)(
   void     *inClientData,
   SInt64   inSize
);

typedef OSStatus (*AudioFile_WriteProc) (
   void          *inClientData,
   SInt64        inPosition,
   UInt32        requestCount,
   const void    *buffer,
   UInt32        *actualCount
);


OSStatus AudioFileClose (
   AudioFileID inAudioFile
);

OSStatus AudioFileCountUserData (
   AudioFileID inAudioFile,
   UInt32      inUserDataID,
   UInt32      *outNumberItems
);

/*
OSStatus AudioFileCreateWithURL (
   CFURLRef                          inFileRef,
   AudioFileTypeID                   inFileType,
   const AudioStreamBasicDescription *inFormat,
   UInt32                            inFlags,
   AudioFileID                       *outAudioFile
);
*/

OSStatus AudioFileGetGlobalInfo (
   AudioFilePropertyID inPropertyID,
   UInt32              inSpecifierSize,
   void                *inSpecifier,
   UInt32              *ioDataSize,
   void                *outPropertyData
);

OSStatus AudioFileGetGlobalInfoSize (
   AudioFilePropertyID inPropertyID,
   UInt32              inSpecifierSize,
   void                *inSpecifier,
   UInt32              *outDataSize
);

OSStatus AudioFileGetProperty (
   AudioFileID         inAudioFile,
   AudioFilePropertyID inPropertyID,
   UInt32              *ioDataSize,
   void                *outPropertyData
);

OSStatus AudioFileGetPropertyInfo (
   AudioFileID         inAudioFile,
   AudioFilePropertyID inPropertyID,
   UInt32              *outDataSize,
   UInt32              *isWritable
);

OSStatus AudioFileGetUserData (
   AudioFileID inAudioFile,
   UInt32      inUserDataID,
   UInt32      inIndex,
   UInt32      *ioUserDataSize,
   void        *outUserData
);

OSStatus AudioFileGetUserDataSize (
   AudioFileID inAudioFile,
   UInt32      inUserDataID,
   UInt32      inIndex,
   UInt32      *outUserDataSize
);

OSStatus AudioFileInitializeWithCallbacks (
   void                              *inClientData,
   AudioFile_ReadProc                inReadFunc,
   AudioFile_WriteProc               inWriteFunc,
   AudioFile_GetSizeProc             inGetSizeFunc,
   AudioFile_SetSizeProc             inSetSizeFunc,
   AudioFileTypeID                   inFileType,
   const AudioStreamBasicDescription *inFormat,
   UInt32                            inFlags,
   AudioFileID                       *outAudioFile
);

OSStatus AudioFileOpenURL (
   CFURLRef        inFileRef,
   SInt8           inPermissions,
   AudioFileTypeID inFileTypeHint,
   AudioFileID     *outAudioFile
);

OSStatus AudioFileOpenWithCallbacks (
   void                  *inClientData,
   AudioFile_ReadProc    inReadFunc,
   AudioFile_WriteProc   inWriteFunc,
   AudioFile_GetSizeProc inGetSizeFunc,
   AudioFile_SetSizeProc inSetSizeFunc,
   AudioFileTypeID       inFileTypeHint,
   AudioFileID           *outAudioFile
);

OSStatus AudioFileOptimize (
   AudioFileID inAudioFile
);

OSStatus AudioFileReadBytes (
   AudioFileID inAudioFile,
   Boolean   inUseCache,
   SInt64    inStartingByte,
   UInt32    *ioNumBytes,
   void      *outBuffer
);

OSStatus AudioFileReadPacketData (
   AudioFileID                   inAudioFile,
   Boolean                       inUseCache,
   UInt32                        *ioNumBytes,
   AudioStreamPacketDescription  *outPacketDescriptions,
   SInt64                        inStartingPacket,
   UInt32                        *ioNumPackets,
   void                          *outBuffer
);

OSStatus AudioFileReadPackets (
   AudioFileID                  inAudioFile,
   Boolean                      inUseCache,
   UInt32                       *outNumBytes,
   AudioStreamPacketDescription *outPacketDescriptions,
   SInt64                       inStartingPacket,
   UInt32                       *ioNumPackets,
   void                         *outBuffer
);

OSStatus AudioFileRemoveUserData (
   AudioFileID inAudioFile,
   UInt32      inUserDataID,
   UInt32      inIndex
);

OSStatus AudioFileSetProperty (
    AudioFileID         inAudioFile,
    AudioFilePropertyID inPropertyID,
    UInt32              inDataSize,
    const void          *inPropertyData
);

OSStatus AudioFileSetUserData (
   AudioFileID inAudioFile,
   UInt32      inUserDataID,
   UInt32      inIndex,
   UInt32      inUserDataSize,
   const void  *inUserData
);

OSStatus AudioFileWriteBytes (
   AudioFileID inAudioFile,
   Boolean     inUseCache,
   SInt64      inStartingByte,
   UInt32      *ioNumBytes,
   const void  *inBuffer
);

OSStatus AudioFileWritePackets (
   AudioFileID                        inAudioFile,
   Boolean                            inUseCache,
   UInt32                             inNumBytes,
   const AudioStreamPacketDescription *inPacketDescriptions,
   SInt64                             inStartingPacket,
   UInt32                             *ioNumPackets,
   const void                         *inBuffer
);

#ifdef __cplusplus
}
#endif

#endif

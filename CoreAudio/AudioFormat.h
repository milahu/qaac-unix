#ifndef AudioFormat_H
#define AudioFormat_H

#include "CoreAudioTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
   // AudioStreamBasicDescription structure properties
   kAudioFormatProperty_FormatInfo                       = *(int32_t*)"fmti",
   kAudioFormatProperty_FormatName                       = *(int32_t*)"fnam",
   kAudioFormatProperty_EncodeFormatIDs                  = *(int32_t*)"acof",
   kAudioFormatProperty_DecodeFormatIDs                  = *(int32_t*)"acif",
   kAudioFormatProperty_FormatList                       = *(int32_t*)"flst",
   kAudioFormatProperty_ASBDFromESDS                     = *(int32_t*)"essd",
   kAudioFormatProperty_ChannelLayoutFromESDS            = *(int32_t*)"escl",
   kAudioFormatProperty_OutputFormatList                 = *(int32_t*)"ofls",
   kAudioFormatProperty_Encoders                         = *(int32_t*)"aven",
   kAudioFormatProperty_Decoders                         = *(int32_t*)"avde",
   kAudioFormatProperty_FormatIsVBR                      = *(int32_t*)"fvbr",
   kAudioFormatProperty_FormatIsExternallyFramed         = *(int32_t*)"fexf",
   kAudioFormatProperty_AvailableEncodeBitRates          = *(int32_t*)"aebr",
   kAudioFormatProperty_AvailableEncodeSampleRates       = *(int32_t*)"aesr",
   kAudioFormatProperty_AvailableEncodeChannelLayoutTags = *(int32_t*)"aecl",
   kAudioFormatProperty_AvailableEncodeNumberChannels    = *(int32_t*)"avnc",
   kAudioFormatProperty_ASBDFromMPEGPacket               = *(int32_t*)"admp",
   //
   // AudioChannelLayout structure properties
   kAudioFormatProperty_BitmapForLayoutTag               = *(int32_t*)"bmtg",
   kAudioFormatProperty_MatrixMixMap                     = *(int32_t*)"mmap",
   kAudioFormatProperty_ChannelMap                       = *(int32_t*)"chmp",
   kAudioFormatProperty_NumberOfChannelsForLayout        = *(int32_t*)"nchm",
   kAudioFormatProperty_ValidateChannelLayout            = *(int32_t*)"vacl",
   kAudioFormatProperty_ChannelLayoutForTag              = *(int32_t*)"cmpl",
   kAudioFormatProperty_TagForChannelLayout              = *(int32_t*)"cmpt",
   kAudioFormatProperty_ChannelLayoutName                = *(int32_t*)"lonm",
   kAudioFormatProperty_ChannelLayoutSimpleName          = *(int32_t*)"lsnm",
   kAudioFormatProperty_ChannelLayoutForBitmap           = *(int32_t*)"cmpb",
   kAudioFormatProperty_ChannelName                      = *(int32_t*)"cnam",
   kAudioFormatProperty_ChannelShortName                 = *(int32_t*)"csnm",
   kAudioFormatProperty_TagsForNumberOfChannels          = *(int32_t*)"tagc",
   kAudioFormatProperty_PanningMatrix                    = *(int32_t*)"panm",
   kAudioFormatProperty_BalanceFade                      = *(int32_t*)"balf",
   //
   // ID3 tag (MP3 metadata) properties
   kAudioFormatProperty_ID3TagSize                       = *(int32_t*)"id3s",
   kAudioFormatProperty_ID3TagToDictionary               = *(int32_t*)"id3d"
};

typedef UInt32 AudioFormatPropertyID;

struct AudioFormatListItem
{
    AudioStreamBasicDescription         mASBD;
    AudioChannelLayoutTag               mChannelLayoutTag;
};
typedef struct AudioFormatListItem AudioFormatListItem;

OSStatus AudioFormatGetProperty (
   AudioFormatPropertyID inPropertyID,
   UInt32                inSpecifierSize,
   const void            *inSpecifier,
   UInt32                *ioPropertyDataSize,
   void                  *outPropertyData
);

OSStatus AudioFormatGetPropertyInfo (
   AudioFormatPropertyID  inPropertyID,
   UInt32                 inSpecifierSize,
   const void             *inSpecifier,
   UInt32                 *outPropertyDataSize
);

#ifdef __cplusplus
}
#endif

#endif

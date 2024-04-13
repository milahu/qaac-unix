#ifndef AudioConverter_H
#define AudioConverter_H

#include "CoreAudioTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct OpaqueAudioConverter;
typedef struct OpaqueAudioConverter *AudioConverterRef;
typedef UInt32 AudioConverterPropertyID;

struct AudioConverterPrimeInfo {
   UInt32 leadingFrames;
   UInt32 trailingFrames;
};
typedef struct AudioConverterPrimeInfo AudioConverterPrimeInfo;

enum {
   kConverterPrimeMethod_Pre     = 0,
   kConverterPrimeMethod_Normal  = 1,
   kConverterPrimeMethod_None    = 2
};

enum {
   kAudioConverterPropertyMinimumInputBufferSize    = *(int32_t*)"mibs",
   kAudioConverterPropertyMinimumOutputBufferSize   = *(int32_t*)"mobs",
   kAudioConverterPropertyMaximumInputBufferSize    = *(int32_t*)"xibs",
   kAudioConverterPropertyMaximumInputPacketSize    = *(int32_t*)"xips",
   kAudioConverterPropertyMaximumOutputPacketSize   = *(int32_t*)"xops",
   kAudioConverterPropertyCalculateInputBufferSize  = *(int32_t*)"cibs",
   kAudioConverterPropertyCalculateOutputBufferSize = *(int32_t*)"cobs",
   kAudioConverterPropertyInputCodecParameters      = *(int32_t*)"icdp",
   kAudioConverterPropertyOutputCodecParameters     = *(int32_t*)"ocdp",
   kAudioConverterSampleRateConverterAlgorithm      = *(int32_t*)"srci",
   kAudioConverterSampleRateConverterComplexity     = *(int32_t*)"srca",
   kAudioConverterSampleRateConverterQuality        = *(int32_t*)"srcq",
   kAudioConverterSampleRateConverterInitialPhase   = *(int32_t*)"srcp",
   kAudioConverterCodecQuality                      = *(int32_t*)"cdqu",
   kAudioConverterPrimeMethod                       = *(int32_t*)"prmm",
   kAudioConverterPrimeInfo                         = *(int32_t*)"prim",
   kAudioConverterChannelMap                        = *(int32_t*)"chmp",
   kAudioConverterDecompressionMagicCookie          = *(int32_t*)"dmgc",
   kAudioConverterCompressionMagicCookie            = *(int32_t*)"cmgc",
   kAudioConverterEncodeBitRate                     = *(int32_t*)"brat",
   kAudioConverterEncodeAdjustableSampleRate        = *(int32_t*)"ajsr",
   kAudioConverterInputChannelLayout                = *(int32_t*)"icl ",
   kAudioConverterOutputChannelLayout               = *(int32_t*)"ocl ",
   kAudioConverterApplicableEncodeBitRates          = *(int32_t*)"aebr",
   kAudioConverterAvailableEncodeBitRates           = *(int32_t*)"vebr",
   kAudioConverterApplicableEncodeSampleRates       = *(int32_t*)"aesr",
   kAudioConverterAvailableEncodeSampleRates        = *(int32_t*)"vesr",
   kAudioConverterAvailableEncodeChannelLayoutTags  = *(int32_t*)"aecl",
   kAudioConverterCurrentOutputStreamDescription    = *(int32_t*)"acod",
   kAudioConverterCurrentInputStreamDescription     = *(int32_t*)"acid",
   kAudioConverterPropertySettings                  = *(int32_t*)"acps",
   kAudioConverterPropertyBitDepthHint              = *(int32_t*)"acbd",
   kAudioConverterPropertyFormatList                = *(int32_t*)"flst",
};

enum {
   kAudioConverterQuality_Max     = 0x7F,
   kAudioConverterQuality_High    = 0x60,
   kAudioConverterQuality_Medium  = 0x40,
   kAudioConverterQuality_Low     = 0x20,
   kAudioConverterQuality_Min     = 0
};

enum {
   kAudioConverterSampleRateConverterComplexity_Linear     = *(int32_t*)"line",
   kAudioConverterSampleRateConverterComplexity_Normal     = *(int32_t*)"norm",
   kAudioConverterSampleRateConverterComplexity_Mastering  = *(int32_t*)"bats",
};

typedef OSStatus (*AudioConverterComplexInputDataProc) (
   AudioConverterRef             inAudioConverter,
   UInt32                        *ioNumberDataPackets,
   AudioBufferList               *ioData,
   AudioStreamPacketDescription  **outDataPacketDescription,
   void                          *inUserData
);

OSStatus AudioConverterNew (
   const AudioStreamBasicDescription    *inSourceFormat,
   const AudioStreamBasicDescription    *inDestinationFormat,
   AudioConverterRef                    *outAudioConverter
);

OSStatus AudioConverterDispose (
   AudioConverterRef inAudioConverter
);

OSStatus AudioConverterReset (
   AudioConverterRef inAudioConverter
);

OSStatus AudioConverterGetProperty(
   AudioConverterRef         inAudioConverter,
   AudioConverterPropertyID  inPropertyID,
   UInt32                    *ioPropertyDataSize,
   void                      *outPropertyData
);

OSStatus AudioConverterGetPropertyInfo(
   AudioConverterRef         inAudioConverter,
   AudioConverterPropertyID  inPropertyID,
   UInt32                    *outSize,
   Boolean                   *outWritable
);

OSStatus AudioConverterSetProperty(
   AudioConverterRef        inAudioConverter,
   AudioConverterPropertyID inPropertyID,
   UInt32                   inPropertyDataSize,
   const void               *inPropertyData
);

OSStatus AudioConverterFillComplexBuffer(
   AudioConverterRef                   inAudioConverter,
   AudioConverterComplexInputDataProc  inInputDataProc,
   void                                *inInputDataProcUserData,
   UInt32                              *ioOutputDataPacketSize,
   AudioBufferList                     *outOutputData,
   AudioStreamPacketDescription        *outPacketDescription
);

#ifdef __cplusplus
}
#endif

#endif

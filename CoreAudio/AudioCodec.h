#ifndef AudioCodec_H
#define AudioCodec_H

#include "CoreAudioTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
   kAudioCodecPropertyNameCFString                        = *(int32_t*)"lnam",
   kAudioCodecPropertyManufacturerCFString                = *(int32_t*)"lmak",
   kAudioCodecPropertyFormatCFString                      = *(int32_t*)"lfor",
   //kAudioCodecPropertyHasVariablePacketByteSizes          = *(int32_t*)"vpk?",
   kAudioCodecPropertySupportedInputFormats               = *(int32_t*)"ifm#",
   kAudioCodecPropertySupportedOutputFormats              = *(int32_t*)"ofm#",
   kAudioCodecPropertyAvailableInputSampleRates           = *(int32_t*)"aisr",
   kAudioCodecPropertyAvailableOutputSampleRates          = *(int32_t*)"aosr",
   kAudioCodecPropertyAvailableBitRateRange               = *(int32_t*)"abrt",
   kAudioCodecPropertyMinimumNumberInputPackets           = *(int32_t*)"mnip",
   kAudioCodecPropertyMinimumNumberOutputPackets          = *(int32_t*)"mnop",
   kAudioCodecPropertyAvailableNumberChannels             = *(int32_t*)"cmnc",
   kAudioCodecPropertyDoesSampleRateConversion            = *(int32_t*)"lmrc",
   kAudioCodecPropertyAvailableInputChannelLayoutTags     = *(int32_t*)"aicl",
   kAudioCodecPropertyAvailableOutputChannelLayoutTags    = *(int32_t*)"aocl",
   kAudioCodecPropertyInputFormatsForOutputFormat         = *(int32_t*)"if4o",
   kAudioCodecPropertyOutputFormatsForInputFormat         = *(int32_t*)"of4i",
   kAudioCodecPropertyFormatInfo                          = *(int32_t*)"acfi",
};

enum {
   kAudioCodecPropertyInputBufferSize               = *(int32_t*)"tbuf",
   kAudioCodecPropertyPacketFrameSize               = *(int32_t*)"pakf",
   kAudioCodecPropertyMaximumPacketByteSize         = *(int32_t*)"pakb",
   kAudioCodecPropertyCurrentInputFormat            = *(int32_t*)"ifmt",
   kAudioCodecPropertyCurrentOutputFormat           = *(int32_t*)"ofmt",
   kAudioCodecPropertyMagicCookie                   = *(int32_t*)"kuki",
   kAudioCodecPropertyUsedInputBufferSize           = *(int32_t*)"ubuf",
   kAudioCodecPropertyIsInitialized                 = *(int32_t*)"init",
   kAudioCodecPropertyCurrentTargetBitRate          = *(int32_t*)"brat",
   kAudioCodecPropertyCurrentInputSampleRate        = *(int32_t*)"cisr",
   kAudioCodecPropertyCurrentOutputSampleRate       = *(int32_t*)"cosr",
   kAudioCodecPropertyQualitySetting                = *(int32_t*)"srcq",
   kAudioCodecPropertyApplicableBitRateRange        = *(int32_t*)"brta",
   kAudioCodecPropertyApplicableInputSampleRates    = *(int32_t*)"isra",
   kAudioCodecPropertyApplicableOutputSampleRates   = *(int32_t*)"osra",
   kAudioCodecPropertyPaddedZeros                   = *(int32_t*)"pad0",
   kAudioCodecPropertyPrimeMethod                   = *(int32_t*)"prmm",
   kAudioCodecPropertyPrimeInfo                     = *(int32_t*)"prim",
   kAudioCodecPropertyCurrentInputChannelLayout     = *(int32_t*)"icl ",
   kAudioCodecPropertyCurrentOutputChannelLayout    = *(int32_t*)"ocl ",
   kAudioCodecPropertySettings                      = *(int32_t*)"acs ",
   kAudioCodecPropertyFormatList                    = *(int32_t*)"acfl",
   kAudioCodecPropertyBitRateControlMode            = *(int32_t*)"acbf",
   kAudioCodecPropertySoundQualityForVBR            = *(int32_t*)"vbrq",
   kAudioCodecPropertyMinimumDelayMode              = *(int32_t*)"mdel"
};

enum {
   kAudioCodecBitRateControlMode_Constant                   = 0,
   kAudioCodecBitRateControlMode_LongTermAverage            = 1,
   kAudioCodecBitRateControlMode_VariableConstrained        = 2,
   kAudioCodecBitRateControlMode_Variable                   = 3,
};

typedef ComponentInstance    AudioCodec;
typedef UInt32 AudioCodecPropertyID;

typedef struct AudioCodecPrimeInfo {
   UInt32        leadingFrames;
   UInt32        trailingFrames;
} AudioCodecPrimeInfo;

ComponentResult AudioCodecGetProperty (
   AudioCodec inCodec,
   AudioCodecPropertyID inPropertyID,
   UInt32 *ioPropertyDataSize,
   void *outPropertyData);

ComponentResult AudioCodecGetPropertyInfo (
   AudioCodec inCodec,
   AudioCodecPropertyID inPropertyID,
   UInt32 *outSize,
   Boolean *outWritable);

ComponentResult AudioCodecInitialize (
   AudioCodec inCodec,
   const AudioStreamBasicDescription *inInputFormat,
   const AudioStreamBasicDescription *inOutputFormat,
   const void *inMagicCookie,
   UInt32 inMagicCookieByteSize);

ComponentResult AudioCodecSetProperty (
   AudioCodec inCodec,
   AudioCodecPropertyID inPropertyID,
   UInt32 inPropertyDataSize,
   const void *inPropertyData);

ComponentResult AudioCodecUninitialize(AudioCodec inCodec);

#ifdef __cplusplus
}
#endif

#endif

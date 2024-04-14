/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

//
//  EndianPortable.h
//
//  Copyright 2011 Apple Inc. All rights reserved.
//

#ifndef _EndianPortable_h
#define _EndianPortable_h

#include <stdint.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#if defined(__llvm__)
#define use_GCC_bswap
#elif defined(__GNUC__)
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ > 2)
#define use_GCC_bswap
#endif
#endif

#ifdef _MSC_VER
#define BSWAP16 _byteswap_ushort
#define BSWAP32 _byteswap_ulong
#define BSWAP64 _byteswap_uint64
#elif defined(use_GCC_bswap)
#define BSWAP16 __builtin_bswap16
#define BSWAP32 __builtin_bswap32
#define BSWAP64 __builtin_bswap64
#else
#define BSWAP16(x) (((x << 8) | ((x >> 8) & 0x00ff)))
#define BSWAP32(x) (((x << 24) | ((x << 8) & 0x00ff0000) | ((x >> 8) & 0x0000ff00) | ((x >> 24) & 0x000000ff)))
#define BSWAP64(x) ((((int64_t)x << 56) | (((int64_t)x << 40) & 0x00ff000000000000LL) | \
                    (((int64_t)x << 24) & 0x0000ff0000000000LL) | (((int64_t)x << 8) & 0x000000ff00000000LL) | \
                    (((int64_t)x >> 8) & 0x00000000ff000000LL) | (((int64_t)x >> 24) & 0x0000000000ff0000LL) | \
                    (((int64_t)x >> 40) & 0x000000000000ff00LL) | (((int64_t)x >> 56) & 0x00000000000000ffLL)))
#endif

#if defined(__i386__)
#define TARGET_RT_LITTLE_ENDIAN 1
#elif defined(__x86_64__)
#define TARGET_RT_LITTLE_ENDIAN 1
#elif defined (TARGET_OS_WIN32) || defined(_WIN32)
#define TARGET_RT_LITTLE_ENDIAN 1
#else

#ifdef __BYTE_ORDER__

#if  __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TARGET_RT_LITTLE_ENDIAN 1
#endif

#else

// Unknown byte order! Presume little endian.
#define TARGET_RT_LITTLE_ENDIAN 1

#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

uint16_t Swap16NtoB(uint16_t inUInt16);
uint16_t Swap16BtoN(uint16_t inUInt16);

uint32_t Swap32NtoB(uint32_t inUInt32);
uint32_t Swap32BtoN(uint32_t inUInt32);

uint64_t Swap64BtoN(uint64_t inUInt64);
uint64_t Swap64NtoB(uint64_t inUInt64);

float SwapFloat32BtoN(float in);
float SwapFloat32NtoB(float in);

double SwapFloat64BtoN(double in);
double SwapFloat64NtoB(double in);

void Swap16(uint16_t * inUInt16);
void Swap24(uint8_t * inUInt24);
void Swap32(uint32_t * inUInt32);

#ifdef __cplusplus
}
#endif

#endif

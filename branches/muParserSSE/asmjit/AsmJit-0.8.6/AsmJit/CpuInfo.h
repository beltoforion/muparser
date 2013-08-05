// AsmJit - Complete JIT Assembler for C++ Language.

// Copyright (c) 2008-2009, Petr Kobalicek <kobalicek.petr@gmail.com>
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

// [Guard]
#ifndef _ASMJIT_CPUINFO_H
#define _ASMJIT_CPUINFO_H

// [Dependencies]
#include "Build.h"

// [Warnings-Push]
#include "WarningsPush.h"

namespace AsmJit {

//! @addtogroup AsmJit_CpuInfo
//! @{

// ============================================================================
// [AsmJit::CpuId]
// ============================================================================

#if defined(ASMJIT_X86) || defined(ASMJIT_X64)
//! @brief Structure (union) used by cpuid() function.
union CpuId
{
  //! @brief cpuid results array(eax, ebx, ecx and edx registers).
  UInt32 i[4];

  struct
  {
    //! @brief cpuid result in eax register.
    UInt32 eax;
    //! @brief cpuid result in ebx register.
    UInt32 ebx;
    //! @brief cpuid result in ecx register.
    UInt32 ecx;
    //! @brief cpuid result in edx register.
    UInt32 edx;
  };
};

//! @brief Calls CPUID instruction with eax == @a in and returns result to @a out.
//!
//! @c cpuid() function has one input parameter that is passed to cpuid through 
//! eax register and results in four output values representing result of cpuid 
//! instruction (eax, ebx, ecx and edx registers).
ASMJIT_API void cpuid(UInt32 in, CpuId* out) ASMJIT_NOTHROW;
#endif // ASMJIT_X86 || ASMJIT_X64

// ============================================================================
// [AsmJit::CpuInfo]
// ============================================================================

//! @brief Informations about host cpu.
struct ASMJIT_HIDDEN CpuInfo
{
  //! @brief Cpu short vendor string.
  char vendor[16];
  //! @brief Cpu vendor id (see @c AsmJit::CpuInfo::VendorId enum).
  UInt32 vendorId;
  //! @brief Cpu family ID.
  UInt32 family;
  //! @brief Cpu model ID.
  UInt32 model;
  //! @brief Cpu stepping.
  UInt32 stepping;
  //! @brief Number of processors or cores.
  UInt32 numberOfProcessors;
  //! @brief Cpu features bitfield, see @c AsmJit::CpuInfo::Feature enum).
  UInt32 features;
  //! @brief Cpu bugs bitfield, see @c AsmJit::CpuInfo::Bug enum).
  UInt32 bugs;

  //! @brief Cpu vendor IDs.
  //!
  //! Cpu vendor IDs are specific for AsmJit library. Vendor ID is not directly
  //! read from cpuid result, instead it's based on CPU vendor string.
  enum VendorId
  {
    //! @brief Unknown vendor.
    Vendor_Unknown = 0,
    //! @brief Intel vendor (GenuineIntel vendor string).
    Vendor_INTEL = 1,
    //! @brief AMD vendor (AuthenticAMD or alternatively AMDisbetter! vendor strings).
    Vendor_AMD = 2,
    //! @brief VIA vendor (VIA VIA VIA vendor string).
    Vendor_VIA = 3
  };

#if defined(ASMJIT_X86) || defined(ASMJIT_X64)
  //! @brief Extended informations for x86/x64 compatible processors.
  struct X86ExtendedInfo
  {
    UInt32 processorType;
    UInt32 brandIndex;
    UInt32 clFlushCacheLineSize;
    UInt32 logicalProcessors;
    UInt32 apicPhysicalId;
  };
  X86ExtendedInfo x86ExtendedInfo;

  //! @brief X86/X64 CPU features.
  enum Feature
  {
    //! @brief Cpu has RDTSC instruction.
    Feature_RDTSC = 1U << 0,
    //! @brief Cpu has RDTSCP instruction.
    Feature_RDTSCP = 1U << 1,
    //! @brief Cpu has CMOV instruction (conditional move)
    Feature_CMOV = 1U << 2,
    //! @brief Cpu has CMPXCHG8B instruction
    Feature_CMPXCHG8B = 1U << 3,
    //! @brief Cpu has CMPXCHG16B instruction (64 bit processors)
    Feature_CMPXCHG16B = 1U << 4,
    //! @brief Cpu has CLFUSH instruction
    Feature_CLFLUSH = 1U << 5,
    //! @brief Cpu has PREFETCH instruction
    Feature_PREFETCH = 1U << 6,
    //! @brief Cpu supports LAHF and SAHF instrictions.
    Feature_LAHF_SAHF = 1U << 7,
    //! @brief Cpu supports FXSAVE and FXRSTOR instructions.
    Feature_FXSR = 1U << 8,
    //! @brief Cpu supports FXSAVE and FXRSTOR instruction optimizations (FFXSR).
    Feature_FFXSR = 1U << 9,

    //! @brief Cpu has MMX.
    Feature_MMX = 1U << 10,
    //! @brief Cpu has extended MMX.
    Feature_MMXExt = 1U << 11,
    //! @brief Cpu has 3dNow!
    Feature_3dNow = 1U << 12,
    //! @brief Cpu has enchanced 3dNow!
    Feature_3dNowExt = 1U << 13,
    //! @brief Cpu has SSE.
    Feature_SSE = 1U << 14,
    //! @brief Cpu has Misaligned SSE (MSSE).
    Feature_MSSE = 1U << 15,
    //! @brief Cpu has SSE2.
    Feature_SSE2 = 1U << 16,
    //! @brief Cpu has SSE3.
    Feature_SSE3 = 1U << 17,
    //! @brief Cpu has Supplemental SSE3 (SSSE3).
    Feature_SSSE3 = 1U << 18,
    //! @brief Cpu has SSE4.A.
    Feature_SSE4_A = 1U << 19,
    //! @brief Cpu has SSE4.1.
    Feature_SSE4_1 = 1U << 20,
    //! @brief Cpu has SSE4.2.
    Feature_SSE4_2 = 1U << 21,
    //! @brief Cpu has SSE5.
    Feature_SSE5 = 1U << 22,
    //! @brief Cpu supports MONITOR and MWAIT instructions.
    Feature_MonitorMWait = 1U << 23,
    //! @brief Cpu supports POPCNT instruction.
    Feature_POPCNT = 1U << 24,
    //! @brief Cpu supports LZCNT instruction.
    Feature_LZCNT  = 1U << 25,
    //! @brief Cpu supports multithreading.
    Feature_MultiThreading = 1U << 29,
    //! @brief Cpu supports execute disable bit (execute protection).
    Feature_ExecuteDisableBit = 1U << 30,
    //! @brief Cpu supports 64 bits.
    Feature_64Bit = 1U << 31
  };

  //! @brief X86/X64 CPU bugs.
  enum Bug
  {
    Bug_AmdLockMB = 1U << 0
  };

#endif // ASMJIT_X86 || ASMJIT_X64
};

//! @brief Detect CPU features to CpuInfo structure @a i.
//!
//! @sa @c CpuInfo.
ASMJIT_API void detectCpuInfo(CpuInfo* i) ASMJIT_NOTHROW;

//! @brief Return CpuInfo (detection is done only once).
//!
//! @sa @c CpuInfo.
ASMJIT_API CpuInfo* cpuInfo() ASMJIT_NOTHROW;

//! @}

} // AsmJit namespace

// [Warnings-Pop]
#include "WarningsPop.h"

// [Guard]
#endif // _ASMJIT_CPUINFO_H

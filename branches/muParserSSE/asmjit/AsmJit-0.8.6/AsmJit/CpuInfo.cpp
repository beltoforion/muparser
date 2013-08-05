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

// [Dependencies]
#include "CpuInfo.h"

#if defined(ASMJIT_WINDOWS)
# include <windows.h>
#endif // ASMJIT_WINDOWS

// 2009-02-05: Thanks to Mike Tajmajer for supporting VC7.1 compiler. This
// shouldn't affect x64 compilation, because x64 compiler starts with
// VS2005 (VC8.0).
#if defined(_MSC_VER)
# if _MSC_VER >= 1400
#  include <intrin.h>
# endif // _MSC_VER >= 1400 (>= VS2005)
#endif // _MSC_VER

#if defined(ASMJIT_POSIX)
#include <errno.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif // ASMJIT_POSIX

// [Warnings-Push]
#include "WarningsPush.h"

// helpers
namespace AsmJit {

static UInt32 detectNumberOfProcessors(void)
{
#if defined(ASMJIT_WINDOWS)
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return info.dwNumberOfProcessors;
#elif defined(ASMJIT_POSIX) && defined(_SC_NPROCESSORS_ONLN)
  // It seems that sysconf returns the number of "logical" processors on both
  // mac and linux.  So we get the number of "online logical" processors.
  long res = sysconf(_SC_NPROCESSORS_ONLN);
  if (res == -1) return 1;

  return static_cast<UInt32>(res);
#else
  return 1;
#endif
}

// This is messy, I know. cpuid is implemented as intrinsic in VS2005, but
// we should support other compilers as well. Main problem is that MS compilers
// in 64 bit mode not allows to use inline assembler, so we need intrinsic and
// we need also asm version.

// cpuid() and detectCpuInfo() for x86 and x64 platforms begins here.
#if defined(ASMJIT_X86) || defined(ASMJIT_X64)
void cpuid(UInt32 in, CpuId* out) ASMJIT_NOTHROW
{
#if defined(_MSC_VER)

// 2009-02-05: Thanks to Mike Tajmajer for supporting VC7.1 compiler.
// ASMJIT_X64 is here only for readibility, only VS2005 can compile 64 bit code.
# if _MSC_VER >= 1400 || defined(ASMJIT_X64)
  // done by intrinsics
  __cpuid(reinterpret_cast<int*>(out->i), in);
# else // _MSC_VER < 1400
  UInt32 cpuid_in = in;
  UInt32* cpuid_out = out->i;

  __asm
  {
    mov     eax, cpuid_in
    mov     edi, cpuid_out
    cpuid
    mov     dword ptr[edi +  0], eax
    mov     dword ptr[edi +  4], ebx
    mov     dword ptr[edi +  8], ecx
    mov     dword ptr[edi + 12], edx
  }
# endif // _MSC_VER < 1400

#elif defined(__GNUC__)

// Note, need to preserve ebx/rbx register!
# if defined(ASMJIT_X86)
#  define __mycpuid(a, b, c, d, inp) \
  asm ("mov %%ebx, %%edi\n"    \
       "cpuid\n"               \
       "xchg %%edi, %%ebx\n"   \
       : "=a" (a), "=D" (b), "=c" (c), "=d" (d) : "a" (inp))
# else
#  define __mycpuid(a, b, c, d, inp) \
  asm ("mov %%rbx, %%rdi\n"    \
       "cpuid\n"               \
       "xchg %%rdi, %%rbx\n"   \
       : "=a" (a), "=D" (b), "=c" (c), "=d" (d) : "a" (inp))
# endif
  __mycpuid(out->eax, out->ebx, out->ecx, out->edx, in);

#endif // compiler
}

struct VendorInfo
{
  UInt32 id;
  char text[12];
};

static const VendorInfo vendorInfo[] = 
{
  { CpuInfo::Vendor_INTEL, { 'G', 'e', 'n', 'u', 'i', 'n', 'e', 'I', 'n', 't', 'e', 'l' } },
  { CpuInfo::Vendor_AMD  , { 'A', 'M', 'D', 'i', 's', 'b', 'e', 't', 't', 'e', 'r', '!' } },
  { CpuInfo::Vendor_AMD  , { 'A', 'u', 't', 'h', 'e', 'n', 't', 'i', 'c', 'A', 'M', 'D' } },
  { CpuInfo::Vendor_VIA  , { 'V', 'I', 'A', '\0','V', 'I', 'A', '\0','V', 'I', 'A', '\0'} }
};

void detectCpuInfo(CpuInfo* i) ASMJIT_NOTHROW
{
  UInt32 a;

  // First clear our struct
  memset(i, 0, sizeof(CpuInfo));
  memcpy(i->vendor, "Unknown", 8);

  i->numberOfProcessors = detectNumberOfProcessors();

#if defined(ASMJIT_X86) || defined(ASMJIT_X64)
  CpuId out;

  // Get vendor string
  cpuid(0, &out);

  memcpy(i->vendor, &out.ebx, 4);
  memcpy(i->vendor + 4, &out.edx, 4);
  memcpy(i->vendor + 8, &out.ecx, 4);

  for (a = 0; a < 3; a++)
  {
    if (memcmp(i->vendor, vendorInfo[a].text, 12) == 0)
    {
      i->vendorId = vendorInfo[a].id;
      break;
    }
  }

  // get feature flags in ecx/edx, and family/model in eax
  cpuid(1, &out);

  // family and model fields
  i->family   = (out.eax >> 8) & 0x0F;
  i->model    = (out.eax >> 4) & 0x0F;
  i->stepping = (out.eax     ) & 0x0F;

  // use extended family and model fields
  if (i->family == 0x0F)
  {
    i->family += ((out.eax >> 20) & 0xFF);
    i->model  += ((out.eax >> 16) & 0x0F) << 4;
  }

  i->x86ExtendedInfo.processorType         = ((out.eax >> 12) & 0x03);
  i->x86ExtendedInfo.brandIndex            = ((out.ebx      ) & 0xFF);
  i->x86ExtendedInfo.clFlushCacheLineSize  = ((out.ebx >>  8) & 0xFF) * 8;
  i->x86ExtendedInfo.logicalProcessors     = ((out.ebx >> 16) & 0xFF);
  i->x86ExtendedInfo.apicPhysicalId        = ((out.ebx >> 24) & 0xFF);

  if (out.ecx & 0x00000001U) i->features |= CpuInfo::Feature_SSE3;
  if (out.ecx & 0x00000008U) i->features |= CpuInfo::Feature_MonitorMWait;
  if (out.ecx & 0x00000200U) i->features |= CpuInfo::Feature_SSSE3;
  if (out.ecx & 0x00002000U) i->features |= CpuInfo::Feature_CMPXCHG16B;
  if (out.ecx & 0x00080000U) i->features |= CpuInfo::Feature_SSE4_1;
  if (out.ecx & 0x00100000U) i->features |= CpuInfo::Feature_SSE4_2;
  if (out.ecx & 0x00800000U) i->features |= CpuInfo::Feature_POPCNT;

  if (out.edx & 0x00000010U) i->features |= CpuInfo::Feature_RDTSC;
  if (out.edx & 0x00000100U) i->features |= CpuInfo::Feature_CMPXCHG8B;
  if (out.edx & 0x00008000U) i->features |= CpuInfo::Feature_CMOV;
  if (out.edx & 0x00800000U) i->features |= CpuInfo::Feature_MMX;
  if (out.edx & 0x01000000U) i->features |= CpuInfo::Feature_FXSR;
  if (out.edx & 0x02000000U) i->features |= CpuInfo::Feature_SSE | CpuInfo::Feature_MMXExt;
  if (out.edx & 0x04000000U) i->features |= CpuInfo::Feature_SSE | CpuInfo::Feature_SSE2;
  if (out.edx & 0x10000000U) i->features |= CpuInfo::Feature_MultiThreading;

  if (i->vendorId == CpuInfo::Vendor_AMD && (out.edx & 0x10000000U))
  {
    // AMD sets Multithreading to ON if it has more cores.
    if (i->numberOfProcessors == 1) i->numberOfProcessors = 2;
  }

  // This comment comes from V8 and I think that its important:
  //
  // Opteron Rev E has a bug in which on very rare occasions a locked
  // instruction doesn't act as a read-acquire barrier if followed by a
  // non-locked read-modify-write instruction.  Rev F has this bug in 
  // pre-release versions, but not in versions released to customers,
  // so we test only for Rev E, which is family 15, model 32..63 inclusive.

  if (i->vendorId == CpuInfo::Vendor_AMD && i->family == 15 && i->model >= 32 && i->model <= 63) 
  {
    i->bugs |= CpuInfo::Bug_AmdLockMB;
  }

  // Calling cpuid with 0x80000000 as the in argument
  // gets the number of valid extended IDs.

  cpuid(0x80000000, &out);

  UInt32 exIds = out.eax;

  for (a = 0x80000001; a < exIds && a <= (0x80000001); a++)
  {
    cpuid(a, &out);

    switch (a)
    {
      case 0x80000001:
        if (out.ecx & 0x00000001U) i->features |= CpuInfo::Feature_LAHF_SAHF;
        if (out.ecx & 0x00000020U) i->features |= CpuInfo::Feature_LZCNT;
        if (out.ecx & 0x00000040U) i->features |= CpuInfo::Feature_SSE4_A;
        if (out.ecx & 0x00000080U) i->features |= CpuInfo::Feature_MSSE;
        if (out.ecx & 0x00000100U) i->features |= CpuInfo::Feature_PREFETCH;
        if (out.ecx & 0x00000800U) i->features |= CpuInfo::Feature_SSE5;

        if (out.edx & 0x00100000U) i->features |= CpuInfo::Feature_ExecuteDisableBit;
        if (out.edx & 0x00200000U) i->features |= CpuInfo::Feature_FFXSR;
        if (out.edx & 0x00400000U) i->features |= CpuInfo::Feature_MMXExt;
        if (out.edx & 0x08000000U) i->features |= CpuInfo::Feature_RDTSCP;
        if (out.edx & 0x20000000U) i->features |= CpuInfo::Feature_64Bit;
        if (out.edx & 0x40000000U) i->features |= CpuInfo::Feature_3dNowExt | CpuInfo::Feature_MMXExt;
        if (out.edx & 0x80000000U) i->features |= CpuInfo::Feature_3dNow;

        break;

      // there are more informations that can be implemented in the future
    }
  }

#endif // ASMJIT_X86 || ASMJIT_X64
}
#else
void detectCpuInfo(CpuInfo* i) ASMJIT_NOTHROW
{
  memset(i, 0, sizeof(CpuInfo));
}
#endif

struct ASMJIT_HIDDEN CpuInfoStatic
{
  CpuInfoStatic() ASMJIT_NOTHROW { detectCpuInfo(&i); }

  CpuInfo i;
};

CpuInfo* cpuInfo() ASMJIT_NOTHROW
{
  static CpuInfoStatic i;
  return &i.i;
}

} // AsmJit

// [Warnings-Pop]
#include "WarningsPop.h"

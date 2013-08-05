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

// this file is used to test cpu detection.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AsmJit/CpuInfo.h>

using namespace AsmJit;

struct BitDescription
{
  UInt32 mask;
  const char* description;
};

static const BitDescription cFeatures[] = 
{
  { CpuInfo::Feature_RDTSC                       , "RDTSC" },
  { CpuInfo::Feature_RDTSCP                      , "RDTSCP" },
  { CpuInfo::Feature_CMOV                        , "CMOV" },
  { CpuInfo::Feature_CMPXCHG8B                   , "CMPXCHG8B" },
  { CpuInfo::Feature_CMPXCHG16B                  , "CMPXCHG16B" },
  { CpuInfo::Feature_CLFLUSH                     , "CLFLUSH" },
  { CpuInfo::Feature_PREFETCH                    , "PREFETCH" },
  { CpuInfo::Feature_LAHF_SAHF                   , "LAHF/SAHF" },
  { CpuInfo::Feature_FXSR                        , "FXSAVE/FXRSTOR" },
  { CpuInfo::Feature_FFXSR                       , "FXSAVE/FXRSTOR Optimizations" },
  { CpuInfo::Feature_MMX                         , "MMX" },
  { CpuInfo::Feature_MMXExt                      , "MMX Extensions" },
  { CpuInfo::Feature_3dNow                       , "3dNow!" },
  { CpuInfo::Feature_3dNowExt                    , "3dNow! Extensions" },
  { CpuInfo::Feature_SSE                         , "SSE" },
  { CpuInfo::Feature_MSSE                        , "Misaligned SSE" },
  { CpuInfo::Feature_SSE2                        , "SSE2" },
  { CpuInfo::Feature_SSE3                        , "SSE3" },
  { CpuInfo::Feature_SSSE3                       , "Suplemental SSE3 (SSSE3)" },
  { CpuInfo::Feature_SSE4_A                      , "SSE4A" },
  { CpuInfo::Feature_SSE4_1                      , "SSE4.1" },
  { CpuInfo::Feature_SSE4_2                      , "SSE4.2" },
  { CpuInfo::Feature_SSE5                        , "SSE5" },
  { CpuInfo::Feature_MonitorMWait                , "MONITOR/MWAIT" },
  { CpuInfo::Feature_POPCNT                      , "POPCNT" },
  { CpuInfo::Feature_LZCNT                       , "LZCNT" },
  { CpuInfo::Feature_MultiThreading              , "MultiThreading" },
  { CpuInfo::Feature_ExecuteDisableBit           , "Execute Disable Bit" },
  { CpuInfo::Feature_64Bit                       , "64 Bit Processor" },
  { 0, NULL }
};

static void printBits(const char* msg, UInt32 mask, const BitDescription* d)
{
  for (; d->mask; d++)
  {
    if (mask & d->mask) printf("%s%s\n", msg, d->description);
  }
}

int main(int argc, char* argv[])
{
  CpuInfo *i = cpuInfo();

  printf("CPUID informations\n");
  printf("==================\n");

  printf("\nBasic informations\n");
  printf("  Vendor              : %s\n", i->vendor);
  printf("  Family              : %u\n", i->family);
  printf("  Model               : %u\n", i->model);
  printf("  Stepping            : %u\n", i->stepping);
  printf("  Number of Processors: %u\n", i->numberOfProcessors);
  printf("  Features            : %0.8X\n", i->features);
  printf("  Bugs                : %0.8X\n", i->bugs);
  printf("\nX86 Extended Info:\n");
  printf("  Processor Type      : %u\n", i->x86ExtendedInfo.processorType);
  printf("  Brand Index         : %u\n", i->x86ExtendedInfo.brandIndex);
  printf("  CL Flush Cache Line : %u\n", i->x86ExtendedInfo.clFlushCacheLineSize);
  printf("  Logical Processors  : %u\n", i->x86ExtendedInfo.logicalProcessors);
  printf("  APIC Physical ID    : %u\n", i->x86ExtendedInfo.apicPhysicalId);

  printf("\nCpu Features:\n");
  printBits("  ", i->features, cFeatures);

  return 0;
}

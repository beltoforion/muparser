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
#ifndef _ASMJIT_BUILD_H
#define _ASMJIT_BUILD_H

// [Include]
#include "Config.h"

// Here should be optional include files that's needed fo successfuly
// use macros defined here. Remember, AsmJit uses only AsmJit namespace
// and all macros are used within it. So for example crash handler is
// not called as AsmJit::crash(0) in ASMJIT_CRASH() macro, but simply
// as crash(0).
#include <stdio.h>
#include <stdlib.h>

// [AsmJit - OS]
#if defined(WINDOWS) || defined(__WINDOWS__) || defined(_WIN32) || defined(_WIN64)
# define ASMJIT_WINDOWS
#elif defined(__linux__)     || defined(__unix__)    || \
      defined(__OpenBSD__)   || defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__DragonFly__) || defined(__BSD__)     || defined(__FREEBSD__) || \
      defined(__APPLE__)
# define ASMJIT_POSIX
#else
# warning "AsmJit - Can't match operating system, using ASMJIT_POSIX"
# define ASMJIT_POSIX
#endif

// [AsmJit - Architecture]
// define it only if it's not defined. In some systems we can
// use -D command in compiler to bypass this autodetection.
#if !defined(ASMJIT_X86) && !defined(ASMJIT_X64)
# if defined(__x86_64__) || defined(__LP64) || defined(__IA64__) || \
     defined(_M_X64)     || defined(_WIN64) 
#  define ASMJIT_X64 // x86-64
# else
// _M_IX86, __INTEL__, __i386__
#  define ASMJIT_X86
# endif
#endif

// [AsmJit - API]

// Hide AsmJit symbols that we don't want to export (SerializerIntrinsics class for example).
#if !defined(ASMJIT_HIDDEN)
# if defined(__GNUC__) && __GNUC__ >= 4
#  define ASMJIT_HIDDEN __attribute__((visibility("hidden")))
# endif // __GNUC__
#endif // ASMJIT_HIDDEN

// Make AsmJit as shared library by default.
#if !defined(ASMJIT_API)
# if defined(ASMJIT_WINDOWS)
#  if defined(__GNUC__)
#   if defined(AsmJit_EXPORTS)
#    define ASMJIT_API __attribute__((dllexport))
#   else
#    define ASMJIT_API __attribute__((dllimport))
#   endif
#  else
#   if defined(AsmJit_EXPORTS)
#    define ASMJIT_API __declspec(dllexport)
#   else
#    define ASMJIT_API __declspec(dllimport)
#   endif
#  endif
# else
#  if defined(__GNUC__)
#   if __GNUC__ >= 4
#    define ASMJIT_API __attribute__((visibility("default")))
#   endif
#  endif
# endif
#endif // ASMJIT_API

// If not detected, fallback to nothing
#if !defined(ASMJIT_HIDDEN)
# define ASMJIT_HIDDEN
#endif // ASMJIT_HIDDEN

#if !defined(ASMJIT_API)
# define ASMJIT_API
#endif // ASMJIT_API

#if !defined(ASMJIT_NOTHROW)
#define ASMJIT_NOTHROW throw()
#endif // ASMJIT_NOTHROW

// [AsmJit - Memory Management]
#if !defined(ASMJIT_MALLOC)
# define ASMJIT_MALLOC ::malloc
#endif // ASMJIT_MALLOC

#if !defined(ASMJIT_REALLOC)
# define ASMJIT_REALLOC ::realloc
#endif // ASMJIT_REALLOC

#if !defined(ASMJIT_FREE)
# define ASMJIT_FREE ::free
#endif // ASMJIT_FREE

// [AsmJit - Crash handler]
namespace AsmJit
{
  static void crash(int* ptr = 0) { *ptr = 0; }
}

// [AsmJit - Calling Conventions]
#if defined(ASMJIT_X86)
# if defined(__GNUC__)
#  define ASMJIT_FASTCALL_2 __attribute__((regparm(2)))
#  define ASMJIT_FASTCALL_3 __attribute__((regparm(3)))
#  define ASMJIT_STDCALL    __attribute__((stdcall))
#  define ASMJIT_CDECL      __attribute__((cdecl))
# else
#  define ASMJIT_FASTCALL_2 __fastcall
#  define ASMJIT_STDCALL    __stdcall
#  define ASMJIT_CDECL      __cdecl
# endif
#else
# define ASMJIT_FASTCALL_2
# define ASMJIT_STDCALL
# define ASMJIT_CDECL
#endif // ASMJIT_X86

#if !defined(ASMJIT_USE)
# define ASMJIT_USE(var) ((void)var)
#endif // ASMJIT_USE

#if !defined(ASMJIT_NOP)
# define ASMJIT_NOP() ((void)0)
#endif // ASMJIT_NOP

// [AsmJit - Types]
namespace AsmJit
{
  typedef char Int8;
  typedef unsigned char UInt8;
  typedef short Int16;
  typedef unsigned short UInt16;
  typedef int Int32;
  typedef unsigned int UInt32;

#if defined(_MSC_VER)
  typedef __int64 Int64;
  typedef unsigned __int64 UInt64;
#else // GCC, other compilers ?
  typedef long long Int64;
  typedef unsigned long long UInt64;
#endif

#if defined(ASMJIT_X86)
  typedef Int32 SysInt;
  typedef UInt32 SysUInt;
#else
  typedef Int64 SysInt;
  typedef UInt64 SysUInt;
#endif
}

#if defined(_MSC_VER)
# define ASMJIT_INT64_C(num) num##i64
# define ASMJIT_UINT64_C(num) num##ui64
#else
# define ASMJIT_INT64_C(num) num##LL
# define ASMJIT_UINT64_C(num) num##ULL
#endif

// [AsmJit - C++ Macros]
#define ASMJIT_ARRAY_SIZE(A) (sizeof(A) / sizeof(*A))

#define ASMJIT_DISABLE_COPY(__type__) \
private: \
  inline __type__(const __type__& other); \
  inline __type__& operator=(const __type__& other)

// [AsmJit - Debug]
#if defined(DEBUG) || defined(_DEBUG)
# if !defined(ASMJIT_CRASH)
#  define ASMJIT_CRASH() crash()
# endif
# if !defined(ASMJIT_ASSERT)
#  define ASMJIT_ASSERT(exp) do { if (!(exp)) ASMJIT_CRASH(); } while(0)
# endif
#else
# if !defined(ASMJIT_CRASH)
#  define ASMJIT_CRASH() ASMJIT_NOP()
# endif
# if !defined(ASMJIT_ASSERT)
#  define ASMJIT_ASSERT(exp) ASMJIT_NOP()
# endif
#endif // DEBUG

// GCC warnings fix: I can't understand why GCC hasn't interface to push/pop
// warning
// #if defined(__GNUC__)
// # if (__GNUC__ * 10000  + __GNUC_MINOR__ * 100  + __GNUC_PATCHLEVEL__) >= 402001
// #  pragma GCC diagnostic ignored "-w"
// # endif
// #endif // __GNUC__

// [Guard]
#endif // _ASMJIT_BUILD_H

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

// This file is designed to be changeable. Platform specific changes
// should be applied to this file and this guarantes and never versions
// of AsmJit library will never overwrite generated config files.
//
// So modify this will by your build system or hand.

// [Guard]
#ifndef _ASMJIT_CONFIG_H
#define _ASMJIT_CONFIG_H

// [AsmJit - OS]
// 
// Provides definitions about your operating system. It's detected by default,
// so override it if you have problems with automatic detection.
//
// #define ASMJIT_WINDOWS 1
// #define ASMJIT_POSIX 2

// [AsmJit - Architecture]
//
// Provides definitions about your cpu architecture. It's detected by default,
// so override it if you have problems with automatic detection.

// #define ASMJIT_X86
// #define ASMJIT_X64

// [AsmJit - API]
//
// If you are embedding AsmJit library into your project (statically), undef
// ASMJIT_API macro. ASMJIT_HIDDEN macro can contain visibility (used by GCC)
// to hide some AsmJit symbols that shouldn't be never exported.
//
// If you have problems with throw() in compilation time, undef ASMJIT_NOTHROW
// to disable this feature. ASMJIT_NOTHROW marks functions that never throws
// an exception.

// #define ASMJIT_HIDDEN
#define ASMJIT_API
// #define ASMJIT_NOTHROW


// [AsmJit - Memory Management]
// #define ASMJIT_MALLOC ::malloc
// #define ASMJIT_REALLOC ::realloc
// #define ASMJIT_FREE ::free

// [AsmJit - Debug]
// #define ASMJIT_CRASH() crash()
// #define ASMJIT_ASSERT(exp) do { if (!(exp)) ASMJIT_CRASH(); } while(0)

// [Guard]
#endif // _ASMJIT_CONFIG_H

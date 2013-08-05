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
#ifndef _ASMJIT_H
#define _ASMJIT_H

//! @mainpage
//!
//! @brief AsmJit is complete x86/x64 JIT Assembler for C++ language
//! 
//! It supports FPU, MMX, 3dNow, SSE, SSE2, SSE3 and SSE4 intrinsics, powerful
//! compiler that helps to write portable functions for 32-bit (x86) and 64-bit
//! (x64) architectures. AsmJit can be used to create functions at runtime that
//! can be called from existing (but also generated) C/C++ code.
//!
//! AsmJit is crossplatform library that supports various compilers and
//! operating systems. Currently only limitation is x86 (32-bit) or x64 (64-bit)
//! processor. Currently tested operating systems are Windows (32 bit and 64 bit),
//! Linux (32 bit and 64 bit) and MacOSX (32 bit). 
//!
//! <b>Introduction</b>
//!
//! AsmJit library contains two main classes for code generation with different
//! goals. First main code generation class is called @c AsmJit::Assembler and 
//! contains low level API that can be used to generate JIT binary code. It 
//! directly emits binary stream that represents encoded x86/x64 assembler 
//! opcodes. Together with operands and labels it can be used to generate 
//! complete code. For details look to @ref AsmJit_Serializer and 
//! @ref AsmJit_Assembler sections.
//!
//! There is also class named @c AsmJit::Compiler that allows to develop 
//! crossplatform assembler code without worring about function calling 
//! conventions and registers allocation. It can be also used to write 32 
//! bit and 64 bit portable code. Compiler is recommended class to use for 
//! code generation. If you want to use pure @c AsmJit::Compiler solution,
//! it's needed also to look at @c AsmJit::Assembler and @c AsmJit::Serializer
//! classes to understand how AsmJit library works and how you should use
//! its API.
//!
//! Everything in AsmJit library is in @c AsmJit namespace.
//!
//! <b>Code generation sections:</b>
//!
//! - @ref AsmJit_Serializer "Serializer" - Intrinsics, operands and labels.
//! - @ref AsmJit_Assembler "Assembler" - Low level code generation.
//! - @ref AsmJit_Compiler "Compiler" - High level code generation.
//! - @ref AsmJit_CpuInfo "Cpu Informations" - Get informations about host processor.
//! - @ref AsmJit_Logging "Logging" - Logging and error handling.
//! - @ref AsmJit_MemoryManagement "Memory Management" - Virtual memory management.
//!
//! <b>Configuration, definitions and utilities:</b>
//!
//! - @ref AsmJit_Config "Configuration" - Macros used to configure AsmJit.
//! - @ref AsmJit_Defs "Definitions" - Constants and macros.
//! - @ref AsmJit_Util "Utilities" - Utilities and helper classes.
//!
//! <b>AsmJit homepage:</b>
//! - http://code.google.com/p/asmjit/
//!
//! <b>External resources:</b>
//! - http://www.agner.org/optimize/
//! - http://www.mark.masmcode.com/ (Assembler Tips)
//! - http://avisynth.org/mediawiki/Filter_SDK/Assembler_optimizing (Optimizing)
//! - http://www.ragestorm.net/distorm/ (Disassembling)


//! @defgroup AsmJit_Assembler Assembler - low level code generation.
//!
//! Contains classes related to @c AsmJit::Assembler that is directly used 
//! to generate machine code stream. It's one of oldest and fastest method 
//! to generate machine code through AsmJit library.
//!
//! - See @c AsmJit::Serializer class for intrinsics and operands
//!   documentation.
//! - See @c AsmJit::Assembler class for low level code generation 
//!   documentation.


//! @defgroup AsmJit_Compiler Compiler - high level code generation.
//!
//! Contains classes related to @c AsmJit::Compiler that is high level code
//! generation class. 
//!
//! - See @c AsmJit::Serializer class for intrinsics and operands
//!   documentation.
//! - See @c AsmJit::Assembler class for low level code generation
//!   and documentation for assembler operands.
//! - See @c AsmJit::Compiler class for high level code generation 
//!   documentation - calling conventions, function declaration
//!   and variables management.

//! @defgroup AsmJit_Config Configuration - macros used to configure AsmJit.
//! library.
//!
//! Contains macros that can be redefined to fit into any project.


//! @defgroup AsmJit_CpuInfo Cpu informations - Get informations about host 
//! processor.
//!
//! X86 or x64 cpuid instruction allows to get information about processor 
//! vendor and it's features. It's always used to detect features like MMX, 
//! SSE and other newer ones.
//!
//! AsmJit library supports low level cpuid call implemented internally as 
//! C++ function using inline assembler or intrinsics and also higher level 
//! CPU features detection. The low level function (also used by higher level 
//! one) is @c AsmJit::cpuid().
//!
//! AsmJit library also contains higher level function @c AsmJit::cpuInfo()
//! that returns features detected by the library. The detection process is
//! done only once and it's cached for all next calls. @c AsmJit::CpuInfo 
//! structure not contains only informations through @c AsmJit::cpuid(), but
//! there is also small multiplatform code to detect number of processors 
//! (or cores) throught operating system API.
//!
//! It's recommended to use @c AsmJit::cpuInfo to detect and check for
//! host processor features.
//!
//! Example how to use AsmJit::cpuid():
//!
//! @code
//! // All functions and structures are in AsmJit namesapce.
//! using namespace AsmJit;
//!
//! // Here will be retrieved result of cpuid call.
//! CpuId out;
//!
//! // Use cpuid function to do the job.
//! cpuid(0 /* eax */, &out /* eax, ebx, ecx, edx */);
//!
//! // Id eax argument to cpuid is 0, ebx, ecx and edx registers 
//! // are filled with cpu vendor.
//! char vendor[13];
//! memcpy(i->vendor, &out.ebx, 4);
//! memcpy(i->vendor + 4, &out.edx, 4);
//! memcpy(i->vendor + 8, &out.ecx, 4);
//! vendor[12] = '\0';
//! 
//! // Print vendor
//! puts(vendor);
//! @endcode
//!
//! If you want to use AsmJit::cpuid() function instead of higher level 
//! @c AsmJit::cpuInfo(), please read processor manuals provided by Intel, 
//! AMD or other manufacturers for cpuid instruction details.
//!
//! Example of using @c AsmJit::cpuInfo():
//!
//! @code
//! // All functions and structures are in AsmJit namesapce.
//! using namespace AsmJit;
//!
//! // Call to cpuInfo return CpuInfo structure that shouldn't be modified.
//! // Make it const by default.
//! const CpuInfo *i = cpuInfo();
//!
//! // Now you are able to get specific features
//!
//! // Processor has SSE2
//! if (i->features & CpuInfo::Feature_SSE2)
//! {
//!   // your code...
//! }
//! // Processor has MMX
//! else if (i->features & CpuInfo::Feature_MMX)
//! {
//!   // your code...
//! }
//! // Processor is old, no SSE2 or MMX support
//! else
//! {
//!   // your code...
//! }
//! @endcode
//!
//! Better example is in AsmJit/test/testcpu.cpp file.
//!
//! @sa AsmJit::cpuid, @c AsmJit::cpuInfo.


//! @defgroup AsmJit_Defs Definitions - registers and instructions constants.
//!
//! Contains constants used in AsmJit library.


//! @defgroup AsmJit_Logging Logging - logging and error handling.
//!
//! Contains classes related to loging assembler output. Currently logging
//! is implemented in @c AsmJit::Logger class.You can override
//! @c AsmJit::Logger::log() to log messages into your stream. There is also
//! @c FILE based logger implemented in @c AsmJit::FileLogger class.
//!
//! To log your assembler output to FILE stream use this code:
//!
//! @code
//! // Create assembler
//! Assembler a;
//!
//! // Create and set file based logger
//! FileLogger logger(stderr);
//! a.setLogger(&logger);
//! @endcode
//!
//! You can see that logging goes through @c AsmJit::Assembler. If you are
//! using @c AsmJit::Compiler and you want to log messages in correct assembler
//! order, you should look at @c AsmJit::Compiler::comment() method. It allows 
//! you to insert text message into @c AsmJit::Emittable list and 
//! @c AsmJit::Compiler will send your message to @c AsmJit::Assembler in 
//! correct order.
//!
//! @sa @c AsmJit::Logger, @c AsmJit::FileLogger.


//! @defgroup AsmJit_Serializer Serializer - code generation intrinsics.
//!
//! Serializer implements assembler intrinsics that's used by @c Assembler
//! and @c Compiler classes. Intrinsics are implemented as overloaded methods
//! of @c AsmJit::Serializer class. This section also contains all assembler
//! primitives used to generate machine code.
//!
//! <b>Registers</b>
//!
//! There are static objects that represents X86 and X64 registers. They can 
//! be used directly (like @c eax, @c mm, @c xmm, ...) or created through 
//! these functions:
//!
//! - @c AsmJit::mk_gpb() - make general purpose byte register
//! - @c AsmJit::mk_gpw() - make general purpose word register
//! - @c AsmJit::mk_gpd() - make general purpose dword register
//! - @c AsmJit::mk_gpq() - make general purpose qword register
//! - @c AsmJit::mk_mm() - make mmx register
//! - @c AsmJit::mk_xmm() - make sse register
//! - @c AsmJit::st() - make x87 register
//!
//! <b>Addressing</b>
//!
//! X86 and x64 architectures contains several addressing modes and most ones
//! are possible with AsmJit library. Memory represents are represented by
//! @c AsmJit::Mem class. These functions are used to make operands that 
//! represents memory addresses:
//!
//! - @c AsmJit::ptr()
//! - @c AsmJit::byte_ptr()
//! - @c AsmJit::word_ptr()
//! - @c AsmJit::dword_ptr()
//! - @c AsmJit::qword_ptr()
//! - @c AsmJit::tword_ptr()
//! - @c AsmJit::dqword_ptr()
//! - @c AsmJit::mmword_ptr()
//! - @c AsmJit::xmmword_ptr()
//! - @c AsmJit::sysint_ptr()
//!
//! Most useful function to make pointer should be @c AsmJit::ptr(). It creates
//! pointer to the target with unspecified size. Unspecified size works in all
//! intrinsics where are used registers (this means that size is specified by
//! register operand or by instruction itself). For example @c AsmJit::ptr() 
//! can't be used with @c AsmJit::Serializer::inc() instruction. In this case 
//! size must be specified and it's also reason to make difference between 
//! pointer sizes.
//!
//! Supported are simple address forms (register + displacement) and complex
//! address forms (register + (register << shift) + displacement).
//!
//! <b>Immediates</b>
//!
//! Immediate values are constants thats passed directly after instruction 
//! opcode. To create such value use @c AsmJit::imm() or @c AsmJit::uimm()
//! methods to create signed or unsigned immediate value.
//!
//! @sa @c AsmJit::Serializer.


//! @defgroup AsmJit_Util Utilities - Utilities and helper classes.
//!
//! Contains some helper classes that's used by AsmJit library.


//! @defgroup AsmJit_MemoryManagement Virtual memory management.
//!
//! Using @c AsmJit::Assembler or @c AsmJit::Compiler to generate machine 
//! code is not final step. Each generated code needs to run in memory 
//! that is not protected against code execution. To alloc this code it's
//! needed to use operating system functions provided to enable execution
//! code in specified memory block or to allocate memory that is not
//! protected. The solution is always to use @c See AsmJit::Assembler::make() 
//! and @c AsmJit::Compiler::make() functions that can allocate memory and
//! relocate code for you. But AsmJit also contains classes for manual memory
//! management thats internally used by AsmJit but can be used by programmers
//! too.
//!
//! Memory management contains low level and high level classes related to
//! allocating and freeing virtual memory. Low level class is 
//! @c AsmJit::VirtualMemory that can allocate and free full pages of
//! virtual memory provided by operating system. Higher level class is
//! @c AsmJit::MemoryManager that is able to manage complete allocation and
//! free mechanism. It internally uses larger chunks of memory to make
//! allocation fast and effective.
//!
//! Using @c AsmJit::VirtualMemory::alloc() is crossplatform way how to 
//! allocate this kind of memory without worrying about operating system 
//! and it's API. Each memory block that is no longer needed should be 
//! freed by @c AsmJit::VirtualMemory::free() method. If you want better
//! comfort and malloc()/free() interface, look at the 
//! @c AsmJit::MemoryManager class.
//!
//! @sa @c AsmJit::VirtualMemory, @ AsmJit::MemoryManager.


//! @addtogroup AsmJit_Config
//! @{

//! @def ASMJIT_WINDOWS
//! @brief Macro that is declared if AsmJit is compiled for Windows.

//! @def ASMJIT_POSIX
//! @brief Macro that is declared if AsmJit is compiled for unix like 
//! operating system.

//! @def ASMJIT_API
//! @brief Attribute that's added to classes that can be exported if AsmJit
//! is compiled as a dll library.

//! @def ASMJIT_MALLOC
//! @brief Function to call to allocate dynamic memory.

//! @def ASMJIT_REALLOC
//! @brief Function to call to reallocate dynamic memory.

//! @def ASMJIT_FREE
//! @brief Function to call to free dynamic memory.

//! @def ASMJIT_CRASH
//! @brief Code that is execute if an one or more operands are invalid.

//! @def ASMJIT_ASSERT
//! @brief Assertion macro. Default implementation calls @c ASMJIT_CRASH
//! if assert fails.

//! @}


//! @namespace AsmJit
//! @brief Main AsmJit library namespace.
//!
//! There are not other namespaces used in AsmJit library.


// [Includes]
#include "Build.h"
#include "Assembler.h"
#include "Compiler.h"
#include "CpuInfo.h"
#include "Defs.h"
#include "Logger.h"
#include "MemoryManager.h"
#include "Serializer.h"
#include "Util.h"
#include "VirtualMemory.h"


// [Guard]
#endif // _ASMJIT_H

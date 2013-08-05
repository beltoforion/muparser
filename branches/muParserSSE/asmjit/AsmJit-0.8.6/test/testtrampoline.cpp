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

// This file is used to test setcc instruction generation.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AsmJit/Compiler.h>
#include <AsmJit/Logger.h>
#include <AsmJit/MemoryManager.h>

#if defined(ASMJIT_X86)

int main(int argc, char* argv[])
{
  printf("Trampoline test can be only used in x64 mode.\n");
  return 0;
}

#else

// This is type of function we will generate
typedef void (*MyFn)(void);

static int i = 0;

// Function that is called from JIT code.
static void calledfn(void)
{
  i++;
}

int main(int argc, char* argv[])
{
  using namespace AsmJit;

  // ==========================================================================
  // Create assembler.
  Assembler a;

  // Log compiler output.
  FileLogger logger(stderr);
  a.setLogger(&logger);

  a.call(imm((SysInt)calledfn)); // First trampoline - call.
  a.jmp(imm((SysInt)calledfn));  // Second trampoline - jump, will return.
  MyFn fn0 = function_cast<MyFn>(a.make());

  a.clear(); // Purge assembler, we will reuse it.
  a.jmp(imm((SysInt)fn0));
  MyFn fn1 = function_cast<MyFn>(a.make());

  // ==========================================================================

  // ==========================================================================
  fn0();
  fn1();

  printf("Status: %s\n", (i == 4) ? "Success" : "Failure");

  // If functions are not needed again they should be freed.
  MemoryManager::global()->free((void*)fn0);
  MemoryManager::global()->free((void*)fn1);
  // ==========================================================================

  return 0;
}

#endif

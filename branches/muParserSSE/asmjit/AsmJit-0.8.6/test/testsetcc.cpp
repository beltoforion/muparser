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

// This is type of function we will generate
typedef void (*MyFn)(int, int, char*);

int main(int argc, char* argv[])
{
  using namespace AsmJit;

  // ==========================================================================
  // Create compiler.
  Compiler c;

  // Log compiler output.
  FileLogger logger(stderr);
  c.setLogger(&logger);

  Function& f = *c.newFunction(CALL_CONV_DEFAULT, BuildFunction3<int, int, char*>());

  // Possibilities to improve code:
  //   f.setNaked(true);
  //   f.setAllocableEbp(true);

  PtrRef src0(f.argument(0));
  PtrRef src1(f.argument(1));
  PtrRef dst0(f.argument(2));

  c.cmp(src0.c(), src1.c());
  c.setz(byte_ptr(dst0.c()));

  // Finish
  c.endFunction();
  // ==========================================================================

  // ==========================================================================
  // Make function
  MyFn fn = function_cast<MyFn>(c.make());

  // Results storage.
  char r[4];

  // Call it
  fn(0, 0, &r[0]); // We are expecting 1 (0 == 0).
  fn(0, 1, &r[1]); // We are expecting 0 (0 != 1).
  fn(1, 0, &r[2]); // We are expecting 0 (1 != 0).
  fn(1, 1, &r[3]); // We are expecting 1 (1 == 1).

  printf("Result from JIT function: %d %d %d %d\n", r[0], r[1], r[2], r[3]);
  printf("Status: %s\n", (r[0] == 1 && r[1] == 0 && r[2] == 0 && r[3] == 1) ? "Success" : "Failure");

  // If function is not needed again it should be freed.
  MemoryManager::global()->free((void*)fn);
  // ==========================================================================

  return 0;
}

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

// This file is used to test overwriting of existing code.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AsmJit/Assembler.h>
#include <AsmJit/MemoryManager.h>

// This is type of function we will generate
typedef int (*MyFn)();

int main(int argc, char* argv[])
{
  using namespace AsmJit;

  // ==========================================================================
  // Create assembler.
  Assembler a;

  // Prolog.
  a.push(nbp);
  a.mov(nbp, nsp);

  // Mark this offset.
  SysInt mark = a.offset();

  // Mov 1024 to EAX/RAX, EAX/RAX is also return value.
  // (This instruction will be patched.
  a.mov(nax, 1024);

  a.mov(nax, ncx);

  // Epilog.
  a.mov(nsp, nbp);
  a.pop(nbp);
  a.ret();
  // ==========================================================================

  // ==========================================================================
  // Patch code at 'mark' position. End variable is needed to remember  current
  // offset (offset where we want to go back), because we must go back  to set
  // correct code size (offset is also used as code size).
  SysInt end = a.toOffset(mark);

  // Patch
  a.mov(ncx, 1024);

  // Go back, this step is very IMPORTANT!
  a.toOffset(end);
  // ==========================================================================

  // ==========================================================================
  // Make function
  MyFn fn = function_cast<MyFn>(a.make());

  // Call it.
  int result = fn();
  printf("Result from jit function: %d\n", result);
  printf("Status: %s\n", result == 1024 ? "Success" : "Failure");

  // If function is not needed again it should be freed.
  MemoryManager::global()->free((void*)fn);
  // ==========================================================================

  return 0;
}

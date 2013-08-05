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

// This file is only included as an example and simple test if jit
// compiler works.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AsmJit/Assembler.h>
#include <AsmJit/Logger.h>
#include <AsmJit/MemoryManager.h>

// This is type of function we will generate
typedef int (*MyFn)();

int main(int argc, char* argv[])
{
  using namespace AsmJit;

  // ==========================================================================
  // Create assembler.
  Assembler a;

  // Log assembler output.
  FileLogger logger(stderr);
  a.setLogger(&logger);

  // Prolog.
  a.push(nbp);
  a.mov(nbp, nsp);

  // Mov 1024 to EAX/RAX, EAX/RAX is also return value.
  a.mov(nax, 1024);

  // Epilog.
  a.mov(nsp, nbp);
  a.pop(nbp);
  a.ret();
  // ==========================================================================

  // NOTE:
  // This function can be also completely rewritten to this form:
  //   a.mov(nax, 1024);
  //   a.ret();
  // If you are interested in removing prolog and epilog, please
  // study calling conventions and check register preservations.

  // ==========================================================================
  // Make function.
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

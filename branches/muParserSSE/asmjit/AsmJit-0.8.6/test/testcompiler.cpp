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

// This file is used to test AsmJit compiler.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AsmJit/Compiler.h>
#include <AsmJit/Logger.h>
#include <AsmJit/MemoryManager.h>

// This is type of function we will generate
typedef void (*MyFn)(int*, int*);

int main(int argc, char* argv[])
{
  using namespace AsmJit;

  // ==========================================================================
  // Create compiler.
  Compiler c;

  // Log assembler output.
  FileLogger logger(stderr);
  c.setLogger(&logger);

  Function& f = *c.newFunction(CALL_CONV_DEFAULT, BuildFunction2<int*, int*>());

  // Possibilities to improve code:
  //   f.setNaked(true);
  //   f.setAllocableEbp(true);

  PtrRef a1(f.argument(0));
  PtrRef a2(f.argument(1));

  // Create some variables, default variable priority is 10.
  Int32Ref x1(f.newVariable(VARIABLE_TYPE_INT32));
  Int32Ref x2(f.newVariable(VARIABLE_TYPE_INT32));
  Int32Ref x3(f.newVariable(VARIABLE_TYPE_INT32));
  Int32Ref x4(f.newVariable(VARIABLE_TYPE_INT32));
  Int32Ref x5(f.newVariable(VARIABLE_TYPE_INT32));
  Int32Ref x6(f.newVariable(VARIABLE_TYPE_INT32));
  Int32Ref x7(f.newVariable(VARIABLE_TYPE_INT32));
  Int32Ref x8(f.newVariable(VARIABLE_TYPE_INT32));

  // Set our variables (use mov with reg/imm to se if register 
  // allocator works).
  // x() means that there will be not read operation, only write.
  c.mov(x1.x(), 1);
  c.mov(x2.x(), 2);
  c.mov(x3.x(), 3);
  c.mov(x4.x(), 4);
  c.mov(x5.x(), 5);
  c.mov(x6.x(), 6);
  c.mov(x7.x(), 7);
  c.mov(x8.x(), 8);

  // Create temporary variable, type int32 and priority 5
  // (lower probability to spill).
  Int32Ref t(f.newVariable(VARIABLE_TYPE_INT32, 5));

  // Make sum (addition)
  // r() means that we can read or write to register, c() means that register
  // is used only for read (compiler can use informations collected by these
  // functions to optimize register allocation and spilling).
  //
  // If you are not sure about .c(), .x() and .r(), use always .r().
  c.xor_(t.r(), t.r());
  c.add(t.r(), x1.c());
  c.add(t.r(), x2.c());
  c.add(t.r(), x3.c());
  c.add(t.r(), x4.c());
  c.add(t.r(), x5.c());
  c.add(t.r(), x6.c());
  c.add(t.r(), x7.c());
  c.add(t.r(), x8.c());

  // Store result to a given pointer in first argument.
  c.mov(dword_ptr(a1.c()), t.c());

  // Make sum (subtraction).
  c.xor_(t.r(), t.r());
  c.sub(t.r(), x1.c());
  c.sub(t.r(), x2.c());
  c.sub(t.r(), x3.c());
  c.sub(t.r(), x4.c());
  c.sub(t.r(), x5.c());
  c.sub(t.r(), x6.c());
  c.sub(t.r(), x7.c());
  c.sub(t.r(), x8.c());

  // Store result to a given pointer in second argument.
  c.mov(dword_ptr(a2.c()), t.c());

  // End of function.
  c.endFunction();
  // ==========================================================================

  // ==========================================================================
  // Make function.
  MyFn fn = function_cast<MyFn>(c.make());

  // Call it.
  int x;
  int y;
  fn(&x, &y);
  printf("\nResults from JIT function: %d %d\n", x, y);
  printf("Status: %s\n", (x == 36 && y == -36) ? "Success" : "Failure");

  // If function is not needed again it should be freed.
  MemoryManager::global()->free((void*)fn);
  // ==========================================================================

  return 0;
}

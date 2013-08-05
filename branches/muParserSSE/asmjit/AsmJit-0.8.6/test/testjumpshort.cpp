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

// This file is used to test possibility to specify short jump instruction. This
// example is designed to fail - emitting too long code to make short jump and
// print error message.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AsmJit/Assembler.h>
#include <AsmJit/Compiler.h>
#include <AsmJit/Logger.h>
#include <AsmJit/MemoryManager.h>

// This is type of function we will generate
typedef void (*MyFn)(void);

using namespace AsmJit;

static MyFn generate(bool useShortJump)
{
  // Create compiler.
  Compiler c;

  // Log assembler output.
  FileLogger logger(stderr);
  c.setLogger(&logger);

  // Create function.
  c.newFunction(CALL_CONV_DEFAULT, BuildFunction0());

  // Create variable and jump.
  Int32Ref var(c.newVariable(VARIABLE_TYPE_INT32));

  Label* end = c.newLabel();
  c.xor_(var.x(), var.x());
  c.cmp(var.c(), imm(0));

  if (useShortJump)
    c.jz_short(end);
  else
    c.jz(end);

  // Add nops to ensure that jump can't be short.
  for (int i = 0; i < 256; i++) c.nop();

  c.bind(end);

  // End of function.
  c.endFunction();

  // Make.
  return function_cast<MyFn>(c.make());
}

int main(int argc, char* argv[])
{
  MyFn fn1;
  MyFn fn2;

  fn1 = generate(false);
  fn2 = generate(true);

  static const char* yesno[] = { "no", "yes" };

  printf("\n");
  printf("First function generated: %s (should be 'yes')\n", yesno[fn1 != NULL]);
  printf("Second function generated: %s (should be 'no')\n", yesno[fn2 != NULL]);
  printf("\n");
  printf("Status: %s\n", (fn1 != NULL && fn2 == NULL) ? "Success" : "Failure");

  // If functions are not needed, they should be freed.
  MemoryManager::global()->free((void*)fn1);
  MemoryManager::global()->free((void*)fn2); // fn2 should be null.
  // ==========================================================================

  return 0;
}

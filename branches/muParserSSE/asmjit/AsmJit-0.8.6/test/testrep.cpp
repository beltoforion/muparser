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

// This file is used as rep-test.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AsmJit/Compiler.h>
#include <AsmJit/Logger.h>
#include <AsmJit/MemoryManager.h>

// This is type of function we will generate
typedef void (*MemCopy)(void* a, void* b, AsmJit::SysUInt size);

int main(int argc, char* argv[])
{
  using namespace AsmJit;

  // ==========================================================================
  // Create compiler.
  Compiler c;

  // Log compiler output.
  FileLogger logger(stderr);
  c.setLogger(&logger);

  c.newFunction(CALL_CONV_DEFAULT, BuildFunction3<void*, void*, SysUInt>());
  {
    PtrRef dst(c.argument(0));
    PtrRef src(c.argument(1));
    SysIntRef cnt(c.argument(2));
    
    dst.alloc(VARIABLE_ALLOC_READWRITE, REG_EDI);
    src.alloc(VARIABLE_ALLOC_READWRITE, REG_ESI);
    cnt.alloc(VARIABLE_ALLOC_READWRITE, REG_ECX);

    c.rep_movsb();
  }
  c.endFunction();
  // ==========================================================================

  // ==========================================================================
  {
    MemCopy copy = function_cast<MemCopy>(c.make());

    char src[20] = "Hello AsmJit";
    char dst[20];
    
    copy(dst, src, strlen(src) + 1);
    printf("src=%s\n", src);
    printf("dst=%s\n", dst);

    MemoryManager::global()->free((void*)copy);
  }
  // ==========================================================================

  return 0;
}

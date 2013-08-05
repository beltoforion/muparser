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

// [Dependencies]
#include "Defs.h"

// [Warnings-Push]
#include "WarningsPush.h"

namespace AsmJit {

const char* errorCodeToString(UInt32 error) ASMJIT_NOTHROW
{
  static const char* errorMessage[] = {
    "No error",
    "No heap memory",
    "No virtual memory",
    "Unknown instruction",
    "Illegal instruction",
    "Illegal addressing",
    "Illegal short jump",
    "Unknown error"
  };

  // Saturate error code to be able to use errorMessage[].
  if (error > _ERROR_COUNT) error = _ERROR_COUNT;

  return errorMessage[error];
}

} // AsmJit

// [Warnings-Pop]
#include "WarningsPop.h"

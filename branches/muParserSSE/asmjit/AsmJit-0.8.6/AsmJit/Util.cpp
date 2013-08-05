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
#include "Build.h"
#include "Util.h"

// [Warnings-Push]
#include "WarningsPush.h"

namespace AsmJit {

// ============================================================================
// [AsmJit::Buffer]
// ============================================================================

void Buffer::emitData(const void* dataPtr, SysUInt dataLen) ASMJIT_NOTHROW
{
  SysInt max = capacity() - offset();
  if ((SysUInt)max < dataLen)
  {
    if (!realloc(offset() + dataLen)) return;
  }

  memcpy(_cur, dataPtr, dataLen);
  _cur += dataLen;
}

bool Buffer::realloc(SysInt to) ASMJIT_NOTHROW
{
  if (capacity() < to)
  {
    SysInt len = offset();

    UInt8 *newdata;
    if (_data)
      newdata = (UInt8*)ASMJIT_REALLOC(_data, to);
    else
      newdata = (UInt8*)ASMJIT_MALLOC(to);
    if (!newdata) return false;

    _data = newdata;
    _cur = newdata + len;
    _max = newdata + to;
    _max -= (to >= _growThreshold) ? _growThreshold : to;

    _capacity = to;
  }

  return true;
}

bool Buffer::grow() ASMJIT_NOTHROW
{
  SysInt to = _capacity;

  if (to < 512)
    to = 1024;
  else if (to > 65536)
    to += 65536;
  else
    to <<= 1;

  return realloc(to);
}

void Buffer::clear() ASMJIT_NOTHROW
{
  _cur = _data;
}

void Buffer::free() ASMJIT_NOTHROW
{
  if (!_data) return;
  ASMJIT_FREE(_data);

  _data = NULL;
  _cur = NULL;
  _max = NULL;
  _capacity = 0;
}

UInt8* Buffer::take() ASMJIT_NOTHROW
{
  UInt8* data = _data;

  _data = NULL;
  _cur = NULL;
  _max = NULL;
  _capacity = 0;

  return data;
}

// ============================================================================
// [AsmJit::Zone]
// ============================================================================

Zone::Zone(SysUInt chunkSize) ASMJIT_NOTHROW
{
  _chunks = NULL;
  _total = 0;
  _chunkSize = chunkSize;
}

Zone::~Zone() ASMJIT_NOTHROW
{
  freeAll();
}

void* Zone::alloc(SysUInt size) ASMJIT_NOTHROW
{
  // Align to 4 or 8 bytes
  size = (size + sizeof(SysInt)-1) & ~(sizeof(SysInt)-1);

  Chunk* cur = _chunks;

  if (!cur || cur->remain() < size)
  {
    SysUInt chSize = _chunkSize;
    if (chSize < size) chSize = size;

    cur = (Chunk*)ASMJIT_MALLOC(sizeof(Chunk) - sizeof(void*) + chSize);
    if (!cur) return NULL;

    cur->prev = _chunks;
    cur->pos = 0;
    cur->size = _chunkSize;
    _chunks = cur;
  }

  UInt8* p = cur->data + cur->pos;
  cur->pos += size;
  _total += size;
  return (void*)p;
}

void Zone::clear() ASMJIT_NOTHROW
{
  Chunk* cur = _chunks;
  if (!cur) return;

  while (cur->prev)
  {
    Chunk* prev = cur->prev;
    ASMJIT_FREE(cur);
    cur = prev;
  }

  _chunks = cur;
  _chunks->pos = 0;
  _total = 0;
}

void Zone::freeAll() ASMJIT_NOTHROW
{
  Chunk* cur = _chunks;
  if (!cur) return;

  do {
    Chunk* prev = cur->prev;
    ASMJIT_FREE(cur);
    cur = prev;
  } while (cur);

  _chunks = NULL;
  _total = 0;
}

} // AsmJit namespace

// [Warnings-Pop]
#include "WarningsPop.h"

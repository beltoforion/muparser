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
#ifndef _ASMJIT_UTIL_H
#define _ASMJIT_UTIL_H

// [Dependencies]
#include "Build.h"

#include <stdlib.h>
#include <string.h>

namespace AsmJit {

//! @addtogroup AsmJit_Util
//! @{

// ============================================================================
// [AsmJit::Macros]
// ============================================================================

// Skip documenting this.
#if !defined(ASMJIT_NODOC)
struct ASMJIT_HIDDEN _DontInitialize {};
struct ASMJIT_HIDDEN _Initialize {};
#endif // !ASMJIT_NODOC

// ============================================================================
// [AsmJit::function_cast<>]
// ============================================================================

//! @brief Cast used to cast pointer to function. It's like reinterpret_cast<>, 
//! but uses internally C style cast to work with MinGW.
//!
//! If you are using single compiler and @c reinterpret_cast<> works for you,
//! there is no reason to use @c AsmJit::function_cast<>. If you are writing
//! crossplatform software with various compiler support, consider using
//! @c AsmJit::function_cast<> instead of @c @c reinterpret_cast<>.
template<typename T, typename Z>
static inline T function_cast(Z* p) ASMJIT_NOTHROW { return (T)p; }

// ============================================================================
// [AsmJit::isIntX]
// ============================================================================

//! @brief Returns @c true if a given integer @a x is signed 8 bit integer
static inline bool isInt8(SysInt x) ASMJIT_NOTHROW { return x >= -128 && x <= 127; }
//! @brief Returns @c true if a given integer @a x is unsigned 8 bit integer
static inline bool isUInt8(SysInt x) ASMJIT_NOTHROW { return x >= 0 && x <= 255; }

//! @brief Returns @c true if a given integer @a x is signed 16 bit integer
static inline bool isInt16(SysInt x) ASMJIT_NOTHROW { return x >= -32768 && x <= 32767; }
//! @brief Returns @c true if a given integer @a x is unsigned 16 bit integer
static inline bool isUInt16(SysInt x) ASMJIT_NOTHROW { return x >= 0 && x <= 65535; }

//! @brief Returns @c true if a given integer @a x is signed 16 bit integer
static inline bool isInt32(SysInt x) ASMJIT_NOTHROW
{
#if defined(ASMJIT_X86)
  return true;
#else
  return x >= ASMJIT_INT64_C(-2147483648) && x <= ASMJIT_INT64_C(2147483647);
#endif
}
//! @brief Returns @c true if a given integer @a x is unsigned 16 bit integer
static inline bool isUInt32(SysInt x) ASMJIT_NOTHROW
{
#if defined(ASMJIT_X86)
  return x >= 0;
#else
  return x >= 0 && x <= ASMJIT_INT64_C(4294967295);
#endif
}

// ============================================================================
// [AsmJit::floatAsInt32, int32AsFloat]
// ============================================================================

//! @brief used to cast float to 32 bit integer and vica versa.
//!
//! @internal
union I32FPUnion
{
  //! @brief 32 bit signed integer value.
  Int32 i;
  //! @brief 32 bit SP-FP value.
  float f;
};

//! @brief used to cast double to 64 bit integer and vica versa.
//!
//! @internal
union I64FPUnion
{
  //! @brief 64 bit signed integer value.
  Int64 i;
  //! @brief 64 bit DP-FP value.
  double f;
};

//! @brief Binary cast 32 bit integer to SP-FP value (@c float).
static inline float int32AsFloat(Int32 i) ASMJIT_NOTHROW
{
  I32FPUnion u;
  u.i = i;
  return u.f;
}

//! @brief Binary cast SP-FP value (@c float) to 32 bit integer.
static inline Int32 floatAsInt32(float f) ASMJIT_NOTHROW
{
  I32FPUnion u;
  u.f = f;
  return u.i;
}

//! @brief Binary cast 64 bit integer to DP-FP value (@c double).
static inline double int64AsDouble(Int64 i) ASMJIT_NOTHROW
{
  I64FPUnion u;
  u.i = i;
  return u.f;
}

//! @brief Binary cast DP-FP value (@c double) to 64 bit integer.
static inline Int64 doubleAsInt64(double f) ASMJIT_NOTHROW
{
  I64FPUnion u;
  u.f = f;
  return u.i;
}

// ============================================================================
// [AsmJit::(X)MMData]
// ============================================================================

//! @brief Structure used for MMX specific data (64 bits).
//!
//! This structure can be used to load / store data from / to MMX register.
union ASMJIT_HIDDEN MMData
{
  //! @brief Array of eight signed 8 bit integers.
  Int8   sb[8];
  //! @brief Array of eight unsigned 8 bit integers.
  UInt8  ub[8];
  //! @brief Array of four signed 16 bit integers.
  Int16  sw[4];
  //! @brief Array of four unsigned 16 bit integers.
  UInt16 uw[4];
  //! @brief Array of two signed 32 bit integers.
  Int32  sd[2];
  //! @brief Array of two unsigned 32 bit integers.
  UInt32 ud[2];
  //! @brief Array of one signed 64 bit integer.
  Int64  sq[1];
  //! @brief Array of one unsigned 64 bit integer.
  UInt64 uq[1];

  //! @brief Array of two SP-FP values.
  float  sf[2];

  //! @brief Set all eight signed 8 bit integers.
  inline void set_sb(Int8 x0, Int8 x1, Int8 x2, Int8 x3, Int8 x4, Int8 x5, Int8 x6, Int8 x7) ASMJIT_NOTHROW
  { sb[0] = x0; sb[1] = x1; sb[2] = x2; sb[3] = x3; sb[4] = x4; sb[5] = x5; sb[6] = x6; sb[7] = x7; }

  //! @brief Set all eight unsigned 8 bit integers.
  inline void set_ub(UInt8 x0, UInt8 x1, UInt8 x2, UInt8 x3, UInt8 x4, UInt8 x5, UInt8 x6, UInt8 x7) ASMJIT_NOTHROW
  { ub[0] = x0; ub[1] = x1; ub[2] = x2; ub[3] = x3; ub[4] = x4; ub[5] = x5; ub[6] = x6; ub[7] = x7; }

  //! @brief Set all four signed 16 bit integers.
  inline void set_sw(Int16 x0, Int16 x1, Int16 x2, Int16 x3) ASMJIT_NOTHROW
  { sw[0] = x0; sw[1] = x1; sw[2] = x2; sw[3] = x3; }

  //! @brief Set all four unsigned 16 bit integers.
  inline void set_uw(UInt16 x0, UInt16 x1, UInt16 x2, UInt16 x3) ASMJIT_NOTHROW
  { uw[0] = x0; uw[1] = x1; uw[2] = x2; uw[3] = x3; }

  //! @brief Set all two signed 32 bit integers.
  inline void set_sd(Int32 x0, Int32 x1) ASMJIT_NOTHROW
  { sd[0] = x0; sd[1] = x1; }

  //! @brief Set all two unsigned 32 bit integers.
  inline void set_ud(UInt32 x0, UInt32 x1) ASMJIT_NOTHROW
  { ud[0] = x0; ud[1] = x1; }

  //! @brief Set signed 64 bit integer.
  inline void set_sd(Int64 x0) ASMJIT_NOTHROW
  { sq[0] = x0; }

  //! @brief Set unsigned 64 bit integer.
  inline void set_ud(UInt64 x0) ASMJIT_NOTHROW
  { uq[0] = x0; }

  //! @brief Set all two SP-FP values.
  inline void set_sf(float x0, float x1) ASMJIT_NOTHROW
  { sf[0] = x0; sf[1] = x1; }
};

//! @brief Structure used for SSE specific data (128 bits).
//!
//! This structure can be used to load / store data from / to SSE register.
//!
//! @note Always align SSE data to 16 bytes.
union ASMJIT_HIDDEN XMMData
{
  //! @brief Array of sixteen signed 8 bit integers.
  Int8   sb[16];
  //! @brief Array of sixteen unsigned 8 bit integers.
  UInt8  ub[16];
  //! @brief Array of eight signed 16 bit integers.
  Int16  sw[8];
  //! @brief Array of eight unsigned 16 bit integers.
  UInt16 uw[8];
  //! @brief Array of four signed 32 bit integers.
  Int32  sd[4];
  //! @brief Array of four unsigned 32 bit integers.
  UInt32 ud[4];
  //! @brief Array of two signed 64 bit integers.
  Int64  sq[2];
  //! @brief Array of two unsigned 64 bit integers.
  UInt64 uq[2];

  //! @brief Array of four SP-FP values.
  float  sf[4];
  //! @brief Array of two DP-FP values.
  double df[2];

  inline void set_sb(
    Int8 x0, Int8 x1, Int8 x2 , Int8 x3 , Int8 x4 , Int8 x5 , Int8 x6 , Int8 x7 ,
    Int8 x8, Int8 x9, Int8 x10, Int8 x11, Int8 x12, Int8 x13, Int8 x14, Int8 x15) ASMJIT_NOTHROW
  {
    sb[0] = x0; sb[1] = x1; sb[ 2] = x2 ; sb[3 ] = x3 ; sb[4 ] = x4 ; sb[5 ] = x5 ; sb[6 ] = x6 ; sb[7 ] = x7 ;
    sb[8] = x8; sb[9] = x9; sb[10] = x10; sb[11] = x11; sb[12] = x12; sb[13] = x13; sb[14] = x14; sb[15] = x15; 
  }

  inline void set_ub(
    UInt8 x0, UInt8 x1, UInt8 x2 , UInt8 x3 , UInt8 x4 , UInt8 x5 , UInt8 x6 , UInt8 x7 ,
    UInt8 x8, UInt8 x9, UInt8 x10, UInt8 x11, UInt8 x12, UInt8 x13, UInt8 x14, UInt8 x15) ASMJIT_NOTHROW
  {
    ub[0] = x0; ub[1] = x1; ub[ 2] = x2 ; ub[3 ] = x3 ; ub[4 ] = x4 ; ub[5 ] = x5 ; ub[6 ] = x6 ; ub[7 ] = x7 ;
    ub[8] = x8; ub[9] = x9; ub[10] = x10; ub[11] = x11; ub[12] = x12; ub[13] = x13; ub[14] = x14; ub[15] = x15; 
  }

  inline void set_sw(Int16 x0, Int16 x1, Int16 x2, Int16 x3, Int16 x4, Int16 x5, Int16 x6, Int16 x7) ASMJIT_NOTHROW
  { sw[0] = x0; sw[1] = x1; sw[2] = x2; sw[3] = x3; sw[4] = x4; sw[5] = x5; sw[6] = x6; sw[7] = x7; }

  inline void set_uw(UInt16 x0, UInt16 x1, UInt16 x2, UInt16 x3, UInt16 x4, UInt16 x5, UInt16 x6, UInt16 x7) ASMJIT_NOTHROW
  { uw[0] = x0; uw[1] = x1; uw[2] = x2; uw[3] = x3; uw[4] = x4; uw[5] = x5; uw[6] = x6; uw[7] = x7; }

  inline void set_sd(Int32 x0, Int32 x1, Int32 x2, Int32 x3) ASMJIT_NOTHROW
  { sd[0] = x0; sd[1] = x1; sd[2] = x2; sd[3] = x3; }

  inline void set_ud(UInt32 x0, UInt32 x1, UInt32 x2, UInt32 x3) ASMJIT_NOTHROW
  { ud[0] = x0; ud[1] = x1; ud[2] = x2; ud[3] = x3; }

  inline void set_sd(Int64 x0, Int64 x1) ASMJIT_NOTHROW
  { sq[0] = x0; sq[1] = x1; }

  inline void set_ud(UInt64 x0, UInt64 x1) ASMJIT_NOTHROW
  { uq[0] = x0; uq[1] = x1; }

  inline void set_sf(float x0, float x1, float x2, float x3) ASMJIT_NOTHROW
  { sf[0] = x0; sf[1] = x1; sf[2] = x2; sf[3] = x3; }

  inline void set_df(double x0, double x1) ASMJIT_NOTHROW
  { df[0] = x0; df[1] = x1; }
};

// ============================================================================
// [AsmJit::Buffer]
// ============================================================================

//! @brief Buffer used to store instruction stream in AsmJit.
//! 
//! This class can be dangerous, if you don't know how it works. Assembler
//! instruction stream is usually constructed by multiple calls of emit
//! functions that emits bytes, words, dwords or qwords. But to decrease
//! AsmJit library size and improve performance, we are not checking for
//! buffer overflow for each emit operation, but only once in highler level
//! emit instruction.
//!
//! So, if you want to use this class, you need to do buffer checking yourself
//! by using @c ensureSpace() method. It's designed to grow buffer if needed.
//! Threshold for growing is named @c growThreshold() and it means count of
//! bytes for emitting single operation. Default size is set to 16 bytes,
//! because x86 and x64 instruction can't be larger (so it's space to hold 1
//! instruction).
//!
//! Example using Buffer:
//!
//! @code
//! // Buffer instance, growThreshold == 16
//! // (no memory allocated in constructor).
//! AsmJit::Buffer buf(16);
//!
//! // Begin of emit stream, ensure space can fail on out of memory error.
//! if (buf.ensureSpace()) 
//! {
//!   // here, you can emit up to 16 (growThreshold) bytes
//!   buf.emitByte(0x00);
//!   buf.emitByte(0x01);
//!   buf.emitByte(0x02);
//!   buf.emitByte(0x03);
//!   ...
//! }
//! @endcode
struct ASMJIT_API Buffer
{
  inline Buffer(SysInt growThreshold = 16) ASMJIT_NOTHROW :
    _data(NULL),
    _cur(NULL),
    _max(NULL),
    _capacity(0),
    _growThreshold(growThreshold)
  {
  }

  inline ~Buffer() ASMJIT_NOTHROW
  {
    if (_data) ASMJIT_FREE(_data);
  }

  //! @brief Return start of buffer.
  inline UInt8* data() const ASMJIT_NOTHROW { return _data; }

  //! @brief Return current pointer in code buffer.
  inline UInt8* cur() const ASMJIT_NOTHROW { return _cur; }

  //! @brief Return maximum pointer in code buffer for growing.
  inline UInt8* maximum() const ASMJIT_NOTHROW { return _max; }

  //! @brief Return current offset in buffer (same as codeSize()).
  inline SysInt offset() const ASMJIT_NOTHROW { return (SysInt)(_cur - _data); }

  //! @brief Return capacity of buffer.
  inline SysInt capacity() const ASMJIT_NOTHROW { return _capacity; }

  //! @brief Return grow threshold.
  inline SysInt growThreshold() const ASMJIT_NOTHROW { return _growThreshold; }

  //! @brief Ensure space for next instruction
  inline bool ensureSpace() ASMJIT_NOTHROW { return (_cur >= _max) ? grow() : true; }

  //! @brief Sets offset to @a o and returns previous offset.
  //!
  //! This method can be used to truncate buffer or it's used to
  //! overwrite specific position in buffer by Assembler.
  inline SysInt toOffset(SysInt o) ASMJIT_NOTHROW
  {
    ASMJIT_ASSERT(o < _capacity);

    SysInt prev = (SysInt)(_cur - _data);
    _cur = _data + o;
    return prev;
  }

  //! @brief Reallocate buffer.
  //!
  //! It's only used for growing, buffer is never reallocated to smaller 
  //! number than current capacity() is.
  bool realloc(SysInt to) ASMJIT_NOTHROW;

  //! @brief Used to grow the buffer.
  //!
  //! It will typically realloc to twice size of capacity(), but if capacity()
  //! is large, it will use smaller steps.
  bool grow() ASMJIT_NOTHROW;

  //! @brief Clear everything, but not deallocate buffer.
  void clear() ASMJIT_NOTHROW;

  //! @brief Free buffer and NULL all pointers.
  void free() ASMJIT_NOTHROW;

  //! @brief Return buffer and NULL all pointers.
  UInt8* take() ASMJIT_NOTHROW;

  //! @brief Emit Byte.
  inline void emitByte(UInt8 x) ASMJIT_NOTHROW
  {
    *_cur++ = x;
  }

  //! @brief Emit Word (2 bytes).
  inline void emitWord(UInt16 x) ASMJIT_NOTHROW
  {
    *(UInt16 *)_cur = x;
    _cur += 2;
  }

  //! @brief Emit DWord (4 bytes).
  inline void emitDWord(UInt32 x) ASMJIT_NOTHROW
  {
    *(UInt32 *)_cur = x;
    _cur += 4;
  }

  //! @brief Emit QWord (8 bytes).
  inline void emitQWord(UInt64 x) ASMJIT_NOTHROW
  {
    *(UInt64 *)_cur = x;
    _cur += 8;
  }

  //! @brief Emit system signed integer (4 or 8 bytes).
  inline void emitSysInt(SysInt x) ASMJIT_NOTHROW
  {
    *(SysInt *)_cur = x;
    _cur += sizeof(SysInt);
  }

  //! @brief Emit system unsigned integer (4 or 8 bytes).
  inline void emitSysUInt(SysUInt x) ASMJIT_NOTHROW
  {
    *(SysUInt *)_cur = x;
    _cur += sizeof(SysUInt);
  }

  //! @brief Emit custom data. 
  void emitData(const void* ptr, SysUInt len) ASMJIT_NOTHROW;

  //! @brief Set byte at position @a pos.
  inline UInt8 getByteAt(SysInt pos) const ASMJIT_NOTHROW
  {
    return *reinterpret_cast<const UInt8*>(_data + pos);
  }

  //! @brief Set word at position @a pos.
  inline UInt16 getWordAt(SysInt pos) const ASMJIT_NOTHROW
  {
    return *reinterpret_cast<const UInt16*>(_data + pos);
  }

  //! @brief Set word at position @a pos.
  inline UInt32 getDWordAt(SysInt pos) const ASMJIT_NOTHROW
  {
    return *reinterpret_cast<const UInt32*>(_data + pos);
  }

  //! @brief Set word at position @a pos.
  inline UInt64 getQWordAt(SysInt pos) const ASMJIT_NOTHROW
  {
    return *reinterpret_cast<const UInt64*>(_data + pos);
  }

  //! @brief Set byte at position @a pos.
  inline void setByteAt(SysInt pos, UInt8 x) ASMJIT_NOTHROW
  {
    *reinterpret_cast<UInt8*>(_data + pos) = x;
  }

  //! @brief Set word at position @a pos.
  inline void setWordAt(SysInt pos, UInt16 x) ASMJIT_NOTHROW
  {
    *reinterpret_cast<UInt16*>(_data + pos) = x;
  }

  //! @brief Set word at position @a pos.
  inline void setDWordAt(SysInt pos, UInt32 x) ASMJIT_NOTHROW
  {
    *reinterpret_cast<UInt32*>(_data + pos) = x;
  }

  //! @brief Set word at position @a pos.
  inline void setQWordAt(SysInt pos, UInt64 x) ASMJIT_NOTHROW
  {
    *reinterpret_cast<UInt64*>(_data + pos) = x;
  }

  // All members are public, because they can be accessed and modified by 
  // Assembler/Compiler directly.

  //! @brief Beginning position of buffer.
  UInt8* _data;
  //! @brief Current position in buffer.
  UInt8* _cur;
  //! @brief Maximum position in buffer for realloc.
  UInt8* _max;

  //! @brief Buffer capacity (in bytes).
  SysInt _capacity;

  //! @brief Grow threshold
  SysInt _growThreshold;
};

// ============================================================================
// [AsmJit::PodVector<>]
// ============================================================================

//! @brief Template used to store and manage array of POD data.
//!
//! This template has these adventages over other vector<> templates:
//! - Non-copyable (designed to be non-copyable, we want it)
//! - No copy-on-write (some implementations of stl can use it)
//! - Optimized for working only with POD types
//! - Uses ASMJIT_... memory management macros
template <typename T>
struct PodVector
{
  //! @brief Create new instance of PodVector template. Data will not
  //! be allocated (will be NULL).
  inline PodVector() ASMJIT_NOTHROW : _data(NULL), _length(0), _capacity(0)
  {
  }
  
  //! @brief Destroy PodVector and free all data.
  inline ~PodVector() ASMJIT_NOTHROW
  {
    if (_data) ASMJIT_FREE(_data);
  }

  //! @brief Return vector data.
  inline T* data() ASMJIT_NOTHROW { return _data; }
  //! @overload
  inline const T* data() const ASMJIT_NOTHROW { return _data; }
  //! @brief Return vector length.
  inline SysUInt length() const ASMJIT_NOTHROW { return _length; }
  //! @brief Return vector capacity (allocation capacity).
  inline SysUInt capacity() const ASMJIT_NOTHROW { return _capacity; }

  //! @brief Clear vector data, but not free internal buffer.
  void clear() ASMJIT_NOTHROW
  {
    _length = 0;
  }

  //! @brief Clear vector data and free internal buffer.
  void free() ASMJIT_NOTHROW
  {
    if (_data) 
    {
      ASMJIT_FREE(_data);
      _data = 0;
      _length = 0;
      _capacity = 0;
    }
  }

  //! @brief Prepend @a item to vector.
  bool prepend(const T& item) ASMJIT_NOTHROW
  {
    if (_length == _capacity && !_grow()) return false;

    memmove(_data + 1, _data, sizeof(T) * _length);
    memcpy(_data, &item, sizeof(T));

    _length++;
    return true;
  }

  bool insert(SysUInt index, const T& item) ASMJIT_NOTHROW
  {
    ASMJIT_ASSERT(index <= _length);
    if (_length == _capacity && !_grow()) return false;

    T* dst = _data + index;
    memmove(dst + 1, dst, _length - index);
    memcpy(dst, &item, sizeof(T));

    _length++;
    return true;
  }

  //! @brief Append @a item to vector.
  bool append(const T& item) ASMJIT_NOTHROW
  {
    if (_length == _capacity && !_grow()) return false;

    memcpy(_data + _length, &item, sizeof(T));

    _length++;
    return true;
  }

  //! @brief Return index of @a val or (SysUInt)-1 if not found.
  SysUInt indexOf(const T& val) const ASMJIT_NOTHROW
  {
    SysUInt i = 0, len = _length;
    for (i = 0; i < len; i++) { if (_data[i] == val) return i; }
    return (SysUInt)-1;
  }

  //! @brief Remove element at index @a i.
  void removeAt(SysUInt i)
  {
    ASMJIT_ASSERT(i < _length);

    T* dst = _data + i;
    _length--;
    memmove(dst, dst + 1, _length - i);
  }

  void swap(PodVector<T>& other)
  {
    T* _tmp_data = _data;
    SysUInt _tmp_length = _length;
    SysUInt _tmp_capacity = _capacity;

    _data = other._data;
    _length = other._length;
    _capacity = other._capacity;

    other._data = _tmp_data;
    other._length = _tmp_length;
    other._capacity = _tmp_capacity;
  }

  //! @brief Return item at @a i position.
  inline T& operator[](SysUInt i) ASMJIT_NOTHROW { ASMJIT_ASSERT(i < _length); return _data[i]; }
  //! @brief Return item at @a i position.
  inline const T& operator[](SysUInt i) const ASMJIT_NOTHROW { ASMJIT_ASSERT(i < _length); return _data[i]; }

private:
  //! @brief Called to grow internal array.
  bool _grow() ASMJIT_NOTHROW
  {
    return _realloc(_capacity < 16 ? 16 : _capacity << 1);
  }

  //! @brief Realloc internal array to fit @a to items.
  bool _realloc(SysUInt to) ASMJIT_NOTHROW
  {
    ASMJIT_ASSERT(to >= _length);

    T* p = reinterpret_cast<T*>(_data 
      ? ASMJIT_REALLOC(_data, to * sizeof(T)) 
      : ASMJIT_MALLOC(to * sizeof(T)));
    if (!p) return false;

    _data = p;
    _capacity = to;
    return true;
  }

  //! @brief Items.
  T* _data;
  //! @brief Length of buffer (count of items in array).
  SysUInt _length;
  //! @brief Capacity of buffer (maximum items that can fit to current array).
  SysUInt _capacity;

  // Disable copy.
  ASMJIT_DISABLE_COPY(PodVector<T>);
};

// ============================================================================
// [AsmJit::Zone]
// ============================================================================

//! @brief Memory allocator designed to fast alloc memory that will be freed
//! in one step.
//!
//! @note This is hackery for performance. Concept is that objects created
//! by @c Zone are freed all at once. This means that lifetime of 
//! these objects are same as zone object itselt.
//!
//! All emittables, variables, labels and states allocated by @c Compiler are
//! allocated through @c Zone object.
struct ASMJIT_API Zone
{
  // [Construction / Destruction]

  //! @brief Create new instance of @c Zone.
  //! @param chunkSize Default size for one zone chunk.
  Zone(SysUInt chunkSize) ASMJIT_NOTHROW;

  //! @brief Destroy zone instance.
  ~Zone() ASMJIT_NOTHROW;

  // [Methods]

  //! @brief Allocate @c size bytes of memory and return pointer to it.
  //!
  //! Pointer allocated by this way will be valid until @c Zone object is
  //! destroyed. To create class by this way use placement @c new and 
  //! @c delete operators:
  //!
  //! @code
  //! // Example of allocating simple class
  //!
  //! // Your class
  //! class Object
  //! {
  //!   // members...
  //! };
  //!
  //! // Your function
  //! void f()
  //! {
  //!   // We are using AsmJit namespace
  //!   using namespace AsmJit
  //!
  //!   // Create zone object with chunk size of 65536 bytes.
  //!   Zone zone(65536);
  //!
  //!   // Create your objects using zone object allocating, for example:
  //!   Object* obj = new(zone.alloc(sizeof(YourClass))) Object();
  //! 
  //!   // ... lifetime of your objects ...
  //! 
  //!   // Destroy your objects:
  //!   obj->~Object();
  //!
  //!   // Zone destructor will free all memory allocated through it, 
  //!   // alternative is to call @c zone.freeAll().
  //! }
  //! @endcode
  void* alloc(SysUInt size) ASMJIT_NOTHROW;

  //! @brief Free all allocated memory except first block that remains for reuse.
  //!
  //! Note that this method will invalidate all instances using this memory
  //! allocated by this zone instance.
  void clear() ASMJIT_NOTHROW;

  //! @brief Free all allocated memory at once.
  //!
  //! Note that this method will invalidate all instances using this memory
  //! allocated by this zone instance.
  void freeAll() ASMJIT_NOTHROW;

  //! @brief Return total size of allocated objects - by @c alloc().
  inline SysUInt total() const ASMJIT_NOTHROW { return _total; }
  //! @brief Return (default) chunk size.
  inline SysUInt chunkSize() const ASMJIT_NOTHROW { return _chunkSize; }

  // [Chunk]

  //! @brief One allocated chunk of memory.
  struct ASMJIT_HIDDEN Chunk
  {
    //! @brief Link to previous chunk.
    Chunk* prev;
    //! @brief Position in this chunk.
    SysUInt pos;
    //! @brief Size of this chunk (in bytes).
    SysUInt size;

    //! @brief Data.
    UInt8 data[sizeof(void*)];

    //! @brief Return count of remaining (unused) bytes in chunk.
    inline SysUInt remain() const ASMJIT_NOTHROW { return size - pos; }
  };

private:
  //! @brief Last allocated chunk of memory.
  Chunk* _chunks;
  //! @brief Total size of allocated objects - by @c alloc() method.
  SysUInt _total;
  //! @brief One chunk size.
  SysUInt _chunkSize;
};

//! @}

} // AsmJit namespace

#endif // _ASMJIT_UTIL_H

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
#ifndef _ASMJIT_ASSEMBLERX86X64_H
#define _ASMJIT_ASSEMBLERX86X64_H

#if !defined(_ASMJIT_ASSEMBLER_H)
#warning "AsmJit/AssemblerX86X64 can be only included by AsmJit/Assembler.h"
#endif // _ASMJIT_ASSEMBLER_H

// [Dependencies]
#include "Build.h"
#include "Serializer.h"
#include "Util.h"

// [Warnings-Push]
#include "WarningsPush.h"

// [AsmJit::]
namespace AsmJit {

//! @addtogroup AsmJit_Assembler
//! @{

//! @brief Assembler - low level code generation.
//!
//! @c Assembler is the main class in AsmJit for generating low level x86/x64 
//! binary. It creates internal buffer, where opcodes are stored and contains 
//! methods that mimics x86/x64 assembler instructions. Because 
//! @c AsmJit::Assembler is based on @c AsmJit::Serializer it supports also
//! type checking by C++ compiler. It's nearly impossible to create invalid 
//! instruction (for example <code>mov [eax], [eax]</code>).
//!
//! Each call to assembler intrinsics directly emits instruction to internal
//! binary stream. Instruction emitting also contains runtime checks so it's
//! impossible to create instruction that is not valid.
//!
//! @c AsmJit::Assembler contains internal buffer where all emitted 
//! instructions are stored. Look at @c AsmJit::Buffer for buffer 
//! implementation. To generate and allocate memory for function use
//! @c AsmJit::Assembler::make() method that will allocate memory using 
//! provided memory manager ( see @c AsmJit::MemoryManager::global() ) and
//! relocates code to provided address. If you want to create your function
//! manually, you should look at @c AsmJit::VirtualMemory and use 
//! @c AsmJit::Assembler::relocCode() method to relocate emitted code into 
//! provided memory location. You can also take emitted buffer by @c take()
//! method to do something else with it. If you take buffer, you must free it
//! manually by @c ASMJIT_FREE() macro.
//!
//! <b>Code Generation</b>
//!
//! To generate code is only needed to create instance of @c AsmJit::Assembler
//! and to use intrinsics. See example how to do that:
//! 
//! @code
//! // Use AsmJit namespace
//! using namespace AsmJit;
//!
//! // Create Assembler instance
//! Assembler a;
//!
//! // Prolog
//! a.push(ebp);
//! a.mov(ebp, esp);
//!
//! // Mov 1024 to EAX, EAX is also return value.
//! a.mov(eax, imm(1024));
//!
//! // Epilog
//! a.mov(esp, ebp);
//! a.pop(ebp);
//! a.ret();
//! @endcode
//!
//! You can see that syntax is very close to Intel one. Only difference is that
//! you are calling functions that emits the binary code for you. All registers
//! are in @c AsmJit namespace, so it's very comfortable to use it (look at
//! first line). There is also used method @c AsmJit::imm() to create immediate
//! value. Use @c AsmJit::uimm() to create unsigned immediate value.
//!
//! There is also possibility for use memory addresses and immediates. To build
//! memory address use @c ptr(), @c byte_ptr(), @c word_ptr(), @c dword_ptr()
//! and similar methods. In most cases you needs only @c ptr() method, but 
//! there are instructions where you must specify address size.
//!
//! for example (a is @c AsmJit::Assembler instance):
//!
//! @code
//! a.mov(ptr(eax), imm(0));                   // mov ptr [eax], 0
//! a.mov(ptr(eax), edx);                      // mov ptr [eax], edx
//! @endcode
//!
//! But it's also possible to create complex addresses: 
//!
//! @code
//! // eax + ecx*x addresses
//! a.mov(ptr(eax, ecx, TIMES_1), imm(0));     // mov ptr [eax + ecx], 0
//! a.mov(ptr(eax, ecx, TIMES_2), imm(0));     // mov ptr [eax + ecx * 2], 0
//! a.mov(ptr(eax, ecx, TIMES_4), imm(0));     // mov ptr [eax + ecx * 4], 0
//! a.mov(ptr(eax, ecx, TIMES_8), imm(0));     // mov ptr [eax + ecx * 8], 0
//! // eax + ecx*x + disp addresses
//! a.mov(ptr(eax, ecx, TIMES_1,  4), imm(0)); // mov ptr [eax + ecx     +  4], 0
//! a.mov(ptr(eax, ecx, TIMES_2,  8), imm(0)); // mov ptr [eax + ecx * 2 +  8], 0
//! a.mov(ptr(eax, ecx, TIMES_4, 12), imm(0)); // mov ptr [eax + ecx * 4 + 12], 0
//! a.mov(ptr(eax, ecx, TIMES_8, 16), imm(0)); // mov ptr [eax + ecx * 8 + 16], 0
//! @endcode
//!
//! All addresses shown are using @c AsmJit::ptr() to make memory operand.
//! Some assembler instructions (single operand ones) needs to specify memory
//! operand size. For example calling <code>a.inc(ptr(eax))</code> can't be
//! used. @c AsmJit::Assembler::inc(), @c AsmJit::Assembler::dec() and similar
//! instructions can't be serialized without specifying how bytes they are
//! operating on. See next code how assembler works:
//!
//! @code
//! // [byte] address
//! a.inc(byte_ptr(eax));                      // inc byte ptr [eax]
//! a.dec(byte_ptr(eax));                      // dec byte ptr [eax]
//! // [word] address
//! a.inc(word_ptr(eax));                      // inc word ptr [eax]
//! a.dec(word_ptr(eax));                      // dec word ptr [eax]
//! // [dword] address
//! a.inc(dword_ptr(eax));                     // inc dword ptr [eax]
//! a.dec(dword_ptr(eax));                     // dec dword ptr [eax]
//! @endcode
//!
//! <b>Calling Code</b>
//!
//! While you are over from emitting instructions, you can make your function
//! using @c AsmJit::Assembler::make() method. This method will use memory
//! manager to allocate virtual memory and relocates generated code to it. For
//! memory allocation is used global memory manager by default and memory is
//! freeable, but of course this default behavior can be overriden specifying
//! your memory manager and allocation type. If you want to do with code
//! something else you can always override make() method and do what you want.
//!
//! You can get size of generated code by @c codeSize() or @c offset() methods.
//! These methods returns you code size (or more precisely current code offset)
//! in bytes. Use takeCode() to take internal buffer (all pointers in 
//! @c AsmJit::Assembler instance will be zeroed and current buffer returned)
//! to use it. If you don't take it,  @c AsmJit::Assembler destructor will
//! free it automatically. To alloc and run code manually don't use
//! @c malloc()'ed memory, but instead use @c AsmJit::VirtualMemory::alloc()
//! to get memory for executing (specify @c canExecute to @c true) or
//! @c AsmJit::MemoryManager that provides more effective and comfortable way
//! to allocate virtual memory.
//!
//! See next example how to allocate memory where you can execute code created
//! by @c AsmJit::Assembler:
//!
//! @code
//! using namespace AsmJit;
//!
//! Assembler a;
//!
//! // ... your code generation 
//!
//! // your function prototype
//! typedef void (*MyFn)();
//!
//! // make your function
//! MyFn fn = function_cast<MyFn>(a.make());
//!
//! // call your function
//! fn();
//!
//! // If you don't need your function again, free it.
//! MemoryManager::global()->free(fn);
//! @endcode
//!
//! There is also low level alternative how to allocate virtual memory and 
//! relocate code to it:
//!
//! @code
//! using namespace AsmJit;
//!
//! Assembler a;
//!
//! // ... your code generation 
//!
//! // your function prototype
//! typedef void (*MyFn)();
//!
//! // alloc memory for your function
//! MyFn fn = function_cast<MyFn>(
//!   MemoryManager::global()->alloc(a.codeSize());
//!
//! // relocate code (will make the function)
//! a.relocCode(fn);
//!
//! // call your function
//! fn();
//!
//! // If you don't need your function again, free it.
//! MemoryManager::global()->free(fn);
//! @endcode
//!
//! @c note This was very primitive example how to call generated code.
//! In real production code you will never alloc and free code for one run,
//! you will usually use generated code many times.
//!
//! <b>Using labels</b>
//!
//! While generating assembler code, you will usually need to create complex
//! code with labels. Labels are fully supported and you can call @c jmp or 
//! @c je (and similar) instructions to initialized or yet uninitialized label.
//! Each label expects to be bound into offset. To bind label to specific 
//! offset, use @c bind() method.
//!
//! See next example that contains complete code that creates simple memory
//! copy function (in DWORD entities).
//!
//! @code
//! // Example: Usage of Label (32 bit code)
//! //
//! // Create simple DWORD memory copy function:
//! // ASMJIT_STDCALL void copy32(UInt32* dst, const UInt32* src, sysuint_t count);
//! using namespace AsmJit;
//!
//! // Assembler instance
//! Assembler a;
//!
//! // Constants
//! const int arg_offset = 8; // Arguments offset (STDCALL EBP)
//! const int arg_size = 12;  // Arguments size
//!
//! // Labels
//! Label L_Loop;
//!
//! // Prolog
//! a.push(ebp);
//! a.mov(ebp, esp);
//! a.push(esi);
//! a.push(edi);
//!
//! // Fetch arguments
//! a.mov(esi, dword_ptr(ebp, arg_offset + 0)); // get dst
//! a.mov(edi, dword_ptr(ebp, arg_offset + 4)); // get src
//! a.mov(ecx, dword_ptr(ebp, arg_offset + 8)); // get count
//!
//! // Bind L_Loop label to here
//! a.bind(&L_Loop);
//!
//! Copy 4 bytes
//! a.mov(eax, dword_ptr(esi));
//! a.mov(dword_ptr(edi), eax);
//!
//! // Increment pointers
//! a.add(esi, 4);
//! a.add(edi, 4);
//! 
//! // Repeat loop until ecx != 0
//! a.dec(ecx);
//! a.jz(&L_Loop);
//! 
//! // Epilog
//! a.pop(edi);
//! a.pop(esi);
//! a.mov(esp, ebp);
//! a.pop(ebp);
//! 
//! // Return: STDCALL convention is to pop stack in called function
//! a.ret(arg_size);
//! @endcode
//!
//! If you need more abstraction for generating assembler code and you want
//! to hide calling conventions between 32 bit and 64 bit operating systems,
//! look at @c Compiler class that is designed for higher level code 
//! generation.
//!
//! @sa @c Compiler.
struct ASMJIT_API Assembler : public Serializer
{
  // -------------------------------------------------------------------------
  // [Structures]
  // -------------------------------------------------------------------------

  //! @brief Data structure used to link linked-labels.
  struct LinkData
  {
    //! @brief Previous link.
    LinkData* prev;
    //! @brief Offset.
    SysInt offset;
    //! @brief Inlined displacement.
    SysInt displacement;
    //! @brief RelocId if link must be absolute when relocated.
    SysInt relocId;
  };

  // 32 bit x86 architecture uses absolute addressing model in memory operands
  // while 64 bit mode uses relative addressing model (RIP + displacement). In
  // code we are always using relative addressing model for referencing labels
  // and embedded data. In 32 bit mode we must patch all references to absolute
  // address before we can call generated function. We are patching only memory 
  // operands.

  //! @brief Reloc to absolute address data
  struct RelocData
  {
    enum Type
    {
      ABSOLUTE_TO_ABSOLUTE = 0,
      RELATIVE_TO_ABSOLUTE = 1,
      ABSOLUTE_TO_RELATIVE = 2,
      ABSOLUTE_TO_RELATIVE_TRAMPOLINE = 3
    };

    //! @brief Type of relocation.
    UInt32 type;

    //! @brief Size of relocation (4 or 8 bytes).
    UInt32 size;

    //! @brief Offset from code begin address.
    SysInt offset;

    //! @brief Relative displacement or absolute address.
    union
    {
      //! @brief Relative displacement from code begin address (not to @c offset).
      SysInt destination;
      //! @brief Absolute address where to jump;
      void* address;
    };
  };

  // -------------------------------------------------------------------------
  // [Construction / Destruction]
  // -------------------------------------------------------------------------

  //! @brief Creates Assembler instance.
  Assembler() ASMJIT_NOTHROW;
  //! @brief Destroys Assembler instance
  virtual ~Assembler() ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Buffer Getters / Setters]
  // -------------------------------------------------------------------------

  //! @brief Return start of assembler code buffer.
  //!
  //! Note that buffer address can change if you emit instruction or something
  //! else. Use this pointer only when you finished or make sure you do not
  //! use returned pointer after emitting.
  inline UInt8* code() const ASMJIT_NOTHROW
  { return _buffer.data(); }

  //! @brief Ensure space for next instruction.
  //!
  //! Note that this method can return false. It's rare and probably you never
  //! get this, but in some situations it's still possible.
  inline bool ensureSpace() ASMJIT_NOTHROW
  { return _buffer.ensureSpace(); }

  //! @brief Return current offset in buffer).
  inline SysInt offset() const ASMJIT_NOTHROW
  { return _buffer.offset(); }

  //! @brief Return current offset in buffer (same as offset() + tramplineSize()).
  inline SysInt codeSize() const ASMJIT_NOTHROW
  { return _buffer.offset() + trampolineSize(); }

  //! @brief Return size of all possible trampolines needed to successfuly generate
  //! relative jumps to absolute addresses. This value is only non-zero if jmp
  //! of call instructions were used with immediate operand (this means jump or
  //! call absolute address directly).
  //!
  //! Currently only _emitJmpOrCallReloc() method can increase trampoline size
  //! value.
  inline SysInt trampolineSize() const ASMJIT_NOTHROW
  { return _trampolineSize; }

  //! @brief Sets offset to @a o and returns previous offset.
  //!
  //! This method can be used to truncate code (previous offset is not
  //! recorded) or to overwrite instruction stream at position @a o.
  //!
  //! @return Previous offset value that can be uset to set offset back later.
  inline SysInt toOffset(SysInt o) ASMJIT_NOTHROW
  { return _buffer.toOffset(o); }

  //! @brief Return capacity of internal code buffer.
  inline SysInt capacity() const ASMJIT_NOTHROW 
  { return _buffer.capacity(); }

  //! @brief Reallocate internal buffer.
  //!
  //! It's only used for growing, buffer is never reallocated to smaller 
  //! number than current capacity() is.
  bool realloc(SysInt to) ASMJIT_NOTHROW;

  //! @brief Used to grow the buffer.
  //!
  //! It will typically realloc to twice size of capacity(), but if capacity()
  //! is large, it will use smaller steps.
  bool grow() ASMJIT_NOTHROW;

  //! @brief Clear everything, but not deallocate buffers.
  void clear() ASMJIT_NOTHROW;

  //! @brief Free internal buffer and NULL all pointers.
  void free() ASMJIT_NOTHROW;

  //! @brief Return internal buffer and NULL all pointers.
  UInt8* takeCode() ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Stream Setters / Getters]
  // -------------------------------------------------------------------------

  //! @brief Set byte at position @a pos.
  inline UInt8 getByteAt(SysInt pos) const ASMJIT_NOTHROW
  { return _buffer.getByteAt(pos); }
  
  //! @brief Set word at position @a pos.
  inline UInt16 getWordAt(SysInt pos) const ASMJIT_NOTHROW
  { return _buffer.getWordAt(pos); }
  
  //! @brief Set word at position @a pos.
  inline UInt32 getDWordAt(SysInt pos) const ASMJIT_NOTHROW
  { return _buffer.getDWordAt(pos); }
  
  //! @brief Set word at position @a pos.
  inline UInt64 getQWordAt(SysInt pos) const ASMJIT_NOTHROW
  { return _buffer.getQWordAt(pos); }

  //! @brief Set byte at position @a pos.
  inline void setByteAt(SysInt pos, UInt8 x) ASMJIT_NOTHROW
  { _buffer.setByteAt(pos, x); }
  
  //! @brief Set word at position @a pos.
  inline void setWordAt(SysInt pos, UInt16 x) ASMJIT_NOTHROW
  { _buffer.setWordAt(pos, x); }
  
  //! @brief Set word at position @a pos.
  inline void setDWordAt(SysInt pos, UInt32 x) ASMJIT_NOTHROW
  { _buffer.setDWordAt(pos, x); }
  
  //! @brief Set word at position @a pos.
  inline void setQWordAt(SysInt pos, UInt64 x) ASMJIT_NOTHROW
  { _buffer.setQWordAt(pos, x); }

  //! @brief Set word at position @a pos.
  inline Int32 getInt32At(SysInt pos) const ASMJIT_NOTHROW
  { return (Int32)_buffer.getDWordAt(pos); }
  
  //! @brief Set int32 at position @a pos.
  inline void setInt32At(SysInt pos, Int32 x) ASMJIT_NOTHROW
  { _buffer.setDWordAt(pos, (Int32)x); }

  //! @brief Set custom variable @a imm at position @a pos.
  //!
  //! @note This function is used to patch existing code.
  void setVarAt(SysInt pos, SysInt i, UInt8 isUnsigned, UInt32 size) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Assembler Emitters]
  //
  // These emitters are not protecting buffer from overrun, this must be 
  // done is _emitX86() methods by:
  //   if (!canEmit()) return;
  // -------------------------------------------------------------------------

  //! @brief Return @c true if next instruction can be emitted.
  //!
  //! This function behaves like @c ensureSpace(), but it also checks if
  //! assembler is in error state and in that case returns @c false.
  //! Assembler must internally always use this function.
  //!
  //! It's implemented like:
  //!   <code>return ensureSpace() && !error();</code>
  bool canEmit() ASMJIT_NOTHROW;

  //! @brief Emit Byte to internal buffer.
  inline void _emitByte(UInt8 x) ASMJIT_NOTHROW
  { _buffer.emitByte(x); }

  //! @brief Emit Word (2 bytes) to internal buffer.
  inline void _emitWord(UInt16 x) ASMJIT_NOTHROW
  { _buffer.emitWord(x); }

  //! @brief Emit DWord (4 bytes) to internal buffer.
  inline void _emitDWord(UInt32 x) ASMJIT_NOTHROW
  { _buffer.emitDWord(x); }

  //! @brief Emit QWord (8 bytes) to internal buffer.
  inline void _emitQWord(UInt64 x) ASMJIT_NOTHROW
  { _buffer.emitQWord(x); }

  //! @brief Emit Int32 (4 bytes) to internal buffer.
  inline void _emitInt32(Int32 x) ASMJIT_NOTHROW
  { _buffer.emitDWord((UInt32)x); }

  //! @brief Emit system signed integer (4 or 8 bytes) to internal buffer.
  inline void _emitSysInt(SysInt x) ASMJIT_NOTHROW
  { _buffer.emitSysInt(x); }

  //! @brief Emit system unsigned integer (4 or 8 bytes) to internal buffer.
  inline void _emitSysUInt(SysUInt x) ASMJIT_NOTHROW
  { _buffer.emitSysUInt(x); }

  //! @brief Emit immediate value of specified @a size.
  void _emitImmediate(const Immediate& imm, UInt32 size) ASMJIT_NOTHROW;

  //! @brief Emit single @a opCode without operands.
  inline void _emitOpCode(UInt32 opCode) ASMJIT_NOTHROW
  {
    // instruction prefix
    if (opCode & 0xFF000000) _emitByte((UInt8)((opCode & 0xFF000000) >> 24));
    // instruction opcodes
    if (opCode & 0x00FF0000) _emitByte((UInt8)((opCode & 0x00FF0000) >> 16));
    if (opCode & 0x0000FF00) _emitByte((UInt8)((opCode & 0x0000FF00) >>  8));
    // last opcode is always emitted (can be also 0x00)
    _emitByte((UInt8)(opCode & 0x000000FF));
  }

  //! @brief Emit CS (code segmend) prefix.
  //!
  //! Behavior of this function is to emit code prefix only if memory operand
  //! address uses code segment. Code segment is used through memory operand
  //! with attached @c AsmJit::Label.
  void _emitSegmentPrefix(const Operand& rm) ASMJIT_NOTHROW;

  //! @brief Emit MODR/M byte.
  //! @internal
  inline void _emitMod(UInt8 m, UInt8 o, UInt8 r) ASMJIT_NOTHROW
  { _emitByte(((m & 0x03) << 6) | ((o & 0x07) << 3) | (r & 0x07)); }

  //! @brief Emit SIB byte.
  inline void _emitSib(UInt8 s, UInt8 i, UInt8 b) ASMJIT_NOTHROW
  { _emitByte(((s & 0x03) << 6) | ((i & 0x07) << 3) | (b & 0x07)); }

  //! @brief Emit REX prefix (64 bit mode only).
  inline void _emitRexR(UInt8 w, UInt8 opReg, UInt8 regCode) ASMJIT_NOTHROW
  {
#if defined(ASMJIT_X64)
    UInt8 r = (opReg & 0x8) != 0;
    UInt8 b = (regCode & 0x8) != 0;

    // w Default operand size(0=Default, 1=64 bits).
    // r Register field (1=high bit extension of the ModR/M REG field).
    // x Index field not used in RexR
    // b Base field (1=high bit extension of the ModR/M or SIB Base field).
    if (w || r || b || (_properties & (1 << PROPERTY_X86_FORCE_REX)))
    {
      _emitByte(0x40 | (w << 3) | (r << 2) | b);
    }
#else
    ASMJIT_USE(w);
    ASMJIT_USE(opReg);
    ASMJIT_USE(regCode);
#endif // ASMJIT_X64
  }

  //! @brief Emit REX prefix (64 bit mode only).
  inline void _emitRexRM(UInt8 w, UInt8 opReg, const Operand& rm) ASMJIT_NOTHROW
  {
#if defined(ASMJIT_X64)
    UInt8 r = (opReg & 0x8) != 0;
    UInt8 x = 0;
    UInt8 b = 0;

    if (rm.isReg())
    {
      b = (reinterpret_cast<const BaseReg&>(rm).code() & 0x8) != 0;
    }
    else if (rm.isMem())
    {
      x = ((reinterpret_cast<const Mem&>(rm).index() & 0x8) != 0) & (reinterpret_cast<const Mem&>(rm).index() != NO_REG);
      b = ((reinterpret_cast<const Mem&>(rm).base() & 0x8) != 0) & (reinterpret_cast<const Mem&>(rm).base() != NO_REG);
    }

    // w Default operand size(0=Default, 1=64 bits).
    // r Register field (1=high bit extension of the ModR/M REG field).
    // x Index field (1=high bit extension of the SIB Index field).
    // b Base field (1=high bit extension of the ModR/M or SIB Base field).
    if (w || r || x || b || (_properties & (1 << PROPERTY_X86_FORCE_REX)))
    {
      _emitByte(0x40 | (w << 3) | (r << 2) | (x << 1) | b);
    }
#else
    ASMJIT_USE(w);
    ASMJIT_USE(opReg);
    ASMJIT_USE(rm);
#endif // ASMJIT_X64
  }

  //! @brief Emit Register / Register - calls _emitMod(3, opReg, r)
  inline void _emitModR(UInt8 opReg, UInt8 r) ASMJIT_NOTHROW
  { _emitMod(3, opReg, r); }

  //! @brief Emit Register / Register - calls _emitMod(3, opReg, r.code())
  inline void _emitModR(UInt8 opReg, const BaseReg& r) ASMJIT_NOTHROW
  { _emitMod(3, opReg, r.code()); }

  //! @brief Emit register / memory address combination to buffer.
  //!
  //! This method can hangle addresses from simple to complex ones with
  //! index and displacement.
  void _emitModM(UInt8 opReg, const Mem& mem, SysInt immSize) ASMJIT_NOTHROW;

  //! @brief Emit Reg<-Reg or Reg<-Reg|Mem ModRM (can be followed by SIB 
  //! and displacement) to buffer.
  //!
  //! This function internally calls @c _emitModM() or _emitModR() that depends
  //! to @a op type.
  //!
  //! @note @a opReg is usually real register ID (see @c R) but some instructions
  //! have specific format and in that cases @a opReg is part of opcode.
  void _emitModRM(UInt8 opReg, const Operand& op, SysInt immSize) ASMJIT_NOTHROW;

  //! @brief Emit instruction where register is inlined to opcode.
  void _emitX86Inl(UInt32 opCode, UInt8 i16bit, UInt8 rexw, UInt8 reg) ASMJIT_NOTHROW;

  //! @brief Emit instruction with reg/memory operand.
  void _emitX86RM(UInt32 opCode, UInt8 i16bit, UInt8 rexw, UInt8 o, 
    const Operand& op, SysInt immSize) ASMJIT_NOTHROW;

  //! @brief Emit FPU instruction with no operands.
  void _emitFpu(UInt32 opCode) ASMJIT_NOTHROW;

  //! @brief Emit FPU instruction with one operand @a sti (index of FPU register).
  void _emitFpuSTI(UInt32 opCode, UInt32 sti) ASMJIT_NOTHROW;

  //! @brief Emit FPU instruction with one operand @a opReg and memory operand @a mem.
  void _emitFpuMEM(UInt32 opCode, UInt8 opReg, const Mem& mem) ASMJIT_NOTHROW;

  //! @brief Emit MMX/SSE instruction.
  void _emitMmu(UInt32 opCode, UInt8 rexw, UInt8 opReg, const Operand& src,
    SysInt immSize) ASMJIT_NOTHROW;

  //! @brief Emit displacement.
  LinkData* _emitDisplacement(Label* label, SysInt inlinedDisplacement, int size) ASMJIT_NOTHROW;

  //! @brief Emit relative relocation to absolute pointer @a target. It's needed
  //! to add what instruction is emitting this, because in x64 mode the relative
  //! displacement can be impossible to calculate and in this case the trampoline
  //! is used.
  void _emitJmpOrCallReloc(UInt32 instruction, void* target) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Relocation helpers]
  // -------------------------------------------------------------------------

  //! @brief Relocate code to a given address @a dst.
  //!
  //! A given buffer will be overwritten, to get number of bytes required use
  //! @c codeSize() or @c offset() methods.
  virtual void relocCode(void* dst) const ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [EmitX86]
  // -------------------------------------------------------------------------

  virtual void _inlineComment(const char* text, SysInt len = -1) ASMJIT_NOTHROW;
  virtual void _emitX86(UInt32 code, const Operand* o1, const Operand* o2, const Operand* o3) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Embed]
  // -------------------------------------------------------------------------

  virtual void _embed(const void* dataPtr, SysUInt dataLen) ASMJIT_NOTHROW;
  virtual void _embedLabel(Label* label) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Align]
  // -------------------------------------------------------------------------

  virtual void align(SysInt m) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Labels]
  // -------------------------------------------------------------------------

  //! @brief Create and return new @a Label managed by assembler.
  //!
  //! Note that if you create labels by this way they are not checked like
  //! Labels statically allocated on the stack!.
  Label* newLabel() ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Bind]
  // -------------------------------------------------------------------------

  virtual void bind(Label* label) ASMJIT_NOTHROW;

  //! @brief Bind label to pos - called from bind(Label* label).
  void bindTo(Label* label, SysInt pos) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Make]
  // -------------------------------------------------------------------------

  virtual void* make(MemoryManager* memoryManager = NULL, UInt32 allocType = MEMORY_ALLOC_FREEABLE) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Links]
  // -------------------------------------------------------------------------

  LinkData* _newLinkData() ASMJIT_NOTHROW;
  void _freeLinkData(LinkData* link) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Members]
  // -------------------------------------------------------------------------

  //! @brief Binary code buffer.
  Buffer _buffer;

  //! @brief Size of possible trampolines.
  SysInt _trampolineSize;

  //! @brief Linked list of unused links (@c LinkData* structures)
  LinkData* _unusedLinks;

  //! @brief Relocations data.
  PodVector<RelocData> _relocData;

  //! @brief Buffer for inline comment (for next instruction).
  char _inlineCommentBuffer[MAX_INLINE_COMMENT_SIZE];
};

//! @}

} // AsmJit namespace

// [Warnings-Pop]
#include "WarningsPop.h"

// [Guard]
#endif // _ASMJIT_ASSEMBLERX86X64_H

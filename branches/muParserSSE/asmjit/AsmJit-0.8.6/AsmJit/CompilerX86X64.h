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
#ifndef _ASMJIT_COMPILERX86X64_H
#define _ASMJIT_COMPILERX86X64_H

#if !defined(_ASMJIT_COMPILER_H)
#warning "AsmJit/CompilerX86X64 can be only included by AsmJit/Compiler.h"
#endif // _ASMJIT_COMPILER_H

// [Dependencies]
#include "Build.h"
#include "Assembler.h"
#include "Serializer.h"
#include "Util.h"

#include <string.h>

// A little bit C++.
#include <new>

// [Warnings-Push]
#include "WarningsPush.h"

namespace AsmJit {

//! @addtogroup AsmJit_Compiler
//! @{

// ============================================================================
// [Constants]
// ============================================================================

//! @brief Calling convention type.
//!
//! Calling convention is scheme how function arguments are passed into 
//! function and how functions returns values. In assembler programming
//! it's needed to always comply with function calling conventions, because
//! even small inconsistency can cause undefined behavior or crash.
//!
//! List of calling conventions for 32 bit x86 mode:
//! - @c CALL_CONV_CDECL - Calling convention for C runtime.
//! - @c CALL_CONV_STDCALL - Calling convention for WinAPI functions.
//! - @c CALL_CONV_MSTHISCALL - Calling convention for C++ members under 
//!      Windows (produced by MSVC and all MSVC compatible compilers).
//! - @c CALL_CONV_MSFASTCALL - Fastest calling convention that can be used
//!      by MSVC compiler.
//! - @c CALL_CONV_BORNANDFASTCALL - Borland fastcall convention.
//! - @c CALL_CONV_GCCFASTCALL_2 - GCC fastcall convention with 2 register
//!      arguments.
//! - @c CALL_CONV_GCCFASTCALL_3 - GCC fastcall convention with 3 register
//!      arguments.
//!
//! List of calling conventions for 64 bit x86 mode (x64):
//! - @c CALL_CONV_X64W - Windows 64 bit calling convention (WIN64 ABI).
//! - @c CALL_CONV_X64U - Unix 64 bit calling convention (AMD64 ABI).
//!
//! There is also @c CALL_CONV_DEFAULT that is defined to fit best to your 
//! compiler.
//!
//! These types are used together with @c AsmJit::Compiler::newFunction() 
//! method.
enum CALL_CONV
{
  //! @brief Calling convention is invalid (can't be used).
  CALL_CONV_NONE = 0,

  // [X64 Calling Conventions]

  //! @brief X64 calling convention for Windows platform.
  //!
  //! For first four arguments are used these registers:
  //! - 1. 32/64 bit integer or floating point argument - rcx/xmm0
  //! - 2. 32/64 bit integer or floating point argument - rdx/xmm1
  //! - 3. 32/64 bit integer or floating point argument - r8/xmm2
  //! - 4. 32/64 bit integer or floating point argument - r9/xmm3
  //!
  //! Note first four arguments here means arguments at positions from 1 to 4
  //! (included). For example if second argument is not passed by register then
  //! rdx/xmm1 register is unused.
  //!
  //! All other arguments are pushed on the stack in right-to-left direction.
  //! Stack is aligned by 16 bytes. There is 32 bytes shadow space on the stack
  //! that can be used to save up to four 64 bit registers (probably designed to
  //! be used to save first four arguments passed in registers).
  //!
  //! Arguments direction:
  //! - Right to Left (except for first 4 parameters that's in registers)
  //!
  //! Stack is cleaned by:
  //! - Caller.
  //!
  //! Return value:
  //! - Integer types - RAX register.
  //! - Floating points - XMM0 register.
  //!
  //! Stack is always aligned by 16 bytes.
  //!
  //! More informations about this calling convention can be found on MSDN:
  //! http://msdn.microsoft.com/en-us/library/9b372w95.aspx .
  CALL_CONV_X64W = 1,

  //! @brief X64 calling convention for Unix platforms (AMD64 ABI).
  //!
  //! First six 32 or 64 bit integer arguments are passed in rdi, rsi, rdx, 
  //! rcx, r8, r9 registers. First eight floating point or XMM arguments 
  //! are passed in xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7 registers.
  //! This means that in registers can be transferred up to 14 arguments total.
  //!
  //! There is also RED ZONE below the stack pointer that can be used for 
  //! temporary storage. The red zone is the space from [rsp-128] to [rsp-8].
  //! 
  //! Arguments direction:
  //! - Right to Left (Except for arguments passed in registers).
  //!
  //! Stack is cleaned by:
  //! - Caller.
  //!
  //! Return value:
  //! - Integer types - RAX register.
  //! - Floating points - XMM0 register.
  //!
  //! Stack is always aligned by 16 bytes.
  CALL_CONV_X64U = 2,

  // [X86 Calling Conventions]

  //! @brief Cdecl calling convention (used by C runtime).
  //!
  //! Compatible across MSVC and GCC.
  //!
  //! Arguments direction:
  //! - Right to Left
  //!
  //! Stack is cleaned by:
  //! - Caller.
  CALL_CONV_CDECL = 3,

  //! @brief Stdcall calling convention (used by WinAPI).
  //!
  //! Compatible across MSVC and GCC.
  //!
  //! Arguments direction:
  //! - Right to Left
  //!
  //! Stack is cleaned by:
  //! - Callee.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - st(0) register.
  CALL_CONV_STDCALL = 4,

  //! @brief MSVC specific calling convention used by MSVC/Intel compilers
  //! for struct/class methods.
  //!
  //! This is MSVC (and Intel) only calling convention used in Windows
  //! world for C++ class methods. Implicit 'this' pointer is stored in
  //! ECX register instead of storing it on the stack.
  //!
  //! Arguments direction:
  //! - Right to Left (except this pointer in ECX)
  //!
  //! Stack is cleaned by:
  //! - Callee.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - st(0) register.
  //!
  //! C++ class methods that have variable count of arguments uses different
  //! calling convention called cdecl.
  //!
  //! @note This calling convention is always used by MSVC for class methods,
  //! it's implicit and there is no way how to override it.
  CALL_CONV_MSTHISCALL = 5,

  //! @brief MSVC specific fastcall.
  //!
  //! Two first parameters (evaluated from left-to-right) are in ECX:EDX 
  //! registers, all others on the stack in right-to-left order.
  //!
  //! Arguments direction:
  //! - Right to Left (except to first two integer arguments in ECX:EDX)
  //!
  //! Stack is cleaned by:
  //! - Callee.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - st(0) register.
  //!
  //! @note This calling convention differs to GCC one in stack cleaning
  //! mechanism.
  CALL_CONV_MSFASTCALL = 6,

  //! @brief Borland specific fastcall with 2 parameters in registers.
  //!
  //! Two first parameters (evaluated from left-to-right) are in ECX:EDX 
  //! registers, all others on the stack in left-to-right order.
  //!
  //! Arguments direction:
  //! - Left to Right (except to first two integer arguments in ECX:EDX)
  //!
  //! Stack is cleaned by:
  //! - Callee.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - st(0) register.
  //!
  //! @note Arguments on the stack are in left-to-right order that differs
  //! to other fastcall conventions used in different compilers.
  CALL_CONV_BORLANDFASTCALL = 7,

  //! @brief GCC specific fastcall with 2 parameters in registers.
  //!
  //! Two first parameters (evaluated from left-to-right) are in ECX:EDX 
  //! registers, all others on the stack in right-to-left order.
  //!
  //! Arguments direction:
  //! - Right to Left (except to first two integer arguments in ECX:EDX)
  //!
  //! Stack is cleaned by:
  //! - Caller.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - st(0) register.
  //!
  //! @note This calling convention differs to MSVC one in stack cleaning
  //! mechanism.
  CALL_CONV_GCCFASTCALL_2 = 8,

  //! @brief GCC specific fastcall with 3 parameters in registers.
  //!
  //! Three first parameters (evaluated from left-to-right) are in 
  //! ECX:EDX:EAX registers, all others on the stack in right-to-left order.
  //!
  //! Arguments direction:
  //! - Right to Left (except to first three integer arguments in ECX:EDX:EAX)
  //!
  //! Stack is cleaned by:
  //! - Caller.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - st(0) register.
  CALL_CONV_GCCFASTCALL_3 = 9,

  // [Preferred Calling Convention]

  //! @def CALL_CONV_DEFAULT
  //! @brief Default calling convention for current platform / operating 
  //! system.

#if defined(ASMJIT_X86)
  CALL_CONV_DEFAULT = CALL_CONV_CDECL
#else
# if defined(WIN32) || defined(_WIN32) || defined(WINDOWS)
  CALL_CONV_DEFAULT = CALL_CONV_X64W
# else
  CALL_CONV_DEFAULT = CALL_CONV_X64U
# endif
#endif // ASMJIT_X86
};

//! @brief Variable type.
//!
//! Variable type is used by @c AsmJit::Function::newVariable() method and can
//! be also retrieved by @c AsmJit::VariableRef::type().
enum VARIABLE_TYPE
{
  //! @brief Invalid variable type (don't use).
  VARIABLE_TYPE_NONE = 0,

  //! @brief Variable is 32 bit integer (@c Int32).
  VARIABLE_TYPE_INT32 = 1,
  //! @brief Variable is 32 bit unsigned integer (@c UInt32).
  VARIABLE_TYPE_UINT32 = 1,

  //! @var VARIABLE_TYPE_INT64
  //! @brief Variable is 64 bit signed integer (@c Int64).
  //! @note Can be used only in 64 bit mode.

  //! @var VARIABLE_TYPE_UINT64
  //! @brief Variable is 64 bit unsigned integer (@c UInt64).
  //! @note Can be used only in 64 bit mode.

  //! @var VARIABLE_TYPE_SYSINT
  //! @brief Variable is system wide integer (@c Int32 or @c Int64).
  //! @var VARIABLE_TYPE_SYSUINT
  //! @brief Variable is system wide unsigned integer (@c UInt32 or @c UInt64).
#if defined(ASMJIT_X86)
  VARIABLE_TYPE_SYSINT = VARIABLE_TYPE_INT32,
  VARIABLE_TYPE_SYSUINT = VARIABLE_TYPE_UINT32,
#else
  VARIABLE_TYPE_INT64 = 2,
  VARIABLE_TYPE_UINT64 = 2,
  VARIABLE_TYPE_SYSINT = VARIABLE_TYPE_INT64,
  VARIABLE_TYPE_SYSUINT = VARIABLE_TYPE_UINT64,
#endif

  //! @brief Variable is pointer or reference to memory (or any type).
  VARIABLE_TYPE_PTR = VARIABLE_TYPE_SYSUINT,

  //! @brief Variable is X87 (FPU) SP-FP number (float).
  //!
  //! TODO: Float registers allocation is not supported.
  VARIABLE_TYPE_X87_FLOAT = 3,

  //! @brief Variable is X87 (FPU) DP-FP number (double).
  //!
  //! TODO: Double registers allocation is not supported.
  VARIABLE_TYPE_X87_DOUBLE = 4,

  //! @brief Variable is SSE scalar SP-FP number.
  VARIABLE_TYPE_XMM_FLOAT = 5,

  //! @brief Variable is SSE2 scalar DP-FP number.
  VARIABLE_TYPE_XMM_DOUBLE = 6,

  //! @brief Variable is SSE packed SP-FP number (4 floats).
  VARIABLE_TYPE_XMM_FLOAT_4 = 7,
  //! @brief Variable is SSE2 packed DP-FP number (2 doubles).
  VARIABLE_TYPE_XMM_DOUBLE_2 = 8,

#if defined(ASMJIT_X86)
  VARIABLE_TYPE_FLOAT = VARIABLE_TYPE_X87_FLOAT,
  VARIABLE_TYPE_DOUBLE = VARIABLE_TYPE_X87_DOUBLE,
#else
  VARIABLE_TYPE_FLOAT = VARIABLE_TYPE_XMM_FLOAT,
  VARIABLE_TYPE_DOUBLE = VARIABLE_TYPE_XMM_DOUBLE,
#endif

  //! @brief Variable is MM register / memory location.
  VARIABLE_TYPE_MM = 9,

  //! @brief Variable is XMM register / memory location.
  VARIABLE_TYPE_XMM = 10,

  //! @brief Count of variable types.
  _VARIABLE_TYPE_COUNT
};

// ============================================================================
// [AsmJit::Variable]
// ============================================================================

//! @brief Variable.
//!
//! Variables reresents registers or memory locations that can be allocated by
//! @c Function. Each function arguments are also allocated as variables, so
//! accessing function arguments is similar to accessing function variables.
//!
//! Variables can be declared by @c AsmJit::Function::newVariable() or by 
//! declaring function arguments by AsmJit::Compiler::newFunction(). Compiler
//! always returns variables as pointers to @c Variable instance.
//!
//! Variable instances are never accessed directly, instead there are wrappers.
//! Wrappers are designed to simplify variables management and it's lifetime.
//! Because variables are based on reference counting, each variable that is
//! returned from @c Compiler needs to be wrapped in @c VariableRef or similar
//! (@c Int32Ref, @c Int64Ref, @c SysIntRef, @c PtrRef, @c MMRef, @c XMMRef)
//! classes. Each wrapper class is designed to wrap specific variable type. For
//! example integer should be always wrapped into @c Int32Ref or @c Int64Ref,
//! MMX register to @c MMRef, etc...
//!
//! Variable wrapping is also needed, because it's lifetime is based on 
//! reference counting. Each variable returned from compiler has reference 
//! count equal to zero! So wrapper class increases it to one (or more that
//! depends how much you wrapped it) and destroying wrapper means decreasing
//! reference count. If reference count is decreased to zero, variable life
//! ends, it's marked as unused and compiler can reuse it later.
//!
//! @sa @c VariableRef, @c Int32Ref, @c Int64Ref, @c SysIntRef, @c PtrRef, 
//! @c MMRef, @c XMMRef.
struct ASMJIT_API Variable
{
  // [Typedefs]

  //! @brief Custom alloc function type.
  typedef void (*AllocFn)(Variable* v);
  //! @brief Custom spill function type.
  typedef void (*SpillFn)(Variable* v);

  // [Construction / Destruction]

  //! @brief Create a new @a Variable instance.
  //!
  //! Always use @c AsmJit::Function::newVariable() method to create 
  //! @c Variable.
  Variable(Compiler* c, Function* f, UInt8 type);
  //! @brief Destroy variable instance.
  //!
  //! Never destroy @c Variable instance created by @c Compiler.
  virtual ~Variable();

  // [Methods]

  //! @brief Return compiler that owns this variable.
  inline Compiler* compiler() const ASMJIT_NOTHROW
  { return _compiler; }

  //! @brief Return function that owns this variable.
  inline Function* function() const ASMJIT_NOTHROW
  { return _function; }

  //! @brief Return reference count.
  inline SysUInt refCount() const ASMJIT_NOTHROW
  { return _refCount; }

  //! @brief Return spill count.
  inline SysUInt spillCount() const ASMJIT_NOTHROW
  { return _spillCount; }
  
  //! @brief Return life id.
  inline SysUInt lifeId() const ASMJIT_NOTHROW
  { return _lifeId; }
  
  //! @brief Return register access statistics.
  inline SysUInt registerAccessCount() const ASMJIT_NOTHROW
  { return _registerAccessCount; }
  
  //! @brief Return memory access statistics.
  inline SysUInt memoryAccessCount() const ASMJIT_NOTHROW
  { return _memoryAccessCount; }

  //! @brief Return variable type, see @c VARIABLE_TYPE.
  inline UInt8 type() const ASMJIT_NOTHROW
  { return _type; }

  //! @brief Return variable size (in bytes).
  inline UInt8 size() const ASMJIT_NOTHROW
  { return _size; }

  //! @brief Return variable state, see @c VARIABLE_STATE.
  inline UInt8 state() const ASMJIT_NOTHROW
  { return _state; }

  //! @brief Return variable priority.
  //!
  //! Variable priority is used for spilling. Lower number means less chance
  //! to spill. Zero means that variable can't be never spilled.
  inline UInt8 priority() const ASMJIT_NOTHROW
  { return _priority; }

  //! @brief Return variable register code (where it now lives), or NO_REG if
  //! it's only in memory (spilled).
  inline UInt8 registerCode() const ASMJIT_NOTHROW
  { return _registerCode; }

  //! @brief Return variable preferred register.
  inline UInt8 preferredRegisterCode() const ASMJIT_NOTHROW
  { return _preferredRegisterCode; }

  //! @brief Return variable home register code (this is usually last 
  //! allocated register code).
  inline UInt8 homeRegisterCode() const ASMJIT_NOTHROW
  { return _homeRegisterCode; }

  //! @brief Return variable changed state.
  inline UInt8 changed() const ASMJIT_NOTHROW
  { return _changed; }
  
  //! @brief Return whether variable is reusable.
  inline UInt8 reusable() const ASMJIT_NOTHROW
  { return _reusable; }

  //! @brief Return whether variable contains custom memory home address.
  inline UInt8 customMemoryHome() const ASMJIT_NOTHROW
  { return _customMemoryHome; }

  //! @brief Return whether variable contains custom memory home address.
  inline UInt8 stackArgument() const ASMJIT_NOTHROW
  { return _stackArgument; }

  //! @brief Return variable stack offset.
  //!
  //! @note Stack offsets can be changed by Compiler, don't use this 
  //! to generate memory operands.
  inline SysInt stackOffset() const ASMJIT_NOTHROW
  { return _stackOffset; }

  //! @brief Set variable priority.
  void setPriority(UInt8 priority);

  //! @brief Set varialbe preferred register.
  inline void setPreferredRegisterCode(UInt8 code) ASMJIT_NOTHROW
  { _preferredRegisterCode = code; }

  //! @brief Set variable changed state.
  inline void setChanged(UInt8 changed) ASMJIT_NOTHROW
  { _changed = changed; }

  //! @brief Memory operand that will be always pointed to variable memory address. */
  inline const Mem& memoryOperand() const ASMJIT_NOTHROW
  { return *_memoryOperand; }

  void setMemoryHome(const Mem& memoryHome) ASMJIT_NOTHROW;

  // [Reference counting]

  //! @brief Increase reference count and return itself.
  Variable* ref();

  //! @brief Decrease reference count. If reference is decreased to zero, 
  //! variable is marked as unused and deallocated.
  void deref();

  // [Code Generation]

  //! @brief Allocate variable to register.
  //! @param mode Allocation mode (see @c VARIABLE_ALLOC enum)
  //! @param preferredRegister Preferred register to use (see @c AsmJit::REG enum).
  inline bool alloc(
    UInt8 mode = VARIABLE_ALLOC_READWRITE, 
    UInt8 preferredRegister = NO_REG);

  //! @brief Allocate variable to register and return it in @a dest.
  //!
  //! This function is similar to @c alloc(), but setups @a dest register. It's
  //! convenience method for @c AsmJit::VariableRef register accesses.
  void getReg(
    UInt8 mode, UInt8 preferredRegister,
    BaseReg* dest, UInt8 regType);

  const Mem& m();

  //! @brief Spill variable (move to memory).
  inline bool spill();

  //! @brief Unuse variable
  //!
  //! This will completely destroy variable. After @c unuse() you can use
  //! @c alloc() to allocate it again.
  inline void unuse();

  // [Custom Spill / Restore]

  //! @brief Return @c true if this variable uses custom alloc or spill 
  //! functions (this means bypassing built-in functions).
  //!
  //! @note If alloc or spill function is set, variable is marked as custom
  //! and there is no chance to move it to / from stack. For example mmx zero
  //! register can be implemented in allocFn() that will emit pxor(mm, mm).
  //! Variable will never spill into stack in this case.
  inline bool isCustom() const ASMJIT_NOTHROW
  { return _allocFn != NULL || _spillFn != NULL; }

  //! @brief Get custom alloc function.
  inline AllocFn allocFn() const ASMJIT_NOTHROW
  { return _allocFn; }

  //! @brief Get custom spill function.
  inline SpillFn spillFn() const ASMJIT_NOTHROW
  { return _spillFn; }

  //! @brief Get custom data pointer.
  inline void* dataPtr() const ASMJIT_NOTHROW
  { return _dataPtr; }

  //! @brief Get custom data pointer.
  inline SysInt dataInt() const ASMJIT_NOTHROW
  { return _dataInt; }

  //! @brief Set custom alloc function.
  inline void setAllocFn(AllocFn fn) ASMJIT_NOTHROW
  { _allocFn = fn; }

  //! @brief Set custom spill function.
  inline void setSpillFn(SpillFn fn) ASMJIT_NOTHROW
  { _spillFn = fn; }

  //! @brief Set custom data pointer.
  inline void setDataPtr(void* data) ASMJIT_NOTHROW
  { _dataPtr = data; }

  //! @brief Set custom data integer.
  inline void setDataInt(SysInt data) ASMJIT_NOTHROW
  { _dataInt = data; }

  //! @brief Get variable name.
  inline const char* name() const ASMJIT_NOTHROW
  { return _name; }

  //! @brief Set variable name.
  void setName(const char* name) ASMJIT_NOTHROW;

private:
  //! @brief Set variable stack offset.
  //! @internal
  inline void setStackOffset(SysInt stackOffset) ASMJIT_NOTHROW
  { _stackOffset = stackOffset; }

  //! @brief Set most members by one shot.
  inline void setAll(
    UInt8 type, UInt8 size, UInt8 state, UInt8 priority, 
    UInt8 registerCode, UInt8 preferredRegisterCode,
    SysInt stackOffset) ASMJIT_NOTHROW
  {
    _type = type;
    _size = size;
    _state = state;
    _priority = priority;
    _registerCode = registerCode;
    _preferredRegisterCode = preferredRegisterCode;
    _stackOffset = stackOffset;
  }

  //! @brief Compiler that owns this variable.
  Compiler* _compiler;
  //! @brief Function that owns this variable.
  Function* _function;

  //! @brief Reference count.
  SysUInt _refCount;

  //! @brief How many times was variable spilled (in current context).
  SysUInt _spillCount;
  //! @brief Register access count (in current context).
  SysUInt _registerAccessCount;
  //! @brief Memory access count (in current context).
  SysUInt _memoryAccessCount;

  //! @brief Current variable life ID (also means how many times was variable reused).
  SysUInt _lifeId;

  //! @brief How many times was variable spilled (in current context).
  SysUInt _globalSpillCount;
  //! @brief Register access count (in current context).
  SysUInt _globalRegisterAccessCount;
  //! @brief Memory access count (in current context).
  SysUInt _globalMemoryAccessCount;

  //! @brief Variable type, see @c VARIABLE_TYPE.
  UInt8 _type;

  //! @brief Variable size (in bytes).
  UInt8 _size;

  //! @brief Variable state, see @c VARIABLE_STATE.
  UInt8 _state;

  //! @brief Variable priority.
  UInt8 _priority;

  //! @brief Register code if variable state is @c VARIABLE_STATE_REGISTER.
  UInt8 _registerCode;

  //! @brief Default register where to alloc variable.
  UInt8 _preferredRegisterCode;

  //! @brief Last allocated register code.
  UInt8 _homeRegisterCode;

  //! @brief true if variable in register was changed and when spilling it 
  //! needs to be copied into memory location.
  UInt8 _changed;

  //! @brief Whether variable can be reused.
  UInt8 _reusable;

  //! @brief Whether variable contains custom home address.
  UInt8 _customMemoryHome;

  //! @brief Whether variable is function argument passed onto the stack.
  UInt8 _stackArgument;

  //! @brief Stack location.
  SysInt _stackOffset;

  //! @brief Variable memory operand.
  Mem* _memoryOperand;

  //! @brief Custom alloc function (or NULL).
  AllocFn _allocFn;

  //! @brief Custom spill function (or NULL).
  SpillFn _spillFn;

  //! @brief Custom void* data that can be used by custom spill and restore functions.
  void* _dataPtr;

  //! @brief Custom integer that can be used by custom spill and restore functions.
  SysInt _dataInt;

  //! @brief Variable name.
  char _name[MAX_VARIABLE_LENGTH];

  friend struct CompilerCore;
  friend struct Function;
  friend struct VariableRef;

  // Disable copy.
  ASMJIT_DISABLE_COPY(Variable);
};

// ============================================================================
// [AsmJit::XXXRef]
// ============================================================================

//! @brief Base class for variable wrappers.
//!
//! @c VariableRef class is designed to manage @c Variable instances. It's 
//! based on reference counting and if reference gets to zero (in destructor), 
//! variable is freed by compiler. This helps with scoping variables and 
//! minimizes mistakes that can be done with manual allocation / freeing.
//!
//! @note Compiler can reuse existing variables if reference gets zero.
//!
//! @sa @c Variable,
//!     @c Int32Ref, @c Int64Ref, @c SysIntRef, @c PtrRef, @c MMRef, @c XMMRef.
struct ASMJIT_HIDDEN VariableRef
{
  // [Typedefs]

  typedef Variable::AllocFn AllocFn;
  typedef Variable::SpillFn SpillFn;

  // [Construction / Destruction]

  //! @brief Create new uninitialized variable reference.
  //!
  //! Using uninitialized variable reference is forbidden.
  inline VariableRef() : _v(NULL) {}

  //! @brief Reference variable @a v (@a v can't be @c NULL).
  inline VariableRef(Variable* v) : _v(v->ref()) {}

  //! @brief Dereference variable if it's wrapped.
  inline ~VariableRef() { if (_v) _v->deref(); }

  inline VariableRef& operator=(Variable* v)
  {
    use(v); return *this;
  }

  //! @brief Return @c Variable instance.
  inline Variable* v() const { return _v; }

  // [Methods]

  //! @brief Return variable type, see @c VARIABLE_TYPE.
  inline UInt8 type() const { ASMJIT_ASSERT(_v); return _v->type(); }
  //! @brief Return variable size (in bytes).
  inline UInt8 size() const { ASMJIT_ASSERT(_v); return _v->size(); }
  //! @brief Return variable state, see @c VARIABLE_STATE.
  inline UInt8 state() const { ASMJIT_ASSERT(_v); return _v->state(); }

  void use(Variable* v)
  {
    Variable* tmp = v->ref();
    if (_v) _v->deref();
    _v = tmp;
  }

  //! @brief Allocate variable to register.
  //! @param mode Allocation mode (see @c VARIABLE_ALLOC enum)
  //! @param preferredRegister Preferred register to use (see @c REG enum).
  inline bool alloc(
    UInt8 mode = VARIABLE_ALLOC_READWRITE, 
    UInt8 preferredRegister = NO_REG)
  {
    ASMJIT_ASSERT(_v);
    return _v->alloc(mode, preferredRegister);
  }

  //! @brief Spill variable (move to memory).
  inline bool spill()
  {
    ASMJIT_ASSERT(_v);
    return _v->spill();
  }

  //! @brief Unuse variable
  //!
  //! This will completely destroy variable. After @c unuse() you can use
  //! @c alloc() to allocate it again.
  inline void unuse()
  {
    if (_v) _v->unuse();
  }

  //! @brief Destroy variable (@c VariableRef can't be used anymore after destroy).
  inline void destroy()
  {
    if (_v)
    {
      _v->deref();
      _v = NULL;
    }
  }

  //! @brief Get variable preferred register code.
  inline UInt8 preferredRegisterCode() const
  { ASMJIT_ASSERT(_v); return _v->preferredRegisterCode(); }

  //! @brief Set variable preferred register code to @a code.
  //! @param code Preferred register code (see @c AsmJit::REG enum).
  inline void setPreferredRegisterCode(UInt8 code)
  { ASMJIT_ASSERT(_v); _v->setPreferredRegisterCode(code); }

  //! @brief Get variable home register code.
  inline UInt8 homeRegisterCode() const
  { ASMJIT_ASSERT(_v); return _v->homeRegisterCode(); }

  //! @brief Get variable priority.
  inline UInt8 priority() const
  { ASMJIT_ASSERT(_v); return _v->priority(); }

  //! @brief Set variable priority.
  inline void setPriority(UInt8 priority)
  { ASMJIT_ASSERT(_v); _v->setPriority(priority); }

  //! @brief Return if variable changed state.
  inline UInt8 changed() const
  { ASMJIT_ASSERT(_v); return _v->changed(); }

  //! @brief Set variable changed state.
  inline void setChanged(UInt8 changed)
  { ASMJIT_ASSERT(_v); _v->setChanged(changed); }

  //! @brief Return whether variable is reusable.
  inline UInt8 reusable() const ASMJIT_NOTHROW
  { ASMJIT_ASSERT(_v); return _v->reusable(); }

  //! @brief Return whether variable contains custom memory home address.
  inline UInt8 customMemoryHome() const ASMJIT_NOTHROW
  { ASMJIT_ASSERT(_v); return _v->customMemoryHome(); }

  inline void setMemoryHome(const Mem& memoryHome) ASMJIT_NOTHROW
  { ASMJIT_ASSERT(_v); _v->setMemoryHome(memoryHome); }

  //! @brief Return memory address operand.
  //!
  //! @note Getting memory address operand will always call @c spill().
  inline const Mem& m() const
  { ASMJIT_ASSERT(_v); return _v->m(); }

  // [Reference counting]

  //! @brief Increase reference count and return @c Variable instance.
  inline Variable* ref()
  { ASMJIT_ASSERT(_v); return _v->ref(); }

  // [Custom Spill / Restore]

  //! @brief Return @c true if variable uses custom alloc / spill functions.
  //!
  //! @sa @c AsmJit::Variable::isCustom() method.
  inline bool isCustom() const { ASMJIT_ASSERT(_v); return _v->isCustom(); }

  //! @brief Get custom restore function.
  inline AllocFn allocFn() const { ASMJIT_ASSERT(_v); return _v->allocFn(); }
  //! @brief Get custom spill function.
  inline SpillFn spillFn() const { ASMJIT_ASSERT(_v); return _v->spillFn(); }
  //! @brief Get custom data pointer.
  inline void* dataPtr() const { ASMJIT_ASSERT(_v); return _v->dataPtr(); }
  //! @brief Get custom data pointer.
  inline SysInt dataInt() const { ASMJIT_ASSERT(_v); return _v->dataInt(); }

  //! @brief Set custom restore function.
  inline void setAllocFn(AllocFn fn) { ASMJIT_ASSERT(_v); _v->setAllocFn(fn); }
  //! @brief Set custom spill function.
  inline void setSpillFn(SpillFn fn) { ASMJIT_ASSERT(_v); _v->setSpillFn(fn); }
  //! @brief Set custom data.
  inline void setDataPtr(void* data) { ASMJIT_ASSERT(_v); _v->setDataPtr(data); }
  //! @brief Set custom data.
  inline void setDataInt(SysInt data) { ASMJIT_ASSERT(_v); _v->setDataInt(data); }
  
  inline const char* name() const { ASMJIT_ASSERT(_v); return _v->name(); }
  inline void setName(const char* name) const { ASMJIT_ASSERT(_v); _v->setName(name); }

  inline bool operator==(const VariableRef& other) const { return _v == other._v; }
  inline bool operator!=(const VariableRef& other) const { return _v != other._v; }

protected:
  void _assign(const VariableRef& other)
  {
    Variable* tmp = other._v ? other._v->ref() : NULL;
    if (_v) _v->deref();
    _v = tmp;
  }

  //! @brief @c Variable instance.
  Variable* _v;

private:
  // Disable copy.
  ASMJIT_DISABLE_COPY(VariableRef);
};

// Helper macro for VariableRef::... register access methods.
#define __REG_ACCESS(__regClass__, __allocType__, __preferredRegCode__, __regType__) \
  ASMJIT_ASSERT(_v); \
  __regClass__ result; \
  _v->getReg(__allocType__, __preferredRegCode__, (BaseReg*)&result, __regType__); \
  return result

//! @brief 32 bit integer variable wrapper.
struct ASMJIT_HIDDEN Int32Ref : public VariableRef
{
  // [Construction / Destruction]

  inline Int32Ref() : VariableRef() {}
  inline Int32Ref(const Int32Ref& other) : VariableRef(other._v) {}
  inline Int32Ref(Variable* v) : VariableRef(v) {}

  inline Int32Ref& operator=(const Int32Ref& other) { _assign(other); return *this; }

  // [Registers]

  inline Register r  (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READWRITE, pref, REG_GPD); }
  inline Register r8 (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READWRITE, pref, REG_GPB); }
  inline Register r16(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READWRITE, pref, REG_GPW); }
  inline Register r32(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READWRITE, pref, REG_GPD); }
#if defined(ASMJIT_X64)
  inline Register r64(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READWRITE, pref, REG_GPQ); }
#endif // ASMJIT_X64

  inline Register c  (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READ     , pref, REG_GPD); }
  inline Register c8 (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READ     , pref, REG_GPB); }
  inline Register c16(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READ     , pref, REG_GPW); }
  inline Register c32(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READ     , pref, REG_GPD); }
#if defined(ASMJIT_X64)
  inline Register c64(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READ     , pref, REG_GPQ); }
#endif // ASMJIT_X64

  inline Register x  (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_WRITE    , pref, REG_GPD); }
  inline Register x8 (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_WRITE    , pref, REG_GPB); }
  inline Register x16(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_WRITE    , pref, REG_GPW); }
  inline Register x32(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_WRITE    , pref, REG_GPD); }
#if defined(ASMJIT_X64)
  inline Register x64(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_WRITE    , pref, REG_GPQ); }
#endif // ASMJIT_X64
};

#if defined(ASMJIT_X64)
//! @brief 64 bit integer variable wrapper.
struct ASMJIT_HIDDEN Int64Ref : public VariableRef
{
  // [Construction / Destruction]

  inline Int64Ref() : VariableRef() {}
  inline Int64Ref(const Int64Ref& other) : VariableRef(other._v) {}
  inline Int64Ref(Variable* v) : VariableRef(v) {}

  inline Int64Ref& operator=(const Int64Ref& other) { _assign(other); return *this; }

  // [Registers]

  inline Register r  (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READWRITE, pref, REG_GPQ); }
  inline Register r8 (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READWRITE, pref, REG_GPB); }
  inline Register r16(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READWRITE, pref, REG_GPW); }
  inline Register r32(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READWRITE, pref, REG_GPD); }
  inline Register r64(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READWRITE, pref, REG_GPQ); }

  inline Register c  (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READ     , pref, REG_GPQ); }
  inline Register c8 (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READ     , pref, REG_GPB); }
  inline Register c16(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READ     , pref, REG_GPW); }
  inline Register c32(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READ     , pref, REG_GPD); }
  inline Register c64(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_READ     , pref, REG_GPQ); }

  inline Register x  (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_WRITE    , pref, REG_GPQ); }
  inline Register x8 (UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_WRITE    , pref, REG_GPB); }
  inline Register x16(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_WRITE    , pref, REG_GPW); }
  inline Register x32(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_WRITE    , pref, REG_GPD); }
  inline Register x64(UInt8 pref = NO_REG) const { __REG_ACCESS(Register, VARIABLE_ALLOC_WRITE    , pref, REG_GPQ); }
};
#endif // ASMJIT_X64

//! @brief MMX variable wrapper.
struct ASMJIT_HIDDEN MMRef : public VariableRef
{
  // [Construction / Destruction]

  inline MMRef() : VariableRef() {}
  inline MMRef(const MMRef& other) : VariableRef(other._v) {}
  inline MMRef(Variable* v) : VariableRef(v) {}

  inline MMRef& operator=(const MMRef& other) { _assign(other); return *this; }

  // [Registers]

  inline MMRegister r(UInt8 pref = NO_REG) const { __REG_ACCESS(MMRegister, VARIABLE_ALLOC_READWRITE, pref, REG_MM); }
  inline MMRegister c(UInt8 pref = NO_REG) const { __REG_ACCESS(MMRegister, VARIABLE_ALLOC_READ     , pref, REG_MM); }
  inline MMRegister x(UInt8 pref = NO_REG) const { __REG_ACCESS(MMRegister, VARIABLE_ALLOC_WRITE    , pref, REG_MM); }
};

//! @brief SSE variable wrapper.
struct ASMJIT_HIDDEN XMMRef : public VariableRef
{
  // [Construction / Destruction]

  inline XMMRef() : VariableRef() {}
  inline XMMRef(const XMMRef& other) : VariableRef(other._v) {}
  inline XMMRef(Variable* v) : VariableRef(v) {}

  inline XMMRef& operator=(const XMMRef& other) { _assign(other); return *this; }

  // [Registers]

  inline XMMRegister r(UInt8 pref = NO_REG) const { __REG_ACCESS(XMMRegister, VARIABLE_ALLOC_READWRITE, pref, REG_XMM); }
  inline XMMRegister c(UInt8 pref = NO_REG) const { __REG_ACCESS(XMMRegister, VARIABLE_ALLOC_READ     , pref, REG_XMM); }
  inline XMMRegister x(UInt8 pref = NO_REG) const { __REG_ACCESS(XMMRegister, VARIABLE_ALLOC_WRITE    , pref, REG_XMM); }
};

#if defined(ASMJIT_X86)
typedef Int32Ref SysIntRef;
#else
typedef Int64Ref SysIntRef;
#endif

//! @brief Pointer variable wrapper (same as system integer).
typedef SysIntRef PtrRef;

// Cleanup
#undef __REG_ACCESS

// ============================================================================
// [AsmJit::State]
// ============================================================================

//! @brief Contains informations about current register state.
//!
//! @note Always use StateRef to manage register states and don't create State
//! directly. Instead use @c AsmJit::Function::saveState() and 
//! @c AsmJit::Function::restoreState() methods.
//!
//! @sa StateRef
struct ASMJIT_API State
{
  // [Construction / Destruction]

  State(Compiler* c, Function* f);
  virtual ~State();

  //! @brief State entry (variable and some details about variable life cycle).
  struct Entry
  {
    Variable* v;
    UInt32 lifeId;
    UInt8 state;
    UInt8 changed;
  };

  //! @brief State data (allocated registers <-> variables).
  struct Data 
  {
    union
    {
      //! @brief All variables in one array.
      Entry regs[16+8+16];

      struct {
        //! @brief Regeral purpose registers.
        Entry gp[16];
        //! @brief MMX registers.
        Entry mm[8];
        //! @brief XMM registers.
        Entry xmm[16];
      };
    };

    //! @brief used GP registers bitmask.
    UInt32 usedGpRegisters;
    //! @brief used MMX registers bitmask.
    UInt32 usedMmRegisters;
    //! @brief used XMM registers bitmask.
    UInt32 usedXmmRegisters;
  };

  static void saveFunctionState(Data* dst, Function* f);

private:
  //! @brief Clear state.
  void _clear();

  //! @brief Save function state (there is no code generated when saving state).
  void _save();
  //! @brief Restore function state, can spill and alloc registers.
  void _restore();

  //! @brief Set function state to current state.
  //!
  //! @note This method is similar to @c _restore(), but it will not alloc or 
  //! spill registers.
  void _set();

  //! @brief Compiler this state is related to.
  Compiler* _compiler;

  //! @brief Function this state is related to.
  Function* _function;

  //! @brief State data.
  Data _data;

  friend struct CompilerCore;
  friend struct Function;
  friend struct StateRef;

  // Disable copy.
  ASMJIT_DISABLE_COPY(State);
};

// ============================================================================
// [AsmJit::StateRef]
// ============================================================================

//! @brief State wrapper used to manage @c State's.
struct ASMJIT_HIDDEN StateRef
{
  //! @brief Create StateRef instance from @a state (usually returned by 
  //! @c Compiler / @c Function).
  inline StateRef(State* state) :
    _state(state)
  {}

  //! @brief Destroy StateRef instance.
  inline ~StateRef();

  //! @brief Return managed @c State instance.
  inline State* state() const { return _state; }

  //! @brief Implicit cast to @c State.
  inline operator State*() const { return _state; }

private:
  State* _state;

  // Disable copy.
  ASMJIT_DISABLE_COPY(StateRef);
};

// ============================================================================
// [AsmJit::Emittable]
// ============================================================================

//! @brief Emmitable.
//!
//! Emittable is object that can emit single or more instructions. To
//! create your custom emittable it's needed to override abstract virtual
//! method @c emit().
//!
//! When you are finished serializing instructions to the @c Compiler and you
//! call @c Compiler::make(), it will first call @c prepare() method for each
//! emittable in list, then @c emit() and then @c postEmit(). Prepare can be
//! used to calculate something that can be only calculated when everything
//! is finished (for example @c Function uses @c prepare() to relocate memory
//! home for all memory/spilled variables). @c emit() should be used to emit
//! instruction or multiple instructions into @a Assembler stream, and
//! @c postEmit() is here to allow emitting embedded data (after function
//! declaration), etc.
struct ASMJIT_API Emittable
{
  // [Construction / Destruction]

  //! @brief Create new emittable.
  //!
  //! Never create @c Emittable by @c new operator or on the stack, use
  //! @c Compiler::newObject template to do that.
  Emittable(Compiler* c, UInt32 type) ASMJIT_NOTHROW;

  //! @brief Destroy emittable.
  //!
  //! @note Never destroy emittable using @c delete keyword, @c Compiler
  //! manages all emittables in internal memory pool and it will destroy
  //! all emittables after you destroy it.
  virtual ~Emittable() ASMJIT_NOTHROW;

  // [Emit]

  //! @brief Prepare for emitting (optional).
  virtual void prepare();
  //! @brief Emit instruction stream.
  virtual void emit(Assembler& a) = 0;
  //! @brief Post emit (optional).
  virtual void postEmit(Assembler& a);

  // [Methods]

  //! @brief Return compiler instance where this emittable is connected to.
  inline Compiler* compiler() const ASMJIT_NOTHROW { return _compiler; }
  //! @brief Return previsou emittable in list.
  inline Emittable* prev() const ASMJIT_NOTHROW { return _prev; }
  //! @brief Return next emittable in list.
  inline Emittable* next() const ASMJIT_NOTHROW { return _next; }
  //! @brief Return emittable type, see @c EMITTABLE_TYPE.
  inline UInt32 type() const ASMJIT_NOTHROW { return _type; }

  // [Members]

protected:
  //! @brief Compiler where this emittable is connected to.
  Compiler* _compiler;
  //! @brief Previous emittable.
  Emittable* _prev;
  //! @brief Next emittable.
  Emittable* _next;
  //! @brief Type of emittable, see @c EMITTABLE_TYPE.
  UInt32 _type;

private:
  friend struct CompilerCore;

  // Disable copy.
  ASMJIT_DISABLE_COPY(Emittable);
};

// ============================================================================
// [AsmJit::Comment]
// ============================================================================

//! @brief Emittable used to emit comment into @c Assembler logger.
//!
//! Comments allows to comment your assembler stream for better debugging
//! and visualization whats happening. Comments are ignored if logger is not
//! set.
//!
//! Comment data can't be modified after comment was created.
struct ASMJIT_API Comment : public Emittable
{
  // [Construction / Destruction]

  Comment(Compiler* c, const char* str) ASMJIT_NOTHROW;
  virtual ~Comment() ASMJIT_NOTHROW;

  // [Emit]

  virtual void emit(Assembler& a);

  // [Methods]

  //! @brief Get comment data.
  inline const char* str() const ASMJIT_NOTHROW { return _str; }

  // [Members]

private:
  const char* _str;
};

// ============================================================================
// [AsmJit::EmbeddedData]
// ============================================================================

//! @brief Emittable used to emit comment into @c Assembler logger.
//! 
//! @note This class is always allocated by @c AsmJit::Compiler.
struct ASMJIT_API EmbeddedData : public Emittable
{
  // [Construction / Destruction]

  EmbeddedData(Compiler* c, SysUInt capacity, const void* data, SysUInt size) ASMJIT_NOTHROW;
  virtual ~EmbeddedData() ASMJIT_NOTHROW;

  // [Emit]

  virtual void emit(Assembler& a);

  // [Methods]

  //! @brief Get size of embedded data.
  SysUInt size() const ASMJIT_NOTHROW { return _size; }
  //! @brief Get capacity of embedded data.
  //! @internal
  SysUInt capacity() const ASMJIT_NOTHROW { return _size; }
  //! @brief Get pointer to embedded data.
  UInt8* data() const ASMJIT_NOTHROW { return (UInt8*)_data; }

  // [Members]

private:
  SysUInt _size;
  SysUInt _capacity;
  UInt8 _data[sizeof(void*)];

  friend struct CompilerCore;
};

// ============================================================================
// [AsmJit::Align]
// ============================================================================

//! @brief Emittable used to align assembler code.
struct ASMJIT_API Align : public Emittable
{
  // [Construction / Destruction]

  Align(Compiler* c, SysInt size = 0) ASMJIT_NOTHROW;
  virtual ~Align() ASMJIT_NOTHROW;

  // [Methods]

  virtual void emit(Assembler& a);

  //! @brief Get align size in bytes.
  inline SysInt size() const ASMJIT_NOTHROW { return _size; }
  //! @brief Set align size in bytes to @a size.
  inline void setSize(SysInt size) ASMJIT_NOTHROW { _size = size; }

private:
  SysInt _size;
};

// ============================================================================
// [AsmJit::Instruction]
// ============================================================================

//! @brief Emittable that represents single instruction and its operands.
struct ASMJIT_API Instruction : public Emittable
{
  // [Construction / Destruction]

  Instruction(Compiler* c) ASMJIT_NOTHROW;
  Instruction(Compiler* c,
    UInt32 code,
    const Operand* o1, const Operand* o2, const Operand* o3,
    const char* inlineComment = NULL) ASMJIT_NOTHROW;
  virtual ~Instruction() ASMJIT_NOTHROW;

  // [Emit]

  virtual void emit(Assembler& a);

  // [Methods]

  //! @brief Return instruction code, see @c INST_CODE.
  inline UInt32 code() const ASMJIT_NOTHROW { return _code; }

  //! @brief Return array of operands (3 operands total).
  inline Operand* const* ops() ASMJIT_NOTHROW { return _o; }

  //! @brief Return first instruction operand.
  inline Operand* o1() const ASMJIT_NOTHROW { return _o[0]; }

  //! @brief Return second instruction operand.
  inline Operand* o2() const ASMJIT_NOTHROW { return _o[1]; }

  //! @brief Return third instruction operand.
  inline Operand* o3() const ASMJIT_NOTHROW { return _o[2]; }

  //! @brief Set instruction code.
  //!
  //! Please do not modify instruction code if you are not know what you are 
  //! doing. Incorrect instruction code or operands can assert() in runtime.
  inline void setCode(UInt32 code) ASMJIT_NOTHROW { _code = code; }

  // [Members]

private:
  //! @brief Instruction code, see @c INST_CODE.
  UInt32 _code;
  //! @brief Instruction operands.
  Operand *_o[3];
  //! @brief Static array for instruction operands (cache)
  Operand _ocache[3];

  const char* _inlineComment;

  friend struct Function;
};

// ============================================================================
// [AsmJit::TypeAsId]
// ============================================================================

//! @brief Template based type to variable ID converter.
template<typename T>
struct TypeAsId 
{
#if defined(ASMJIT_NODOC)
  enum { 
    //! @brief Variable id, see @c VARIABLE_TYPE enum
    Id = X
  };
#endif
};

// Skip documenting this.
#if !defined(ASMJIT_NODOC)

template<typename T>
struct TypeAsId<T*> { enum { Id = VARIABLE_TYPE_PTR }; };

#define ASMJIT_DECLARE_TYPE_AS_ID(__T__, __Id__) \
  template<> \
  struct TypeAsId<__T__> { enum { Id = __Id__ }; }

ASMJIT_DECLARE_TYPE_AS_ID(Int32, VARIABLE_TYPE_INT32);
ASMJIT_DECLARE_TYPE_AS_ID(UInt32, VARIABLE_TYPE_UINT32);

#if defined(ASMJIT_X64)
ASMJIT_DECLARE_TYPE_AS_ID(Int64, VARIABLE_TYPE_INT64);
ASMJIT_DECLARE_TYPE_AS_ID(UInt64, VARIABLE_TYPE_UINT64);
#endif // ASMJIT_X64

ASMJIT_DECLARE_TYPE_AS_ID(float, VARIABLE_TYPE_FLOAT);
ASMJIT_DECLARE_TYPE_AS_ID(double, VARIABLE_TYPE_DOUBLE);

#endif // !ASMJIT_NODOC

// ============================================================================
// [AsmJit::Function Builder]
// ============================================================================

//! @brief Class used to build function without arguments.
struct BuildFunction0
{
  inline const UInt32* args() const ASMJIT_NOTHROW { return NULL; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 0; }
};

//! @brief Class used to build function with 1 argument.
template<typename P0>
struct BuildFunction1
{
  inline const UInt32* args() const ASMJIT_NOTHROW { static const UInt32 data[] = { TypeAsId<P0>::Id }; return data; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 1; }
};

//! @brief Class used to build function with 2 arguments.
template<typename P0, typename P1>
struct BuildFunction2
{
  inline const UInt32* args() const ASMJIT_NOTHROW { static const UInt32 data[] = { TypeAsId<P0>::Id, TypeAsId<P1>::Id }; return data; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 2; }
};

//! @brief Class used to build function with 3 arguments.
template<typename P0, typename P1, typename P2>
struct BuildFunction3
{
  inline const UInt32* args() const ASMJIT_NOTHROW { static const UInt32 data[] = { TypeAsId<P0>::Id, TypeAsId<P1>::Id, TypeAsId<P2>::Id }; return data; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 3; }
};

//! @brief Class used to build function with 4 arguments.
template<typename P0, typename P1, typename P2, typename P3>
struct BuildFunction4
{
  inline const UInt32* args() const ASMJIT_NOTHROW { static const UInt32 data[] = { TypeAsId<P0>::Id, TypeAsId<P1>::Id, TypeAsId<P2>::Id, TypeAsId<P3>::Id }; return data; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 4; }
};

//! @brief Class used to build function with 5 arguments.
template<typename P0, typename P1, typename P2, typename P3, typename P4>
struct BuildFunction5
{
  inline const UInt32* args() const ASMJIT_NOTHROW { static const UInt32 data[] = { TypeAsId<P0>::Id, TypeAsId<P1>::Id, TypeAsId<P2>::Id, TypeAsId<P3>::Id, TypeAsId<P4>::Id }; return data; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 5; }
};

//! @brief Class used to build function with 6 arguments.
template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
struct BuildFunction6
{
  inline const UInt32* args() const ASMJIT_NOTHROW { static const UInt32 data[] = { TypeAsId<P0>::Id, TypeAsId<P1>::Id, TypeAsId<P2>::Id, TypeAsId<P3>::Id, TypeAsId<P4>::Id, TypeAsId<P5>::Id }; return data; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 6; }
};

//! @brief Class used to build function with 7 arguments.
template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct BuildFunction7
{
  inline const UInt32* args() const ASMJIT_NOTHROW { static const UInt32 data[] = { TypeAsId<P0>::Id, TypeAsId<P1>::Id, TypeAsId<P2>::Id, TypeAsId<P3>::Id, TypeAsId<P4>::Id, TypeAsId<P5>::Id, TypeAsId<P6>::Id }; return data; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 7; }
};

//! @brief Class used to build function with 8 arguments.
template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct BuildFunction8
{
  inline const UInt32* args() const ASMJIT_NOTHROW { static const UInt32 data[] = { TypeAsId<P0>::Id, TypeAsId<P1>::Id, TypeAsId<P2>::Id, TypeAsId<P3>::Id, TypeAsId<P4>::Id, TypeAsId<P5>::Id, TypeAsId<P6>::Id, TypeAsId<P7>::Id }; return data; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 8; }
};

//! @brief Class used to build function with 9 arguments.
template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
struct BuildFunction9
{
  inline const UInt32* args() const ASMJIT_NOTHROW { static const UInt32 data[] = { TypeAsId<P0>::Id, TypeAsId<P1>::Id, TypeAsId<P2>::Id, TypeAsId<P3>::Id, TypeAsId<P4>::Id, TypeAsId<P5>::Id, TypeAsId<P6>::Id, TypeAsId<P7>::Id, TypeAsId<P8>::Id }; return data; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 9; }
};

//! @brief Class used to build function with 9 arguments.
template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
struct BuildFunction10
{
  inline const UInt32* args() const ASMJIT_NOTHROW { static const UInt32 data[] = { TypeAsId<P0>::Id, TypeAsId<P1>::Id, TypeAsId<P2>::Id, TypeAsId<P3>::Id, TypeAsId<P4>::Id, TypeAsId<P5>::Id, TypeAsId<P6>::Id, TypeAsId<P7>::Id, TypeAsId<P8>::Id, TypeAsId<P9>::Id }; return data; }
  inline SysUInt count() const ASMJIT_NOTHROW { return 10; }
};

// ============================================================================
// [AsmJit::Function]
// ============================================================================

//! @brief Function emittable used to generate C/C++ functions.
//!
//! Functions are base blocks for generating assembler output. Each generated
//! assembler stream needs standard entry and leave sequences thats compatible 
//! to operating system conventions (ABI).
//!
//! Function class can be used to generate entry (prolog) and leave (epilog)
//! sequences that is compatible to a given calling convention and to allocate
//! and manage variables that can be allocated to registers or spilled.
//!
//! @note To create function use @c AsmJit::Compiler::newFunction() method, do
//! not create @c Function instances by different ways.
//!
//! @sa @c State, @c StateRef, @c Variable, @c VariableRef.
struct ASMJIT_API Function : public Emittable
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create new @c Function instance.
  //!
  //! @note Always use @c AsmJit::Compiler::newFunction() to create @c Function
  //! instance.
  Function(Compiler* c) ASMJIT_NOTHROW;
  //! @brief Destroy @c Function instance.
  virtual ~Function() ASMJIT_NOTHROW;

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  virtual void prepare();
  virtual void emit(Assembler& a);

  // --------------------------------------------------------------------------
  // [Calling Convention / Function Arguments]
  // --------------------------------------------------------------------------

  //! @brief Set function prototype.
  //!
  //! This will set function calling convention and setup arguments variables.
  void setPrototype(UInt32 cconv, const UInt32* args, SysUInt count);

  //! @brief Set naked function to true or false (naked means no prolog / epilog code).
  void setNaked(UInt8 naked) ASMJIT_NOTHROW;

  //! @brief Enable or disable allocation of EBP/RBP register.
  inline void setAllocableEbp(UInt8 val) ASMJIT_NOTHROW { _allocableEbp = val; }

  //! @brief Enable or disable allocation of EBP/RBP register.
  inline void setPrologEpilogPushPop(UInt8 val) ASMJIT_NOTHROW { _prologEpilogPushPop = val; }

  //! @brief Enable or disable emms instruction in epilog.
  inline void setEmms(UInt8 val) ASMJIT_NOTHROW { _emms = val; }

  //! @brief Enable or disable sfence instruction in epilog.
  inline void setSfence(UInt8 val) ASMJIT_NOTHROW { _sfence = val; }

  //! @brief Enable or disable lfence instruction in epilog.
  inline void setLfence(UInt8 val) ASMJIT_NOTHROW { _lfence = val; }

  //! @brief Return whether optimizing prolog / epilog is enabled or disabled.
  inline void setOptimizedPrologEpilog(UInt8 val) ASMJIT_NOTHROW { _optimizedPrologEpilog = val; }

  //! @brief Return function calling convention, see @c CALL_CONV.
  inline UInt32 cconv() const ASMJIT_NOTHROW { return _cconv; }

  //! @brief Return @c true if callee pops the stack by ret() instruction.
  //!
  //! Stdcall calling convention is designed to pop the stack by callee,
  //! but all calling conventions used in MSVC extept cdecl does that.
  //!
  //! @note This is related to used calling convention, it's not affected by
  //! number of function arguments or their types.
  inline UInt8 calleePopsStack() const ASMJIT_NOTHROW { return _calleePopsStack; }

  //! @brief Return @c true if function is naked (no prolog / epilog code).
  inline UInt8 naked() const ASMJIT_NOTHROW { return _naked; }

  //! @brief Return @c true if EBP/RBP register can be allocated by register allocator.
  inline UInt8 allocableEbp() const ASMJIT_NOTHROW { return _allocableEbp; }

  //! @brief Return @c true if EBP/RBP register can be allocated by register allocator.
  inline UInt8 prologEpilogPushPop() const ASMJIT_NOTHROW { return _prologEpilogPushPop; }

  //! @brief Return whether emms instruction is enabled or disabled in epilog.
  inline UInt8 emms() const ASMJIT_NOTHROW { return _emms; }

  //! @brief Return whether sfence instruction is enabled or disabled in epilog.
  inline UInt8 sfence() const ASMJIT_NOTHROW { return _sfence; }

  //! @brief Return whether lfence instruction is enabled or disabled in epilog.
  inline UInt8 lfence() const ASMJIT_NOTHROW { return _lfence; }

  //! @brief Return whether optimizing prolog / epilog is enabled or disabled.
  inline UInt8 optimizedPrologEpilog() const ASMJIT_NOTHROW { return _optimizedPrologEpilog; }

  //! @brief Return direction of arguments passed on the stack.
  //!
  //! Direction should be always @c ARGUMENT_DIR_RIGHT_TO_LEFT.
  //!
  //! @note This is related to used calling convention, it's not affected by
  //! number of function arguments or their types.
  inline UInt32 cconvArgumentsDirection() const ASMJIT_NOTHROW { return _cconvArgumentsDirection; }

  //! @brief Return registers used to pass first integer parameters by current
  //! calling convention.
  //!
  //! @note This is related to used calling convention, it's not affected by
  //! number of function arguments or their types.
  inline const UInt32* cconvArgumentsGp() const ASMJIT_NOTHROW { return _cconvArgumentsGp; }

  //! @brief Return registers used to pass first SP-FP or DP-FPparameters by 
  //! current calling convention.
  //!
  //! @note This is related to used calling convention, it's not affected by
  //! number of function arguments or their types.
  inline const UInt32* cconvArgumentsXmm() const ASMJIT_NOTHROW { return _cconvArgumentsXmm; }

  //! @brief Return bitmask of general purpose registers that's preserved 
  //! (non-volatile).
  //!
  //! @note This is related to used calling convention, it's not affected by
  //! number of function arguments or their types.
  inline UInt32 cconvPreservedGp() const ASMJIT_NOTHROW { return _cconvPreservedGp; }
  //! @brief Return bitmask of sse registers that's preserved (non-volatile).
  //!
  //! @note This is related to used calling convention, it's not affected by
  //! number of function arguments or their types.
  inline UInt32 cconvPreservedXmm() const ASMJIT_NOTHROW { return _cconvPreservedXmm; }

  // --------------------------------------------------------------------------
  // [Registers allocator / Variables]
  // --------------------------------------------------------------------------

  //! @brief Return argument at @a i.
  inline Variable* argument(SysInt i) ASMJIT_NOTHROW
  {
    ASMJIT_ASSERT((SysUInt)i < _argumentsCount);
    return _variables[i];
  }

  //! @brief Create new variable
  Variable* newVariable(UInt8 type, UInt8 priority = 10, UInt8 preferredRegister = NO_REG);

  bool alloc(Variable* v,
    UInt8 mode = VARIABLE_ALLOC_READWRITE, 
    UInt8 preferredRegister = NO_REG);
  bool spill(Variable* v);
  void unuse(Variable* v);

  void spillAll();
  void spillAllGp();
  void spillAllMm();
  void spillAllXmm();
  void _spillAll(SysUInt start, SysUInt end);
  void spillRegister(const BaseReg& reg);

  //! @brief Get count of free GP registers (in current state).
  SysInt numFreeGp() const;
  //! @brief Get count of free MMX registers (in current state).
  SysInt numFreeMm() const;
  //! @brief Get count of free XMM registers (in current state).
  SysInt numFreeXmm() const;

  //! @brief Get if variable @a v is prevented.
  //! @return true if variable @a v is prevented (in _prevented list).
  //! @sa _prevented.
  bool isPrevented(Variable* v);

  //! @brief Add variable @a v to list of prevented variables.
  //! @sa _prevented.
  void addPrevented(Variable* v);

  //! @brief Remove variable @a v from list of prevented variables.
  //! @sa _prevented.
  void removePrevented(Variable* v);

  //! @brief Remove all variables from list of prevented ones.
  //! @sa _prevented.
  void clearPrevented();

  //! @brief Return spill candidate for variable type @a type.
  Variable* _getSpillCandidate(UInt8 type);

  void _allocAs(Variable* v, UInt8 mode, UInt32 code);
  void _allocReg(UInt8 code, Variable* v);
  void _freeReg(UInt8 code);

  void _moveGp(Variable* v, UInt8 code);
  void _exchangeGp(Variable* v, UInt8 mode, Variable* other);
  void _postAlloc(Variable* v, UInt8 mode);

  //! @brief Return size of alignment on the stack.
  //!
  //! Stack is aligned to 16 bytes by default. For X64 platforms there is 
  //! no extra code needed to align stack to 16 bytes, because it's default
  //! stack alignment.
  inline SysInt stackAlignmentSize() const ASMJIT_NOTHROW { return _stackAlignmentSize; }

  //! @brief Size needed to save registers in prolog / epilog.
  inline SysInt prologEpilogStackSize() const ASMJIT_NOTHROW { return _prologEpilogStackSize; }

  //! @brief Return size of variables on the stack.
  //!
  //! This variable is always aligned to 16 bytes for each platform.
  inline SysInt variablesStackSize() const ASMJIT_NOTHROW { return _variablesStackSize; }

  //! @brief Return count of arguments.
  inline UInt32 argumentsCount() const ASMJIT_NOTHROW { return _argumentsCount; }

  //! @brief Return stack size of all function arguments (passed on the 
  //! stack).
  inline UInt32 argumentsStackSize() const ASMJIT_NOTHROW { return _argumentsStackSize; }

  //! @brief Return bitmask of all used (for actual context) general purpose registers.
  inline UInt32 usedGpRegisters() const ASMJIT_NOTHROW { return _usedGpRegisters; }
  //! @brief Return bitmask of all used (for actual context) mmx registers.
  inline UInt32 usedMmRegisters() const ASMJIT_NOTHROW { return _usedMmRegisters; }
  //! @brief Return bitmask of all used (for actual context) sse registers.
  inline UInt32 usedXmmRegisters() const ASMJIT_NOTHROW { return _usedXmmRegisters; }

  //! @brief Mark general purpose registers in the given @a mask as used.
  inline void useGpRegisters(UInt32 mask) ASMJIT_NOTHROW { _usedGpRegisters |= mask; }
  //! @brief Mark mmx registers in the given @a mask as used.
  inline void useMmRegisters(UInt32 mask) ASMJIT_NOTHROW { _usedMmRegisters |= mask; }
  //! @brief Mark sse registers in the given @a mask as used.
  inline void useXmmRegisters(UInt32 mask) ASMJIT_NOTHROW { _usedXmmRegisters |= mask; }

  //! @brief Mark general purpose registers in the given @a mask as unused.
  inline void unuseGpRegisters(UInt32 mask) ASMJIT_NOTHROW { _usedGpRegisters &= ~mask; }
  //! @brief Mark mmx registers in the given @a mask as unused.
  inline void unuseMmRegisters(UInt32 mask) ASMJIT_NOTHROW { _usedMmRegisters &= ~mask; }
  //! @brief Mark sse registers in the given @a mask as unused.
  inline void unuseXmmRegisters(UInt32 mask) ASMJIT_NOTHROW { _usedXmmRegisters &= ~mask; }

  //! @brief Return bitmask of all changed general purpose registers during
  //! function execution (for generating optimized prolog / epilog).
  inline UInt32 modifiedGpRegisters() const ASMJIT_NOTHROW { return _modifiedGpRegisters; }
  //! @brief Return bitmask of all changed mmx registers during
  //! function execution (for generating optimized prolog / epilog).
  inline UInt32 modifiedMmRegisters() const ASMJIT_NOTHROW { return _modifiedMmRegisters; }
  //! @brief Return bitmask of all changed sse registers during
  //! function execution (for generating optimized prolog / epilog).
  inline UInt32 modifiedXmmRegisters() const ASMJIT_NOTHROW { return _modifiedXmmRegisters; }

  //! @brief Mark general purpose registers in the given @a mask as modified.
  inline void modifyGpRegisters(UInt32 mask) ASMJIT_NOTHROW { _modifiedGpRegisters |= mask; }
  //! @brief Mark mmx registers in the given @a mask as modified.
  inline void modifyMmRegisters(UInt32 mask) ASMJIT_NOTHROW { _modifiedMmRegisters |= mask; }
  //! @brief Mark sse registers in the given @a mask as modified.
  inline void modifyXmmRegisters(UInt32 mask) ASMJIT_NOTHROW { _modifiedXmmRegisters |= mask; }

  //! @brief Get count of GP registers that must be saved by prolog and restored
  //! by epilog.
  SysInt countOfGpRegistersToBeSaved() const ASMJIT_NOTHROW;
  //! @brief Get count of XMM registers that must be saved by prolog and restored
  //! by epilog.
  SysInt countOfXmmRegistersToBeSaved() const ASMJIT_NOTHROW;

  // --------------------------------------------------------------------------
  // [State]
  // --------------------------------------------------------------------------

  //! @brief Save function current register state.
  //!
  //! To save function state always wrap returned value into @c StateRef:
  //!
  //! @code
  //! // Your function
  //! Function &f = ...;
  //!
  //! // Block
  //! {
  //!   // Save state
  //!   StateRef state(f.saveState());
  //!
  //!   // Your code ...
  //!
  //!   // Restore state (automatic by @c StateRef destructor).
  //! }
  //!
  //! @endcode
  State* saveState();

  //! @brief Restore function register state to @a state.
  //! @sa saveState().
  void restoreState(State* state);

  //! @brief Set function register state to @a state.
  void setState(State* state);

  // --------------------------------------------------------------------------
  // [Labels]
  // --------------------------------------------------------------------------

  //! @brief Return function entry label.
  //!
  //! Entry label can be used to call this function from another code that's
  //! being generated.
  inline Label* entryLabel() const ASMJIT_NOTHROW { return _entryLabel; }

  //! @brief Return prolog label (label after function prolog)
  inline Label* prologLabel() const ASMJIT_NOTHROW { return _prologLabel; }

  //! @brief Return exit label.
  //!
  //! Use exit label to jump to function epilog.
  inline Label* exitLabel() const ASMJIT_NOTHROW { return _exitLabel; }

private:
  // --------------------------------------------------------------------------
  // [Calling Convention / Function Arguments]
  // --------------------------------------------------------------------------

  //! @brief Sets function calling convention.
  void _setCallingConvention(UInt32 cconv) ASMJIT_NOTHROW;

  //! @brief Sets function arguments (must be done after correct calling 
  //! convention is set).
  void _setArguments(const UInt32* args, SysUInt len);

  //! @brief Internal, used from other _jmpAndRestore method. This method does
  //! the main job.
  static void _jmpAndRestore(Compiler* c, Label* label);

  //! @brief Calling convention, see @c CALL_CONV.
  UInt32 _cconv;

  //! @brief Callee pops stack;
  UInt8 _calleePopsStack;

  //! @brief Generate naked function?
  UInt8 _naked;

  //! @brief Whether EBP/RBP register can be used by register allocator.
  UInt8 _allocableEbp;

  //! @brief Whether Prolog and epilog should be generated by push/pop
  //! instructions instead of mov instructions.
  UInt8 _prologEpilogPushPop;

  //! @brief Whether to generate emms instruction in epilog.
  UInt8 _emms;

  //! @brief Whether to generate sfence instruction in epilog.
  UInt8 _sfence;

  //! @brief Whether to generate lfence instruction in epilog.
  UInt8 _lfence;

  //! @brief Whether to optimize prolog / epilog sequences.
  UInt8 _optimizedPrologEpilog;

  //! @brief Direction for arguments passed on stack, see @c ARGUMENT_DIR.
  UInt32 _cconvArgumentsDirection;

  //! @brief List of registers that's used for first INT arguments instead of stack.
  UInt32 _cconvArgumentsGp[16];
  //! @brief List of registers that's used for first FP arguments instead of stack.
  UInt32 _cconvArgumentsXmm[16];

  //! @brief Bitmask for preserved general purpose registers.
  UInt32 _cconvPreservedGp;
  //! @brief Bitmask for preserved sse registers.
  UInt32 _cconvPreservedXmm;

  //! @brief Count of arguments (in @c _argumentsList).
  UInt32 _argumentsCount;

  //! @brief Count of bytes consumed by arguments on the stack.
  UInt32 _argumentsStackSize;

  // --------------------------------------------------------------------------
  // [Registers allocator / Variables]
  // --------------------------------------------------------------------------

  //! @brief Size of maximum alignment size on the stack.
  SysInt _stackAlignmentSize;
  //! @brief Size of prolog/epilog on the stack.
  SysInt _prologEpilogStackSize;
  //! @brief Size of all variables on the stack.
  SysInt _variablesStackSize;

  //! @brief Bitmask where are stored are used GP registers.
  UInt32 _usedGpRegisters;
  //! @brief Bitmask where are stored are used MMX registers.
  UInt32 _usedMmRegisters;
  //! @brief Bitmask where are stored are used XMM registers.
  UInt32 _usedXmmRegisters;

  //! @brief Bitmask where are stored are modified GP registers.
  UInt32 _modifiedGpRegisters;
  //! @brief Bitmask where are stored are modified MMX registers.
  UInt32 _modifiedMmRegisters;
  //! @brief Bitmask where are stored are modified XMM registers.
  UInt32 _modifiedXmmRegisters;

  //! @brief List of variables managed by Function/Compiler.
  PodVector<Variable*> _variables;

  //! @brief List of prevented variables that can't be spilled.
  //!
  //! Prevented variables are variables used when generating code chain. First
  //! the variables are allocated/spilled and then the assembler instruction
  //! is emitted.
  //!
  //! For example look at this simple code:
  //!
  //! @code
  //! Compiler::mov(dst_variable.x(), src_variable.c());
  //! @endcode
  //!
  //! Before mov instruction is emitted, variables dst_variable and src_variable
  //! are stored in @c Compiler::_prevented list. Prevention prevents variables
  //! to be spilled (if some variable needs to be allocated and compiler must
  //! decide which variable to spill).
  PodVector<Variable*> _prevented;

  //! @brief Whether to use registers prevention. This is internally turned 
  //! on / off while switching between states.
  //!
  //! Never modify this varialbe unless you know what you are doing. Prevention
  //! is Compiler/Function implementation detail.
  bool _usePrevention;

  // --------------------------------------------------------------------------
  // [State]
  // --------------------------------------------------------------------------

  //! @brief This is similar to State::Data, but we need to save only allocated 
  //! variables (this is enough). First idea was to use State here, but source
  //! code was bloated by it (setting and restoring values that weren't needed)
  union StateData
  {
    //! @brief All variables in one array.
    Variable* regs[16+8+16];

    struct {
      //! @brief Regeral purpose registers.
      Variable* gp[16];
      //! @brief MMX registers.
      Variable* mm[8];
      //! @brief XMM registers.
      Variable* xmm[16];
    };
  };

  //! @brief Current state data.
  StateData _state;

  // --------------------------------------------------------------------------
  // [Labels]
  // --------------------------------------------------------------------------

  //! @brief Function entry point label.
  Label* _entryLabel;
  //! @brief Label that points to start of function prolog generated by @c Prolog.
  Label* _prologLabel;
  //! @brief Label that points before function epilog generated by @c Epilog.
  Label* _exitLabel;

  friend struct CompilerCore;
  friend struct State;
};

// Inlines that uses AsmJit::Function
inline bool Variable::alloc(UInt8 mode, UInt8 preferredRegister) 
{ return function()->alloc(this, mode, preferredRegister); }

inline bool Variable::spill()
{ return function()->spill(this); }

inline void Variable::unuse()
{ function()->unuse(this); }

inline StateRef::~StateRef()
{ if (_state) _state->_function->restoreState(_state); }

// ============================================================================
// [AsmJit::Prolog]
// ============================================================================

//! @brief Prolog emittable.
struct ASMJIT_API Prolog : public Emittable
{
  // [Construction / Destruction]

  Prolog(Compiler* c, Function* f) ASMJIT_NOTHROW;
  virtual ~Prolog() ASMJIT_NOTHROW;

  // [Emit]

  virtual void emit(Assembler& a);

  // [Methods]

  //! @brief Get function associated with this prolog.
  inline Function* function() const ASMJIT_NOTHROW { return _function; }

  // [Members]

private:
  Function* _function;
  Label* _label;

  friend struct CompilerCore;
  friend struct Function;
};

// ============================================================================
// [AsmJit::Epilog]
// ============================================================================

//! @brief Epilog emittable.
struct ASMJIT_API Epilog : public Emittable
{
  // [Construction / Destruction]

  Epilog(Compiler* c, Function* f) ASMJIT_NOTHROW;
  virtual ~Epilog() ASMJIT_NOTHROW;

  // [Emit]

  virtual void emit(Assembler& a);

  // [Methods]

  //! @brief Get function associated with this epilog.
  inline Function* function() const ASMJIT_NOTHROW { return _function; }

  // [Members]

private:
  Function* _function;
  Label* _label;

  friend struct CompilerCore;
  friend struct Function;
};

// ============================================================================
// [AsmJit::Target]
// ============================================================================

//! @brief Target.
//!
//! Target is bound label location.
struct ASMJIT_API Target : public Emittable
{
  // [Construction / Destruction]

  Target(Compiler* c, Label* target) ASMJIT_NOTHROW;
  virtual ~Target() ASMJIT_NOTHROW;

  // [Emit]

  virtual void emit(Assembler& a);

  // [Methods]

  //! @brief Return label bound to this target.
  inline Label* target() const ASMJIT_NOTHROW { return _target; }

  // [Members]

private:
  Label* _target;
};

// ============================================================================
// [AsmJit::JumpTable]
// ============================================================================

//! @brief Jump table.
struct ASMJIT_API JumpTable : public Emittable
{
  // [Construction / Destruction]

  JumpTable(Compiler* c) ASMJIT_NOTHROW;
  virtual ~JumpTable() ASMJIT_NOTHROW;

  // [Emit]

  virtual void emit(Assembler& a);
  virtual void postEmit(Assembler& a);

  // [Methods]

  //! @brief Return target label where are informations about jump adresses.
  inline Label* target() const ASMJIT_NOTHROW { return _target; }

  //! @brief Return labels list.
  PodVector<Label*>& labels() ASMJIT_NOTHROW { return _labels; }

  //! @brief Return labels list.
  const PodVector<Label*>& labels() const ASMJIT_NOTHROW { return _labels; }

  //! @brief Add new label @a target to jump table.
  //! @param target @c Label to add (or NULL to create one).
  //! @param pos Position in jump table where to add it
  Label* addLabel(Label* target = NULL, SysInt pos = -1);

  // [Members]

private:
  Label* _target;
  PodVector<Label*> _labels;
};

// ============================================================================
// [AsmJit::CompilerCore]
// ============================================================================

//! @brief Compiler core.
//!
//! @sa @c AsmJit::Compiler.
struct ASMJIT_API CompilerCore : public Serializer
{
  // -------------------------------------------------------------------------
  // [Typedefs]
  // -------------------------------------------------------------------------

  //! @brief List of variables used and managed by @c Compiler.
  typedef PodVector<Variable*> VariableList;
  //! @brief List of operands used and managed by @c Compiler.
  typedef PodVector<Operand*> OperandList;

  // -------------------------------------------------------------------------
  // [Construction / Destruction]
  // -------------------------------------------------------------------------

  //! @brief Create new (empty) instance of @c Compiler.
  CompilerCore() ASMJIT_NOTHROW;
  //! @brief Destroy @c Compiler instance.
  virtual ~CompilerCore() ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Compiler]
  // -------------------------------------------------------------------------

  //! @brief Clear everything, but not deallocate buffers.
  //!
  //! @note This method will destroy your code.
  void clear() ASMJIT_NOTHROW;

  //! @brief Free internal buffer, all emitters and NULL all pointers.
  //!
  //! @note This method will destroy your code.
  void free() ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Emittables]
  // -------------------------------------------------------------------------

  //! @brief Return first emittables in double linked list.
  inline Emittable* firstEmittable() const ASMJIT_NOTHROW { return _first; }

  //! @brief Return last emittable in double linked list.
  inline Emittable* lastEmittable() const ASMJIT_NOTHROW { return _last; }

  //! @brief Return current emittable after all emittables are emitter.
  //!
  //! @note If this method return @c NULL it means first position.
  inline Emittable* currentEmittable() const ASMJIT_NOTHROW { return _current; }

  //! @brief Add emittable after current and set current to @a emittable.
  void addEmittable(Emittable* emittable) ASMJIT_NOTHROW;

  //! @brief Remove emittable (and if needed set current to previous).
  void removeEmittable(Emittable* emittable) ASMJIT_NOTHROW;

  //! @brief Set new current emittable and return previous one.
  Emittable* setCurrentEmittable(Emittable* current) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Logging]
  // -------------------------------------------------------------------------

  //! @brief Emit a single comment line into @c Assembler logger.
  //!
  //! Emitting comments are useful to log something. Because assembler can be
  //! generated from AST or other data structures, you may sometimes need to
  //! log data characteristics or statistics.
  //!
  //! @note Emitting comment is not directly sent to logger, but instead it's
  //! stored in @c AsmJit::Compiler and emitted when @c serialize() method is
  //! called with all instructions together in correct order.
  void comment(const char* fmt, ...) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Function Builder]
  // -------------------------------------------------------------------------

  //! @brief Create a new function.
  //!
  //! @param cconv Calling convention to use (see @c CALL_CONV enum)
  //! @param params Function arguments prototype.
  //!
  //! This method is usually used as a first step when generating functions
  //! by @c Compiler. First parameter @a cconv specifies function calling
  //! convention to use. Second parameter @a params specifies function
  //! arguments. To create function arguments are used templates 
  //! @c BuildFunction0<>, @c BuildFunction1<...>, @c BuildFunction2<...>, 
  //! etc...
  //!
  //! Templates with BuildFunction prefix are used to generate argument IDs
  //! based on real C++ types. See next example how to generate function with
  //! two 32 bit integer arguments.
  //!
  //! @code
  //! // Building function using AsmJit::Compiler example.
  //!
  //! // Compiler instance
  //! Compiler c;
  //!
  //! // Begin of function (also emits function @c Prolog)
  //! Function& f = *c.newFunction(
  //!   // Default calling convention (32 bit cdecl or 64 bit for host OS)
  //!   CALL_CONV_DEFAULT,
  //!   // Using function builder to generate arguments list
  //!   BuildFunction2<int, int>());
  //!
  //! // End of function (also emits function @c Epilog)
  //! c.endFunction();
  //! @endcode
  //!
  //! You can see that building functions is really easy. Previous code snipped
  //! will generate code for function with two 32 bit integer arguments. You 
  //! can access arguments by @c AsmJit::Function::argument() method. Arguments
  //! are indexed from 0 (like everything in C).
  //!
  //! @code
  //! // Accessing function arguments through AsmJit::Function example.
  //!
  //! // Compiler instance
  //! Compiler c;
  //!
  //! // Begin of function (also emits function @c Prolog)
  //! Function& f = *c.newFunction(
  //!   // Default calling convention (32 bit cdecl or 64 bit for host OS)
  //!   CALL_CONV_DEFAULT,
  //!   // Using function builder to generate arguments list
  //!   BuildFunction2<int, int>());
  //!
  //! // Arguments are like other variables, you need to reference them by
  //! // VariableRef types:
  //! Int32Ref a0 = f.argument(0);
  //! Int32Ref a1 = f.argument(1);
  //!
  //! // To allocate them to registers just use .alloc(), .r(), .x() or .c() 
  //! // variable methods:
  //! c.add(a0.r(), a1.r());
  //!
  //! // End of function (also emits function @c Epilog)
  //! c.endFunction();
  //! @endcode
  //!
  //! Arguments are like variables. How to manipulate with variables is
  //! documented in @c AsmJit::Compiler detail and @c AsmJit::VariableRef 
  //! class.
  //!
  //! @note To get current function use @c currentFunction() method or save
  //! pointer to @c AsmJit::Function returned by @c AsmJit::Compiler::newFunction<>
  //! method. Recommended is to save the pointer.
  //!
  //! @sa @c BuildFunction0, @c BuildFunction1, @c BuildFunction2, ...
  template<typename T>
  Function* newFunction(UInt32 cconv, const T& params) ASMJIT_NOTHROW
  { return newFunction_(cconv, params.args(), params.count()); }

  //! @brief Create a new function (low level version).
  //!
  //! @param cconv Function calling convention (see @c AsmJit::CALL_CONV).
  //! @param args Function arguments (see @c AsmJit::VARIABLE_TYPE).
  //! @param count Arguments count.
  //!
  //! This method is internally called from @c newFunction() method and 
  //! contains arguments thats used internally by @c AsmJit::Compiler.
  //!
  //! @note To get current function use @c currentFunction() method.
  Function* newFunction_(UInt32 cconv, const UInt32* args, SysUInt count) ASMJIT_NOTHROW;

  //! @brief Ends current function.
  Function* endFunction() ASMJIT_NOTHROW;

  //! @brief Return current function.
  //!
  //! This method can be called within @c newFunction() and @c endFunction()
  //! block to get current function you are working with. It's recommended
  //! to store @c AsmJit::Function pointer returned by @c newFunction<> method,
  //! because this allows you in future implement function sections outside of
  //! function itself (yeah, this is possible!).
  inline Function* currentFunction() const ASMJIT_NOTHROW { return _currentFunction; }

  //! @brief Create function prolog (function begin section).
  //!
  //! Function prologs and epilogs are standardized sequences of instructions
  //! thats used to build functions. If you are using @c Function and 
  //! @c AsmJit::Compiler::newFunction() to make a function, keep in mind that
  //! it creates prolog (by @c newFunction()) and epilog (by @c endFunction())
  //! for you.
  //!
  //! @note Never use prolog after @c newFunction() method. It will create
  //! prolog for you!
  //!
  //! @note Compiler can optimize prologs and epilogs.
  //!
  //! @sa @c Prolog, @c Function.
  Prolog* newProlog(Function* f) ASMJIT_NOTHROW;

  //! @brief Create function epilog (function leave section).
  //!
  //! Function prologs and epilogs are standardized sequences of instructions
  //! thats used to build functions. If you are using @c Function and 
  //! @c AsmJit::Compiler::newFunction() to make a function, keep in mind that
  //! it creates prolog (by @c newFunction()) and epilog (by @c endFunction())
  //! for you.
  //!
  //! @note Never use epilog before @c endFunction() method. It will create
  //! epilog for you!
  //!
  //! @note Compiler can optimize prologs and epilogs.
  //!
  //! @sa @c Epilog, @c Function.
  Epilog* newEpilog(Function* f) ASMJIT_NOTHROW;

  // --------------------------------------------------------------------------
  // [Registers allocator / Variables]
  // --------------------------------------------------------------------------

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->argument()
  //! @sa @c Function::argument()
  Variable* argument(SysInt i) ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->newVariable()
  //! @sa @c Function::newVariable()
  Variable* newVariable(UInt8 type, UInt8 priority = 10, UInt8 preferredRegister = NO_REG) ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->alloc()
  //! @sa @c Function::alloc()
  bool alloc(Variable* v,
    UInt8 mode = VARIABLE_ALLOC_READWRITE,
    UInt8 preferredRegister = NO_REG) ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->spill()
  //! @sa @c Function::spill()
  bool spill(Variable* v) ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->unuse()
  //! @sa @c Function::unuse()
  void unuse(Variable* v) ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->spillAll()
  //! @sa @c Function::spillAll()
  void spillAll() ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->spillAllGp()
  //! @sa @c Function::spillAllGp()
  void spillAllGp() ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->spillAllMm()
  //! @sa @c Function::spillAllMm()
  void spillAllMm() ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->spillAllXmm()
  //! @sa @c Function::spillAllXmm()
  void spillAllXmm() ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->spillRegister()
  //! @sa @c Function::spillRegister()
  void spillRegister(const BaseReg& reg) ASMJIT_NOTHROW;

  SysInt numFreeGp() const ASMJIT_NOTHROW;
  SysInt numFreeMm() const ASMJIT_NOTHROW;
  SysInt numFreeXmm() const ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->isPrevented()
  //! @sa @c Function::isPrevented()
  bool isPrevented(Variable* v) ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->addPrevented()
  //! @sa @c Function::addPrevented()
  void addPrevented(Variable* v) ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->removePrevented()
  //! @sa @c Function::removePrevented()
  void removePrevented(Variable* v) ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->clearPrevented()
  //! @sa @c Function::clearPrevented()
  void clearPrevented() ASMJIT_NOTHROW;

  // --------------------------------------------------------------------------
  // [State]
  // --------------------------------------------------------------------------

  //! @brief Convenience method that calls:
  //!   Compiler::currentFunction()->saveState()
  //! @sa @c Function::saveState().
  State* saveState() ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFuncion()->restoreState()
  //! @sa @c Function::restoreState().
  void restoreState(State* state) ASMJIT_NOTHROW;

  //! @brief Convenience method that calls:
  //!   Compiler::currentFuncion()->setState()
  //! @sa @c Function::setState()
  void setState(State* state) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Labels]
  // -------------------------------------------------------------------------

  //! @brief Create and return new @a Label managed by compiler.
  //!
  //! Labels created by compiler are same objects as labels created for 
  //! @c Assembler. There is only one limitation that if you are using 
  //! @c Compiler each label must be created by @c AsmJit::Compiler::newLabel()
  //! method.
  Label* newLabel() ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Jump Table]
  // -------------------------------------------------------------------------

  JumpTable* newJumpTable() ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Memory Management]
  // -------------------------------------------------------------------------

  //! @brief Create object managed by compiler internal memory manager.
  template<typename T>
  inline T* newObject() ASMJIT_NOTHROW
  {
    void* addr = _zoneAlloc(sizeof(T));
    return new(addr) T(reinterpret_cast<Compiler*>(this));
  }

  //! @brief Create object managed by compiler internal memory manager.
  template<typename T, typename P1>
  inline T* newObject(P1 p1) ASMJIT_NOTHROW
  {
    void* addr = _zoneAlloc(sizeof(T));
    return new(addr) T(reinterpret_cast<Compiler*>(this), p1);
  }

  //! @brief Create object managed by compiler internal memory manager.
  template<typename T, typename P1, typename P2>
  inline T* newObject(P1 p1, P2 p2) ASMJIT_NOTHROW
  {
    void* addr = _zoneAlloc(sizeof(T));
    return new(addr) T(reinterpret_cast<Compiler*>(this), p1, p2);
  }

  //! @brief Create object managed by compiler internal memory manager.
  template<typename T, typename P1, typename P2, typename P3>
  inline T* newObject(P1 p1, P2 p2, P3 p3) ASMJIT_NOTHROW
  {
    void* addr = _zoneAlloc(sizeof(T));
    return new(addr) T(reinterpret_cast<Compiler*>(this), p1, p2, p3);
  }

  //! @brief Create object managed by compiler internal memory manager.
  template<typename T, typename P1, typename P2, typename P3, typename P4>
  inline T* newObject(P1 p1, P2 p2, P3 p3, P4 p4) ASMJIT_NOTHROW
  {
    void* addr = _zoneAlloc(sizeof(T));
    return new(addr) T(reinterpret_cast<Compiler*>(this), p1, p2, p3, p4);
  }

  //! @brief Create object managed by compiler internal memory manager.
  template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
  inline T* newObject(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) ASMJIT_NOTHROW
  {
    void* addr = _zoneAlloc(sizeof(T));
    return new(addr) T(reinterpret_cast<Compiler*>(this), p1, p2, p3, p4, p5);
  }

  //! @brief Internal function that registers operand @a op in compiler.
  //!
  //! Operand registration means adding @a op to internal operands list and 
  //! setting operand id.
  //!
  //! @note Operand @a op should by allocated by @c Compiler or you must
  //! guarantee that it will be not destroyed before @c Compiler is destroyed.
  void _registerOperand(Operand* op) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Jumps / Calls]
  // -------------------------------------------------------------------------

  using Serializer::jmp;
  using Serializer::call;

  void jumpToTable(JumpTable* jt, const Register& index) ASMJIT_NOTHROW;

  SysInt _addTarget(void* target) ASMJIT_NOTHROW;

  void _jmpAndRestore(UInt32 code, Label* label, State* state) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Intrinsics]
  // -------------------------------------------------------------------------

  //! @brief Intrinsics helper method.
  //! @internal
  void op_var32(UInt32 code, const Int32Ref& a) ASMJIT_NOTHROW;

  //! @brief Intrinsics helper method.
  //! @internal
  void op_reg32_var32(UInt32 code, const Register& a, const Int32Ref& b) ASMJIT_NOTHROW;

  //! @brief Intrinsics helper method.
  //! @internal
  void op_var32_reg32(UInt32 code, const Int32Ref& a, const Register& b) ASMJIT_NOTHROW;

  //! @brief Intrinsics helper method.
  //! @internal
  void op_var32_imm(UInt32 code, const Int32Ref& a, const Immediate& b) ASMJIT_NOTHROW;

#if defined(ASMJIT_X64)
  //! @brief Intrinsics helper method.
  //! @internal
  void op_var64(UInt32 code, const Int64Ref& a) ASMJIT_NOTHROW;

  //! @brief Intrinsics helper method.
  //! @internal
  void op_reg64_var64(UInt32 code, const Register& a, const Int64Ref& b) ASMJIT_NOTHROW;

  //! @brief Intrinsics helper method.
  //! @internal
  void op_var64_reg64(UInt32 code, const Int64Ref& a, const Register& b) ASMJIT_NOTHROW;

  //! @brief Intrinsics helper method.
  //! @internal
  void op_var64_imm(UInt32 code, const Int64Ref& a, const Immediate& b) ASMJIT_NOTHROW;
#endif // ASMJIT_X64

  // -------------------------------------------------------------------------
  // [EmitX86]
  // -------------------------------------------------------------------------

  virtual void _inlineComment(const char* text, SysInt len = -1) ASMJIT_NOTHROW;
  virtual void _emitX86(UInt32 code, const Operand* o1, const Operand* o2, const Operand* o3) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Embed]
  // -------------------------------------------------------------------------

  virtual void _embed(const void* dataPtr, SysUInt dataSize) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Align]
  // -------------------------------------------------------------------------

  virtual void align(SysInt m) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Bind]
  // -------------------------------------------------------------------------

  virtual void bind(Label* label) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Make]
  // -------------------------------------------------------------------------

  virtual void* make(MemoryManager* memoryManager = NULL, UInt32 allocType = MEMORY_ALLOC_FREEABLE) ASMJIT_NOTHROW;

  //! @brief Method that will emit everything to @c Assembler instance @a a.
  void serialize(Assembler& a) ASMJIT_NOTHROW;

  // -------------------------------------------------------------------------
  // [Variables]
  // -------------------------------------------------------------------------
private:
  //! @brief First emittable.
  Emittable* _first;
  //! @brief Last emittable.
  Emittable* _last;
  //! @brief Current emittable.
  Emittable* _current;

  //! @brief Operands list (operand id is index in this list, id 0 is not valid).
  OperandList _operands;

  //! @brief Current function.
  Function* _currentFunction;

  //! @brief Label id counter (starts from 1).
  UInt32 _labelIdCounter;

  //! @brief Jump table label.
  Label* _jumpTableLabel;

  //! @brief Jump table entities.
  PodVector<void*> _jumpTableData;

  //! @brief Buffer for inline comment (for next instruction).
  const char* _inlineCommentBuffer;

  friend struct Instruction;
  friend struct Variable;
};

// ============================================================================
// [AsmJit::CompilerIntrinsics]
// ============================================================================

//! @brief Implementation of @c Compiler intrinsics.
//!
//! Methods in this class are implemented here, because we wan't to hide them
//! in shared libraries. These methods should be never exported by C++ compiler.
//!
//! @sa @c AsmJit::Compiler.
struct ASMJIT_HIDDEN CompilerIntrinsics : public CompilerCore
{
  //! @brief Create @c CompilerIntrinsics instance. Always use @c AsmJit::Compiler.
  inline CompilerIntrinsics() ASMJIT_NOTHROW {}

  // --------------------------------------------------------------------------
  // [jmpAndRestore]
  // --------------------------------------------------------------------------

  inline void jAndRestore(CONDITION cc, Label* label, State* state)
  {
    ASMJIT_ASSERT(static_cast<UInt32>(cc) <= 0xF);
    _jmpAndRestore(_jcctable[cc], label, state);
  }

  inline void jaAndRestore  (Label* label, State* state) { _jmpAndRestore(INST_JA  , label, state); }
  inline void jaeAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JAE , label, state); }
  inline void jbAndRestore  (Label* label, State* state) { _jmpAndRestore(INST_JB  , label, state); }
  inline void jbeAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JBE , label, state); }
  inline void jcAndRestore  (Label* label, State* state) { _jmpAndRestore(INST_JC  , label, state); }
  inline void jeAndRestore  (Label* label, State* state) { _jmpAndRestore(INST_JE  , label, state); }
  inline void jgAndRestore  (Label* label, State* state) { _jmpAndRestore(INST_JG  , label, state); }
  inline void jgeAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JGE , label, state); }
  inline void jlAndRestore  (Label* label, State* state) { _jmpAndRestore(INST_JL  , label, state); }
  inline void jleAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JLE , label, state); }
  inline void jnaAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JNA , label, state); }
  inline void jnaeAndRestore(Label* label, State* state) { _jmpAndRestore(INST_JNAE, label, state); }
  inline void jnbAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JNB , label, state); }
  inline void jnbeAndRestore(Label* label, State* state) { _jmpAndRestore(INST_JNBE, label, state); }
  inline void jncAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JNC , label, state); }
  inline void jneAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JNE , label, state); }
  inline void jngAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JNG , label, state); }
  inline void jngeAndRestore(Label* label, State* state) { _jmpAndRestore(INST_JNGE, label, state); }
  inline void jnlAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JNL , label, state); }
  inline void jnleAndRestore(Label* label, State* state) { _jmpAndRestore(INST_JNLE, label, state); }
  inline void jnoAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JNO , label, state); }
  inline void jnpAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JNP , label, state); }
  inline void jnsAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JNS , label, state); }
  inline void jnzAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JNZ , label, state); }
  inline void joAndRestore  (Label* label, State* state) { _jmpAndRestore(INST_JO  , label, state); }
  inline void jpAndRestore  (Label* label, State* state) { _jmpAndRestore(INST_JP  , label, state); }
  inline void jpeAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JPE , label, state); }
  inline void jpoAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JPO , label, state); }
  inline void jsAndRestore  (Label* label, State* state) { _jmpAndRestore(INST_JS  , label, state); }
  inline void jzAndRestore  (Label* label, State* state) { _jmpAndRestore(INST_JZ  , label, state); }
  inline void jmpAndRestore (Label* label, State* state) { _jmpAndRestore(INST_JMP , label, state); }

  // --------------------------------------------------------------------------
  // [Intrinsics]
  // --------------------------------------------------------------------------

  using SerializerIntrinsics::adc;
  using SerializerIntrinsics::add;
  using SerializerIntrinsics::and_;
  using SerializerIntrinsics::cmp;
  using SerializerIntrinsics::dec;
  using SerializerIntrinsics::inc;
  using SerializerIntrinsics::mov;
  using SerializerIntrinsics::neg;
  using SerializerIntrinsics::not_;
  using SerializerIntrinsics::or_;
  using SerializerIntrinsics::sbb;
  using SerializerIntrinsics::sub;
  using SerializerIntrinsics::xor_;

  inline void adc(const Register& dst, const Int32Ref& src) { op_reg32_var32(INST_ADC, dst, src); }
  inline void adc(const Int32Ref& dst, const Register& src) { op_var32_reg32(INST_ADC, dst, src); }
  inline void adc(const Int32Ref& dst, const Immediate& src) { op_var32_imm(INST_ADC, dst, src); }

  inline void add(const Register& dst, const Int32Ref& src) { op_reg32_var32(INST_ADD, dst, src); }
  inline void add(const Int32Ref& dst, const Register& src) { op_var32_reg32(INST_ADD, dst, src); }
  inline void add(const Int32Ref& dst, const Immediate& src) { op_var32_imm(INST_ADD, dst, src); }

  inline void and_(const Register& dst, const Int32Ref& src) { op_reg32_var32(INST_AND, dst, src); }
  inline void and_(const Int32Ref& dst, const Register& src) { op_var32_reg32(INST_AND, dst, src); }
  inline void and_(const Int32Ref& dst, const Immediate& src) { op_var32_imm(INST_AND, dst, src); }

  inline void cmp(const Register& dst, const Int32Ref& src) { op_reg32_var32(INST_CMP, dst, src); }
  inline void cmp(const Int32Ref& dst, const Register& src) { op_var32_reg32(INST_CMP, dst, src); }
  inline void cmp(const Int32Ref& dst, const Immediate& src) { op_var32_imm(INST_CMP, dst, src); }

  inline void dec(const Int32Ref& dst) { op_var32(INST_DEC, dst); }
  inline void inc(const Int32Ref& dst) { op_var32(INST_INC, dst); }
  inline void neg(const Int32Ref& dst) { op_var32(INST_NEG, dst); }
  inline void not_(const Int32Ref& dst) { op_var32(INST_NOT, dst); }

  inline void mov(const Register& dst, const Int32Ref& src) { op_reg32_var32(INST_MOV, dst, src); }
  inline void mov(const Int32Ref& dst, const Register& src) { op_var32_reg32(INST_MOV, dst, src); }
  inline void mov(const Int32Ref& dst, const Immediate& src) { op_var32_imm(INST_MOV, dst, src); }

  inline void or_(const Register& dst, const Int32Ref& src) { op_reg32_var32(INST_OR, dst, src); }
  inline void or_(const Int32Ref& dst, const Register& src) { op_var32_reg32(INST_OR, dst, src); }
  inline void or_(const Int32Ref& dst, const Immediate& src) { op_var32_imm(INST_OR, dst, src); }

  inline void sbb(const Register& dst, const Int32Ref& src) { op_reg32_var32(INST_SBB, dst, src); }
  inline void sbb(const Int32Ref& dst, const Register& src) { op_var32_reg32(INST_SBB, dst, src); }
  inline void sbb(const Int32Ref& dst, const Immediate& src) { op_var32_imm(INST_SBB, dst, src); }

  inline void sub(const Register& dst, const Int32Ref& src) { op_reg32_var32(INST_SUB, dst, src); }
  inline void sub(const Int32Ref& dst, const Register& src) { op_var32_reg32(INST_SUB, dst, src); }
  inline void sub(const Int32Ref& dst, const Immediate& src) { op_var32_imm(INST_SUB, dst, src); }

  inline void xor_(const Register& dst, const Int32Ref& src) { op_reg32_var32(INST_XOR, dst, src); }
  inline void xor_(const Int32Ref& dst, const Register& src) { op_var32_reg32(INST_XOR, dst, src); }
  inline void xor_(const Int32Ref& dst, const Immediate& src) { op_var32_imm(INST_XOR, dst, src); }

#if defined(ASMJIT_X64)
  inline void adc(const Register& dst, const Int64Ref& src) { op_reg64_var64(INST_ADC, dst, src); }
  inline void adc(const Int64Ref& dst, const Register& src) { op_var64_reg64(INST_ADC, dst, src); }
  inline void adc(const Int64Ref& dst, const Immediate& src) { op_var64_imm(INST_ADC, dst, src); }

  inline void add(const Register& dst, const Int64Ref& src) { op_reg64_var64(INST_ADD, dst, src); }
  inline void add(const Int64Ref& dst, const Register& src) { op_var64_reg64(INST_ADD, dst, src); }
  inline void add(const Int64Ref& dst, const Immediate& src) { op_var64_imm(INST_ADD, dst, src); }

  inline void and_(const Register& dst, const Int64Ref& src) { op_reg64_var64(INST_AND, dst, src); }
  inline void and_(const Int64Ref& dst, const Register& src) { op_var64_reg64(INST_AND, dst, src); }
  inline void and_(const Int64Ref& dst, const Immediate& src) { op_var64_imm(INST_AND, dst, src); }

  inline void cmp(const Register& dst, const Int64Ref& src) { op_reg64_var64(INST_CMP, dst, src); }
  inline void cmp(const Int64Ref& dst, const Register& src) { op_var64_reg64(INST_CMP, dst, src); }
  inline void cmp(const Int64Ref& dst, const Immediate& src) { op_var64_imm(INST_CMP, dst, src); }

  inline void dec(const Int64Ref& dst) { op_var64(INST_DEC, dst); }
  inline void inc(const Int64Ref& dst) { op_var64(INST_INC, dst); }
  inline void neg(const Int64Ref& dst) { op_var64(INST_NEG, dst); }
  inline void not_(const Int64Ref& dst) { op_var64(INST_NOT, dst); }

  inline void mov(const Register& dst, const Int64Ref& src) { op_reg64_var64(INST_MOV, dst, src); }
  inline void mov(const Int64Ref& dst, const Register& src) { op_var64_reg64(INST_MOV, dst, src); }
  inline void mov(const Int64Ref& dst, const Immediate& src) { op_var64_imm(INST_MOV, dst, src); }

  inline void or_(const Register& dst, const Int64Ref& src) { op_reg64_var64(INST_OR, dst, src); }
  inline void or_(const Int64Ref& dst, const Register& src) { op_var64_reg64(INST_OR, dst, src); }
  inline void or_(const Int64Ref& dst, const Immediate& src) { op_var64_imm(INST_OR, dst, src); }

  inline void sbb(const Register& dst, const Int64Ref& src) { op_reg64_var64(INST_SBB, dst, src); }
  inline void sbb(const Int64Ref& dst, const Register& src) { op_var64_reg64(INST_SBB, dst, src); }
  inline void sbb(const Int64Ref& dst, const Immediate& src) { op_var64_imm(INST_SBB, dst, src); }

  inline void sub(const Register& dst, const Int64Ref& src) { op_reg64_var64(INST_SUB, dst, src); }
  inline void sub(const Int64Ref& dst, const Register& src) { op_var64_reg64(INST_SUB, dst, src); }
  inline void sub(const Int64Ref& dst, const Immediate& src) { op_var64_imm(INST_SUB, dst, src); }

  inline void xor_(const Register& dst, const Int64Ref& src) { op_reg64_var64(INST_XOR, dst, src); }
  inline void xor_(const Int64Ref& dst, const Register& src) { op_var64_reg64(INST_XOR, dst, src); }
  inline void xor_(const Int64Ref& dst, const Immediate& src) { op_var64_imm(INST_XOR, dst, src); }
#endif // ASMJIT_X64
};

// ============================================================================
// [AsmJit::Compiler]
// ============================================================================

//! @brief Compiler - high level code generation.
//!
//! This class is used to store instruction stream and allows to modify
//! it on the fly. It uses different concept than @c AsmJit::Assembler class
//! and in fact @c AsmJit::Assembler is only used as a backend. Compiler never
//! emits machine code and each instruction you use is stored to instruction
//! array instead. This allows to modify instruction stream later and for 
//! example to reorder instructions to make better performance.
//!
//! Using @c AsmJit::Compiler moves code generation to higher level. Higher 
//! level constructs allows to write more abstract and extensible code that
//! is not possible with pure @c AsmJit::Assembler class. Because 
//! @c AsmJit::Compiler needs to create many objects and lifetime of these 
//! objects is small (same as @c AsmJit::Compiler lifetime itself) it uses 
//! very fast memory management model. This model allows to create object 
//! instances in nearly zero time (compared to @c malloc() or @c new() 
//! operators) so overhead by creating machine code by @c AsmJit::Compiler
//! is minimized.
//!
//! <b>Code Generation</b>
//! 
//! First that is needed to know about compiler is that compiler never emits
//! machine code. It's used as a middleware between @c AsmJit::Assembler and
//! your code. There is also convenience method @c make() that allows to
//! generate machine code directly without creating @c AsmJit::Assembler
//! instance.
//!
//! Example how to generate machine code using @c Assembler and @c Compiler:
//! 
//! @code
//! // Assembler instance is low level code generation class that emits 
//! // machine code.
//! Assembler a;
//!
//! // Compiler instance is high level code generation class that stores all
//! // instructions in internal representation.
//! Compiler c;
//!
//! // ... put your code using Compiler instance ...
//!
//! // Final step - generate code. AsmJit::Compiler::serialize() will serialize
//! // all instructions into Assembler and this ensures generating real machine
//! // code.
//! c.serialize(a);
//!
//! // Your function
//! void* fn = a.make();
//! @endcode
//!
//! Example how to generate machine code using only @c Compiler (preferred):
//!
//! @code
//! // Compiler instance is enough.
//! Compiler c;
//!
//! // ... put your code using Compiler instance ...
//!
//! // Your function
//! void* fn = c.make();
//! @endcode
//!
//! You can see that there is @c AsmJit::Compiler::serialize() function that
//! emits instructions into @c AsmJit::Assembler(). This layered architecture
//! means that each class is used for something different and there is no code
//! duplication. For convenience there is also @c AsmJit::Compiler::make()
//! method that can create your function using @c AsmJit::Assembler, but 
//! internally (this is preffered bahavior when using @c AsmJit::Compiler).
//!
//! @c make() allocates memory using global memory manager instance, if your
//! function lifetime is over, you should free that memory by
//! @c AsmJit::MemoryManager::free() method.
//!
//! @code
//! // Compiler instance is enough.
//! Compiler c;
//!
//! // ... put your code using Compiler instance ...
//!
//! // Your function
//! void* fn = c.make();
//!
//! // Free it if you don't want it anymore
//! // (using global memory manager instance)
//! MemoryManager::global()->free(fn);
//! @endcode
//!
//! <b>Functions</b>
//!
//! To build functions with @c Compiler, see @c AsmJit::Compiler::newFunction()
//! method.
//!
//! <b>Variables</b>
//!
//! Compiler also manages your variables and function arguments. Using manual
//! register allocation is not recommended way and it must be done carefully.
//! See @c AsmJit::VariableRef and related classes how to work with variables
//! and next example how to use AsmJit API to create function and manage them:
//!
//! @code
//! // Compiler and function declaration - void f(int*);
//! Compiler c;
//! Function& f = *c.newFunction(CALL_CONV_DEFAULT, BuildFunction1<int*>());
//!
//! // Get argument variable (it's pointer)
//! PtrRef a1(f.argument(0));
//!
//! // Create your variables
//! Int32Ref x1(f.newVariable(VARIABLE_TYPE_INT32));
//! Int32Ref x2(f.newVariable(VARIABLE_TYPE_INT32));
//!
//! // Init your variables
//! c.mov(x1.r(), 1);
//! c.mov(x2.r(), 2);
//!
//! // ... your code ...
//! c.add(x1.r(), x2.r());
//! // ... your code ...
//!
//! // Store result to a given pointer in first argument
//! c.mov(dword_ptr(a1.c()), x1.c());
//!
//! // Make function
//! typedef void (*MyFn)(int*);
//! MyFn fn = function_cast<MyFn>(c.make());
//! @endcode
//!
//! There was presented small code snippet with variables, but it's needed to 
//! explain it more. You can see that there are more variable types that can 
//! be used. Most useful variables that can be allocated to general purpose 
//! registers are variables wrapped to @c Int32Ref, @c Int64Ref, @c SysIntRef 
//! and @c PtrRef. Only @c Int64Ref is limited to 64 bit architecture. 
//! @c SysIntRef and @c PtrRef variables are equal and it's size depends to 
//! architecture (32 or 64 bits).
//!
//! Compiler is not using variables directly, instead you need to create the
//! function and create variables through @c AsmJit::Function. In code you will
//! always work with @c AsmJit::Compiler and @c AsmJit::Function together.
//!
//! Each variable contains state that describes where it is currently allocated
//! and if it's used. Life of variables is based on reference counting and if
//! variable is dereferenced to zero its life ends.
//!
//! Variable states:
//!
//! - Unused (@c AsmJit::VARIABLE_STATE_UNUSED) - State that is assigned to
//!   newly created variables or to not used variables (dereferenced to zero).
//! - In register (@c AsmJit::VARIABLE_STATE_REGISTER) - State that means that
//!   variable is currently allocated in register.
//! - In memory (@c AsmJit::VARIABLE_STATE_MEMORY) - State that means that
//!   variable is currently only in memory location.
//! 
//! When you create new variable, its state is always @c VARIABLE_STATE_UNUSED,
//! allocating it to register or spilling to memory changes this state to 
//! @c VARIABLE_STATE_REGISTER or @c VARIABLE_STATE_MEMORY, respectively. 
//! During variable lifetime it's usual that its state is changed multiple
//! times. To generate better code, you can control allocating and spilling
//! by using up to four types of methods that allows it (see next list).
//!
//! Explicit variable allocating / spilling methods:
//!
//! - @c VariableRef::alloc() - Explicit method to alloc variable into 
//!      register. You can use this before loops or code blocks.
//!
//! - @c VariableRef::spill() - Explicit method to spill variable. If variable
//!      is in register and you call this method, it's moved to its home memory
//!      location. If variable is not in register no operation is performed.
//!
//! Implicit variable allocating / spilling methods:
//!
//! - @c VariableRef::r() - Method used to allocate (if it's not previously
//!      allocated) variable to register for read / write. In most cases
//!      this is the right method to use in your code. If variable is in
//!      memory and you use this method it's allocated and moved to register.
//!      If variable is already in register or it's marked as unused this 
//!      method does nothing.
//! 
//! - @c VariableRef::x() - Method used to allocate variable for write only.
//!      In AsmJit this means completely overwrite it without using it's value.
//!      This method is helpful when you want to prevent from copying variable
//!      from memory to register to save one mov() instruction. If you want
//!      to clear or set your variable to something it's recommended to use
//!      @c VariableRef::x().
//!
//! - @c VariableRef::c() - Method used to use variable as a constant. 
//!      Constants means that you will not change that variable or you don't
//!      want to mark variable as changed. If variable is not marked as changed
//!      and spill happens you will save one mov() instruction that is needed
//!      to copy variable from register to its home address.
//!
//! - @c VariableRef::m() - Method used to access variable memory address. If
//!      variable is allocated in register and you call this method, it's 
//!      spilled, in all other cases it does nothing.
//!
//! Next example shows how allocating and spilling works:
//!
//! @code
//! // Small example to show how variable allocating and spilling works
//!
//! // Your compiler
//! Compiler c;
//!
//! // Your variable
//! Int32Ref var = ...;
//! 
//! // Make sure variable is spilled
//! var.spill();
//! 
//! // 1. Example: using var.r()
//! c.mov(var.r(), imm(0));
//! var.spill();
//! // Generated code:
//! //    mov var.reg, [var.home]
//! //    mov var.reg, 0
//! //    mov [var.home], var.reg
//! 
//! // 2. Example: using var.x()
//! c.mov(var.x(), imm(0));
//! var.spill();
//! // Generated code:
//! //    --- no alloc, .x() inhibits it.
//! //    mov var.reg, 0
//! //    mov [var.home], var.reg
//! 
//! // 3. Example: using var.c()
//! c.mov(var.c(), imm(0));
//! var.spill();
//! // Generated code:
//! //    mov var.reg, [var.home]
//! //    mov var.reg, 0
//! //    --- no spill, .c() means that you are not changing it, it's 'c'onstant
//! 
//! // 4. Example: using var.m()
//! c.mov(var.m(), imm(0));
//! var.spill();
//! // Generated code:
//! //    --- no alloc, because we are not allocating it
//! //    mov [var.home], 0
//! //    --- no spill, because variable is not allocated
//!
//! // 5. Example: using var.x(), setChanged()
//! c.mov(var.x(),imm(0));
//! var.setChanged(false);
//! var.spill();
//! // Generated code:
//! //    --- no alloc, .x() inhibits it.
//! //    mov var.reg, 0
//! //    --- no spill, setChanged(false) marked variable as unmodified
//! @endcode
//!
//! Please see AsmJit tutorials (testcompiler.cpp and testvariables.cpp) for 
//! more complete examples.
//!
//! <b>Intrinsics Extensions</b>
//!
//! Compiler supports extensions to intrinsics implemented in 
//! @c AsmJit::Serializer that enables to use variables in instructions without
//! specifying to use it as register or as memory operand. Sometimes is better
//! not to alloc variable for each read or write. There is limitation that you
//! can use variable without specifying if it's in register or in memory 
//! location only for one operand. This is because x86/x64 architecture not
//! allows to use two memory operands in one instruction and this could 
//! happen without this restriction (two variables in memory).
//!
//! @code
//! // Small example to show how intrinsics extensions works
//!
//! // Your compiler
//! Compiler c;
//!
//! // Your variable
//! Int32Ref var = ...;
//! 
//! // Make sure variable is spilled
//! var.spill();
//! 
//! // 1. Example: Allocated variable
//! var.alloc()
//! c.mov(var, imm(0));
//! var.spill();
//! // Generated code:
//! //    mov var.reg, [var.home]
//! //    mov var.reg, 0
//! //    mov [var.home], var.reg
//! 
//! // 2. Example: Memory variable
//! c.mov(var, imm(0));
//! var.spill();
//! // Generated code:
//! //    --- no alloc, we want variable in memory
//! //    mov [var.home], 0
//! //    --- no spill, becuase variable in not allocated
//! @endcode
//!
//! <b>Memory Management</b>
//!
//! @c Compiler Memory management follows these rules:
//! - Everything created by @c Compiler is always freed by @c Compiler.
//! - To get decent performance, compiler always uses larger buffer for 
//!   objects to allocate and when compiler instance is destroyed, this 
//!   buffer is freed. Destructors of active objects are called when 
//!   destroying compiler instance. Destructors of abadonded compiler
//!   objects are called immediately after abadonding it.
//!
//! This means that you can't use any @c Compiler object after destructing it,
//! it also means that each object like @c Label, @c Variable nad others are
//! created and managed by @c Compiler itself.
//!
//! <b>Compiling process details</b>
//!
//! This section is here for people interested in the compiling process. There
//! are few steps that must be done for each compiled function (or your code).
//!
//! When your Compiler instance is ready, you can create function and add
//! emittables using intrinsics or higher level methods implemented in the
//! @c AsmJit::Compiler. When you are done serializing instructions you will 
//! usually call @c AsmJit::Compiler::make() method to serialize all emittables 
//! to @c AsmJit::Assembler. Next steps shows what's done internally before code
//! is serialized into @c AsmJit::Assembler
//!   (implemented in @c AsmJit::Compiler::serialize() method).
//! 
//! 1. All emittables are traversed (from first to last) and method 
//!    @c AsmJit::Emittable::prepare() is called. This signalizes to all 
//!    emittables that instruction generation step is over and now they
//!    should prepare to code generation. In this step can be processed
//!    variables, states, etc...
//! 2. All emittables are traversed (from first to last) and method 
//!    @c AsmJit::Emittable::emit() is called. In this step each emittable
//!    can serialize real assembler instructions into @c AsmJit::Assembler
//!    instance. This step also generates function prolog and epilog.
//! 3. All emittables are traversed (from first to last) and method 
//!    @c AsmJit::Emittable::postEmit() is called. Post emitting is used
//!    to embed data after function body (not only user data, but also some
//!    helper data that can help generating jumps, variables restore / save
//!    sequences, condition blocks).
//! 4. Jump tables data are emitted.
//!
//! When everything here ends, @c AsmJit::Assembler contains binary stream
//! that needs only relocation to be callable.
//!
//! <b>Differences summary to @c AsmJit::Assembler</b>
//!
//! - Instructions are not translated to machine code immediately, they are
//!   stored as @c Emmitable's (see @c AsmJit::Instruction).
//! - Each @c Label must be allocated by @c AsmJit::Compiler::newLabel().
//! - Contains function builder.
//! - Contains register allocator / variables management.
//! - Contains a lot of helper methods to simplify code generation.
struct ASMJIT_API Compiler : public CompilerIntrinsics
{
  //! @brief Create a new @c Compiler instance.
  Compiler() ASMJIT_NOTHROW;
  //! @brief Destroy @c Compiler instance.
  virtual ~Compiler() ASMJIT_NOTHROW;
};

//! @}

} // AsmJit namespace

// [Warnings-Pop]
#include "WarningsPop.h"

// [Guard]
#endif // _ASMJIT_COMPILERX86X64_H

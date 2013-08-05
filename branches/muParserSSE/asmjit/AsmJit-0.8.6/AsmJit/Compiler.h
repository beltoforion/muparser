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
#ifndef _ASMJIT_COMPILER_H
#define _ASMJIT_COMPILER_H

// [Dependencies]
#include "Build.h"

namespace AsmJit {

//! @addtogroup AsmJit_Compiler
//! @{

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Comment;
struct Compiler;
struct Emittable;
struct Epilog;
struct Function;
struct Instruction;
struct Prolog;
struct State;
struct Variable;

// ============================================================================
// [Constants]
// ============================================================================

//! @brief Emmitable type.
//!
//! For each emittable that is used by @c Compiler must be defined it's type.
//! Compiler can optimize instruction stream by analyzing emittables and each
//! type is hint for it. The most used emittables are instructions
//! (@c EMITTABLE_INSTRUCTION).
enum EMITTABLE_TYPE
{
  //! @brief Emittable is invalid (can't be used).
  EMITTABLE_NONE = 0,
  //! @brief Emittable is comment (no code).
  EMITTABLE_COMMENT,
  //! @brief Emittable is embedded data.
  EMITTABLE_EMBEDDED_DATA,
  //! @brief Emittable is .align directive.
  EMITTABLE_ALIGN,
  //! @brief Emittable is single instruction.
  EMITTABLE_INSTRUCTION,
  //! @brief Emittable is block of instructions.
  EMITTABLE_BLOCK,
  //! @brief Emittable is function declaration.
  EMITTABLE_FUNCTION,
  //! @brief Emittable is function prolog.
  EMITTABLE_PROLOGUE,
  //! @brief Emittable is function epilog.
  EMITTABLE_EPILOGUE,
  //! @brief Emittable is target (bound label).
  EMITTABLE_TARGET,
  //! @brief Emittable is jump table.
  EMITTABLE_JUMP_TABLE
};

//! @brief State of variable.
//!
//! Variable state can be retrieved by @c AsmJit::VariableRef::state().
enum VARIABLE_STATE
{
  //! @brief Variable is currently not used.
  //!
  //! Variables of this state are not used or they are currently not
  //! initialized (short time after @c AsmJit::VariableRef::alloc() call).
  VARIABLE_STATE_UNUSED = 0,

  //! @brief Variable is in register.
  //!
  //! Variable is currently allocated in register.
  VARIABLE_STATE_REGISTER = 1,

  //! @brief Variable is in memory location or spilled.
  //!
  //! Variable was spilled from register to memory or variable is used for
  //! memory only storage.
  VARIABLE_STATE_MEMORY = 2
};

//! @brief Variable alloc mode.
//! @internal
enum VARIABLE_ALLOC
{
  //! @brief Allocating variable to read only.
  //!
  //! Read only variables are used to optimize variable spilling. If variable
  //! is some time ago deallocated and it's not marked as changed (so it was
  //! all the life time read only) then spill is simply NOP (no mov instruction
  //! is generated to move it to it's home memory location).
  VARIABLE_ALLOC_READ = 0x1,

  //! @brief Allocating variable to write only (overwrite).
  //!
  //! Overwriting means that if variable is in memory, there is no generated
  //! instruction to move variable from memory to register, because that
  //! register will be overwritten by next instruction. This is used as a
  //! simple optimization to improve generated code by @c Compiler.
  VARIABLE_ALLOC_WRITE = 0x2,

  //! @brief Allocating variable to read / write.
  //!
  //! Variable allocated for read / write is marked as changed. This means that
  //! if variable must be later spilled into memory, mov (or similar)
  //! instruction will be generated.
  VARIABLE_ALLOC_READWRITE = 0x3
};

//! @brief Variable allocation method.
//!
//! Variable allocation method is used by compiler and it means if compiler
//! should first allocate preserved registers or not. Preserved registers are
//! registers that must be saved / restored by generated function.
//!
//! This option is for people who are calling C/C++ functions from JIT code so
//! Compiler can recude generating push/pop sequences before and after call,
//! respectively.
enum ALLOC_POLICY
{
  //! @brief Allocate preserved registers first.
  ALLOC_POLICY_PRESERVED_FIRST,
  //! @brief Allocate preserved registers last (default).
  ALLOC_POLICY_PRESERVED_LAST
};

//! @brief Arguments direction used by @c Function.
enum ARGUMENT_DIR
{
  //! @brief Arguments are passed left to right.
  //!
  //! This arguments direction is unusual to C programming, it's used by pascal
  //! compilers and in some calling conventions by Borland compiler).
  ARGUMENT_DIR_LEFT_TO_RIGHT = 0,
  //! @brief Arguments are passer right ro left
  //!
  //! This is default argument direction in C programming.
  ARGUMENT_DIR_RIGHT_TO_LEFT = 1
};

//! @brief Anonymous constants used by @c Compiler.
enum {
  //! @brief Maximum length of variable name.
  MAX_VARIABLE_LENGTH = 32
};

//! @}

} // AsmJit namespace.

// ============================================================================
// [Platform Specific]
//
// Following enums must be declared by platform specific header:
// - CALL_CONV - Calling convention.
// - VARIABLE_TYPE - Variable type.
// ============================================================================

// [X86 / X64]
#if defined(ASMJIT_X86) || defined(ASMJIT_X64)
#include "CompilerX86X64.h"
#endif // ASMJIT_X86 || ASMJIT_X64

// [Guard]
#endif // _ASMJIT_COMPILER_H

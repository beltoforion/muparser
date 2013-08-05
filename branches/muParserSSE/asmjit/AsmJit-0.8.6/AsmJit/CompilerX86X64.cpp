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

// We are using sprintf() here.
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER

// [Dependencies]
#include "Assembler.h"
#include "Compiler.h"
#include "CpuInfo.h"
#include "Logger.h"
#include "Util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// [Warnings-Push]
#include "WarningsPush.h"

// VARIABLE_TYPE_INT64 not declared in 32 bit mode, in future this should 
// change
#if defined(ASMJIT_X86)
#define VARIABLE_TYPE_INT64 2
#endif // ASMJIT_X86

namespace AsmJit {

// ============================================================================
// [Helpers]
// ============================================================================

static void delAll(Emittable* first)
{
  Emittable* cur;
  for (cur = first; cur; cur = cur->next()) cur->~Emittable();
}

static void memset32(UInt32* p, UInt32 c, SysUInt len)
{
  SysUInt i;
  for (i = 0; i < len; i++) p[i] = c;
}

struct VariableInfo
{
  enum CLASS_INFO
  {
    CLASS_NONE   = 0x00,
    CLASS_GP     = 0x01,
    CLASS_X87    = 0x02,
    CLASS_MM     = 0x04,
    CLASS_XMM    = 0x08,
    CLASS_SP_FP  = 0x10,
    CLASS_DP_FP  = 0x20,
    CLASS_VECTOR = 0x40
  };

  UInt8 size;
  UInt8 regCode;
  UInt8 clazz;
  UInt8 reserved;
  const char name[12];
};

static VariableInfo variableInfo[] = {
  { 0 , NO_REG , VariableInfo::CLASS_NONE                                                         , 0, "none"        },
  { 4 , REG_GPD, VariableInfo::CLASS_GP                                                           , 0, "int32"       },
  { 8 , REG_GPQ, VariableInfo::CLASS_GP                                                           , 0, "int64"       },
  { 4 , REG_X87, VariableInfo::CLASS_X87  | VariableInfo::CLASS_SP_FP                             , 0, "x87_float"   },
  { 8 , REG_X87, VariableInfo::CLASS_X87  | VariableInfo::CLASS_DP_FP                             , 0, "x87_double"  },
  { 4 , REG_XMM, VariableInfo::CLASS_XMM  | VariableInfo::CLASS_SP_FP                             , 0, "xmm_float"   },
  { 8 , REG_XMM, VariableInfo::CLASS_XMM  | VariableInfo::CLASS_DP_FP                             , 0, "xmm_double"  },
  { 16, REG_XMM, VariableInfo::CLASS_XMM  | VariableInfo::CLASS_SP_FP | VariableInfo::CLASS_VECTOR, 0, "xmm_float4"  },
  { 16, REG_XMM, VariableInfo::CLASS_XMM  | VariableInfo::CLASS_DP_FP | VariableInfo::CLASS_VECTOR, 0, "xmm_double2" },
  { 8 , REG_MM , VariableInfo::CLASS_MM                                                           , 0, "mm"          },
  { 16, REG_XMM, VariableInfo::CLASS_XMM                                                          , 0, "xmm"         }
};

static UInt32 getVariableSize(UInt32 type)
{
  ASMJIT_ASSERT(type < ASMJIT_ARRAY_SIZE(variableInfo));
  return variableInfo[type].size;
}

static UInt8 getVariableRegisterCode(UInt32 type, UInt8 index)
{
  ASMJIT_ASSERT(type < ASMJIT_ARRAY_SIZE(variableInfo));
  return variableInfo[type].regCode | index;
}

static bool isIntegerVariable(UInt32 type)
{
  ASMJIT_ASSERT(type < ASMJIT_ARRAY_SIZE(variableInfo));
  return (variableInfo[type].clazz & VariableInfo::CLASS_GP) != 0;
}

static bool isFloatArgument(UInt32 type)
{
  ASMJIT_ASSERT(type < ASMJIT_ARRAY_SIZE(variableInfo));
  return (variableInfo[type].clazz & (VariableInfo::CLASS_SP_FP | VariableInfo::CLASS_DP_FP)) != 0;
}

// ============================================================================
// [AsmJit::Variable]
// ============================================================================

Variable::Variable(Compiler* c, Function* f, UInt8 type) :
  _compiler(c),
  _function(f),
  _refCount(0),
  _spillCount(0),
  _registerAccessCount(0),
  _memoryAccessCount(0),
  _lifeId(0),
  _globalSpillCount(0),
  _globalRegisterAccessCount(0),
  _globalMemoryAccessCount(0),
  _type(type),
  _size(getVariableSize(type)),
  _state(VARIABLE_STATE_UNUSED),
  _priority(10),
  _registerCode(NO_REG),
  _preferredRegisterCode(0xFF),
  _homeRegisterCode(0xFF),
  _changed(false),
  _reusable(true),
  _customMemoryHome(false),
  _stackArgument(false),
  _stackOffset(0),
  _allocFn(NULL),
  _spillFn(NULL),
  _dataPtr(NULL),
  _dataInt(0)
{
  ASMJIT_ASSERT(f != NULL);

  _memoryOperand = new(c->_zoneAlloc(sizeof(Mem))) Mem(ebp, 0, _size);
  c->_registerOperand(_memoryOperand);

  _name[0] = '\0';
}

Variable::~Variable()
{
}

void Variable::setPriority(UInt8 priority)
{
  _priority = priority;
  
  // Alloc if priority is set to 0
  if (priority == 0) _function->alloc(this);
}

void Variable::setMemoryHome(const Mem& memoryHome) ASMJIT_NOTHROW
{
  _reusable = false;
  _customMemoryHome = true;
  *_memoryOperand = memoryHome;
}

Variable* Variable::ref()
{
  _refCount++;
  return this;
}

void Variable::deref()
{
  if (--_refCount == 0) unuse();
}

void Variable::getReg(
  UInt8 mode, UInt8 preferredRegister,
  BaseReg* dest, UInt8 regType)
{
  alloc(mode, preferredRegister);

  // Setup register in dest
  UInt8 size = 1U << (regType >> 4);
  if (regType == REG_X87) size = 10;
  if (regType == REG_MM ) size = 8;
  if (regType == REG_XMM) size = 16;

  *dest = BaseReg((_registerCode & REGCODE_MASK) | regType, size);

  // Statistics
  _registerAccessCount++;
  _globalRegisterAccessCount++;
}

const Mem& Variable::m()
{
  if (!spill())
  {
    // TODO: Error handling.
  }

  _memoryAccessCount++;
  _globalMemoryAccessCount++;

  return *_memoryOperand;
}

void Variable::setName(const char* name) ASMJIT_NOTHROW
{
  SysUInt len = 0;

  if (name) len = strlen(name);
  if (len > MAX_VARIABLE_LENGTH - 1) len = MAX_VARIABLE_LENGTH - 1;

  memcpy(_name, name, len + 1);
}

// ============================================================================
// [AsmJit::State]
// ============================================================================

// this helper structure is used for jumps that ensures that variable allocator
// state is updated to correct values. It's saved in Label operand as single
// linked list.
struct JumpAndRestore
{
  JumpAndRestore* next;
  Instruction* instruction;
  State* from;
  State* to;
};

State::State(Compiler* c, Function* f) :
  _compiler(c),
  _function(f)
{
  _clear();
}

State::~State()
{
}

void State::saveFunctionState(Data* dst, Function* f)
{
  for (SysUInt i = 0; i < 16+8+16; i++)
  {
    Variable* v = f->_state.regs[i];

    if (v)
    {
      dst->regs[i].v = v;
      dst->regs[i].lifeId = v->lifeId();
      dst->regs[i].state = v->state();
      dst->regs[i].changed = v->changed();
    }
    else
    {
      memset(&dst->regs[i], 0, sizeof(Entry));
    }
  }

  dst->usedGpRegisters  = f->usedGpRegisters ();
  dst->usedMmRegisters  = f->usedMmRegisters ();
  dst->usedXmmRegisters = f->usedXmmRegisters();
}

void State::_clear()
{
  memset(&_data, 0, sizeof(_data));
}

// ============================================================================
// [AsmJit::Emittable]
// ============================================================================

Emittable::Emittable(Compiler* c, UInt32 type) ASMJIT_NOTHROW :
  _compiler(c),
  _next(NULL),
  _prev(NULL),
  _type(type)
{
}

Emittable::~Emittable() ASMJIT_NOTHROW
{
}

void Emittable::prepare()
{
  // ...nothing...
}

void Emittable::postEmit(Assembler& a)
{
  // ...nothing...
}

// ============================================================================
// [AsmJit::Comment]
// ============================================================================

Comment::Comment(Compiler* c, const char* str) ASMJIT_NOTHROW :
  Emittable(c, EMITTABLE_COMMENT)
{
  SysUInt len = strlen(str);

  // Alloc string, but round it up
  _str = (char*)c->_zoneAlloc(
    (len + sizeof(SysInt)) & ~(sizeof(SysInt)-1));
  memcpy((char*)_str, str, len + 1);
}

Comment::~Comment() ASMJIT_NOTHROW
{
}

void Comment::emit(Assembler& a)
{
  if (a.logger()) a.logger()->log(str());
}

// ============================================================================
// [AsmJit::EmbeddedData]
// ============================================================================

EmbeddedData::EmbeddedData(Compiler* c, SysUInt capacity, const void* data, SysUInt size) ASMJIT_NOTHROW :
  Emittable(c, EMITTABLE_EMBEDDED_DATA)
{
  ASMJIT_ASSERT(capacity >= size);

  _size = size;
  _capacity = capacity;
  memcpy(_data, data, size);
}

EmbeddedData::~EmbeddedData() ASMJIT_NOTHROW
{
}

void EmbeddedData::emit(Assembler& a)
{
  a._embed(data(), size());
}

// ============================================================================
// [AsmJit::Align]
// ============================================================================

Align::Align(Compiler* c, SysInt size) ASMJIT_NOTHROW :
  Emittable(c, EMITTABLE_ALIGN), _size(size) 
{
}

Align::~Align() ASMJIT_NOTHROW
{
}

void Align::emit(Assembler& a)
{
  a.align(size());
}

// ============================================================================
// [AsmJit::Instruction]
// ============================================================================

// Used for NULL operands to translate them to OP_NONE.
static const UInt8 _none[sizeof(Operand)] = { 0 };

Instruction::Instruction(Compiler* c) ASMJIT_NOTHROW :
  Emittable(c, EMITTABLE_INSTRUCTION)
{
  _o[0] = &_ocache[0];
  _o[1] = &_ocache[1];
  _o[2] = &_ocache[2];
  memset(_ocache, 0, sizeof(_ocache));
}

Instruction::Instruction(Compiler* c,
  UInt32 code,
  const Operand* o1, const Operand* o2, const Operand* o3,
  const char* inlineComment) ASMJIT_NOTHROW : Emittable(c, EMITTABLE_INSTRUCTION)
{
  _code = code;
  _inlineComment = inlineComment;

  UInt32 oid;

  _o[0] = &_ocache[0];
  _o[1] = &_ocache[1];
  _o[2] = &_ocache[2];

  if (o1 == NULL) { memset(_o[0], 0, sizeof(Operand)); } else if ((oid = o1->operandId()) != 0) { ASMJIT_ASSERT(oid < c->_operands.length()); _o[0] = c->_operands[oid]; } else { _ocache[0] = *o1; }
  if (o2 == NULL) { memset(_o[1], 0, sizeof(Operand)); } else if ((oid = o2->operandId()) != 0) { ASMJIT_ASSERT(oid < c->_operands.length()); _o[1] = c->_operands[oid]; } else { _ocache[1] = *o2; }
  if (o3 == NULL) { memset(_o[2], 0, sizeof(Operand)); } else if ((oid = o3->operandId()) != 0) { ASMJIT_ASSERT(oid < c->_operands.length()); _o[2] = c->_operands[oid]; } else { _ocache[2] = *o3; }
}

Instruction::~Instruction() ASMJIT_NOTHROW
{
}

void Instruction::emit(Assembler& a)
{
  if (_inlineComment) a._inlineComment(_inlineComment);

  a._emitX86(code(), _o[0], _o[1], _o[2]);
}

// ============================================================================
// [AsmJit::Function]
// ============================================================================

Function::Function(Compiler* c) ASMJIT_NOTHROW :
  Emittable(c, EMITTABLE_FUNCTION),
  _stackAlignmentSize(sizeof(SysInt) == 4 ? 0 : 16),
  _variablesStackSize(0),
  _cconv(CALL_CONV_NONE),
  _calleePopsStack(false),
  _naked(false),
  _allocableEbp(false),
  _prologEpilogPushPop(true),
  _emms(false),
  _sfence(false),
  _lfence(false),
  _optimizedPrologEpilog(true),
  _cconvArgumentsDirection(ARGUMENT_DIR_RIGHT_TO_LEFT),
  _cconvPreservedGp(0),
  _cconvPreservedXmm(0),
  _argumentsCount(0),
  _argumentsStackSize(0),
  _usedGpRegisters(0),
  _usedMmRegisters(0),
  _usedXmmRegisters(0),
  _modifiedGpRegisters(0),
  _modifiedMmRegisters(0),
  _modifiedXmmRegisters(0),
  _usePrevention(true),
  _entryLabel(c->newLabel()),
  _prologLabel(c->newLabel()),
  _exitLabel(c->newLabel())
{
  memset32(_cconvArgumentsGp, 0xFFFFFFFF, 16);
  memset32(_cconvArgumentsXmm, 0xFFFFFFFF, 16);
  memset(&_state, 0, sizeof(_state));
}

Function::~Function() ASMJIT_NOTHROW
{
}

void Function::prepare()
{
  // Prepare variables
  static const UInt32 sizes[] = { 16, 8, 4, 2, 1 };

  SysUInt i, v;

  SysInt sp = 0;       // Stack offset
  SysInt pe;           // Prolog / epilog size
  SysInt peGp;         // Prolog / epilog for GP registers size
  SysInt peXmm;        // Prolog / epilog for XMM registers size

  UInt8 argMemBase;    // Address base register for function arguments
  UInt8 varMemBase;    // Address base register for function variables
  SysInt argDisp = 0;  // Displacement for arguments
  SysInt varDisp = 0;  // Displacement for variables
  UInt32 alignSize = 0;// Maximum alignment stack size

  // This is simple optimization to do 16 byte aligned variables first and
  // all others next.
  for (i = 0; i < ASMJIT_ARRAY_SIZE(sizes); i++)
  {
    // See sizes declaration. We allocate variables on the stack in ordered way:
    // 16 byte variables (xmm) first,
    // 8 byte variables (mmx, gpq) second,
    // 4,2,1 (these not needs to be aligned)
    UInt32 size = sizes[i];

    for (v = 0; v < _variables.length(); v++)
    {
      Variable* var = _variables[v];

      // Use only variable with size 'size' and function arguments that was
      // passed through registers.
      if (var->size() == size && !var->stackArgument() && var->_globalMemoryAccessCount > 0)
      {
        // X86 stack is aligned to 32 bits (4 bytes) in 32-bit mode (Is this
        // correct?) and for 128-bits (16 bytes) in 64-bit mode.
        //
        // For MMX and SSE  programming we need 8 or 16 bytes alignment. For
        // MMX/SSE memory operands we can be adjusted by 4 or 12 bytes.
#if defined(ASMJIT_X86)
        if (size ==  8 && alignSize <  8) alignSize =  8;
        if (size == 16 && alignSize < 16) alignSize = 16;
#endif // ASMJIT_X86

        _variables[v]->_stackOffset = sp;
        sp += size;
      }
    }
  }

  // Align to 16 bytes.
  sp = (sp + 15) & ~15;

  // Get prolog/epilog push/pop size on the stack.
  peGp = countOfGpRegistersToBeSaved() * sizeof(SysInt);
  peXmm = countOfXmmRegistersToBeSaved() * 16;
  pe = peGp + peXmm;

  _prologEpilogStackSize = pe;
  _variablesStackSize = sp;
  _stackAlignmentSize = alignSize;

  // Calculate displacements.
  if (naked())
  {
    // Naked functions are using always esp/rsp.
    argMemBase = RID_ESP;
    argDisp = prologEpilogPushPop() ? peGp : 0;

    varMemBase = RID_ESP;
    varDisp = -sp - sizeof(SysInt);
  }
  else
  {
    // Functions with prolog/epilog are using ebp/rbp for variables.
    argMemBase = RID_EBP;
    // Push ebp/rpb size (return address is already in arguments stack offset).
    argDisp = sizeof(SysInt);

    varMemBase = RID_ESP;
    varDisp = 0;
  }

  // Patch all variables to point to correct address in memory
  for (v = 0; v < _variables.length(); v++)
  {
    Variable* var = _variables[v];
    Mem* memop = var->_memoryOperand;

    // Different stack home for function arguments and variables.
    //
    // NOTE: Function arguments given in registers can be relocated onto the
    // stack without problems. This code doest something different, it will
    // not change stack arguments location. So only stack based arguments needs
    // this special handling
    if (var->stackArgument())
    {
      memop->_mem.base = argMemBase;
      memop->_mem.displacement = var->stackOffset() + argDisp;
    }
    else
    {
      memop->_mem.base = varMemBase;
      memop->_mem.displacement = var->stackOffset() + varDisp;
    }
  }
}

void Function::emit(Assembler& a)
{
  // Dump function and statistics if logging enabled.
  Logger* logger = a.logger();
  if (logger && logger->enabled())
  {
    char _buf[1024];
    char* p;
    const char* loc;

    SysUInt i;
    SysUInt varlen = _variables.length();

    // Log function and its parameters
    logger->log("; Function Prototype:\n");
    logger->log(";   (");
    for (i = 0; i < argumentsCount(); i++)
    {
      Variable* v = _variables[i];

      if (i != 0) logger->log(", ");
      logger->log(
        v->type() < _VARIABLE_TYPE_COUNT ? variableInfo[v->type()].name : "unknown");
    }
    logger->log(")\n");
    logger->log(";\n");
    logger->log("; Variables:\n");

    // Log variables
    for (i = 0; i < varlen; i++)
    {
      Variable* v = _variables[i];
      const VariableInfo* vinfo = &variableInfo[v->type()];

      if (v->_globalMemoryAccessCount > 0)
      {
        loc = _buf;
        Logger::dumpOperand(_buf, v->_memoryOperand)[0] = '\0';
      }
      else
      {
        loc = "[None]";
      }

      logger->logFormat(";   %-2u %-12s (%2uB) at %-20s - reg access: %-3u, mem access: %-3u\n",
        // Variable id
        (unsigned int)i,
        // Variable type
        v->type() < _VARIABLE_TYPE_COUNT ? vinfo->name : "unknown",
        // Variable size
        v->size(),
        // Variable memory address
        loc,
        // Register access count
        (unsigned int)v->_globalRegisterAccessCount,
        // Memory access count
        (unsigned int)v->_globalMemoryAccessCount
      );
    }

    // Log Registers
    p = _buf;

    SysUInt r;
    SysUInt modifiedRegisters = 0;
    for (r = 0; r < 3; r++)
    {
      bool first = true;
      UInt32 regs;
      UInt32 type;

      memcpy(p, ";   ", 4); p += 4;

      switch (r)
      {
        case 0: regs = _modifiedGpRegisters ; type = REG_GPN; memcpy(p, "GP :", 4); p += 4; break;
        case 1: regs = _modifiedMmRegisters ; type = REG_MM ; memcpy(p, "MM :", 4); p += 4; break;
        case 2: regs = _modifiedXmmRegisters; type = REG_XMM; memcpy(p, "XMM:", 4); p += 4; break;
      }

      for (i = 0; i < NUM_REGS; i++)
      {
        if ((regs & (1 << i)) != 0)
        {
          if (!first) { *p++ = ','; *p++ = ' '; }
          p = Logger::dumpRegister(p, (UInt8)type, (UInt8)i);
          first = false;
          modifiedRegisters++;
        }
      }
      *p++ = '\n';
    }
    *p = '\0';

    logger->logFormat(";\n");
    logger->logFormat("; Modified registers (%u):\n",
      (unsigned int)modifiedRegisters);
    logger->log(_buf);
  }

  a.bind(_entryLabel);
}

void Function::setPrototype(UInt32 cconv, const UInt32* args, SysUInt count)
{
  _setCallingConvention(cconv);
  _setArguments(args, count);
}

void Function::setNaked(UInt8 naked) ASMJIT_NOTHROW
{
  if (_naked == naked) return;

  _naked = naked;
}

void Function::_setCallingConvention(UInt32 cconv) ASMJIT_NOTHROW
{
  // Safe defaults
  _cconv = cconv;
  _calleePopsStack = false;

  memset32(_cconvArgumentsGp, 0xFFFFFFFF, 16);
  memset32(_cconvArgumentsXmm, 0xFFFFFFFF, 16);
  memset(&_state, 0, sizeof(_state));

  _cconvArgumentsDirection = ARGUMENT_DIR_RIGHT_TO_LEFT;
  _argumentsStackSize = 0;

#if defined(ASMJIT_X86)
  // [X86 calling conventions]
  _cconvPreservedGp =
    (1 << (REG_EBX & REGCODE_MASK)) |
    (1 << (REG_ESP & REGCODE_MASK)) |
    (1 << (REG_EBP & REGCODE_MASK)) |
    (1 << (REG_ESI & REGCODE_MASK)) |
    (1 << (REG_EDI & REGCODE_MASK)) ;
  _cconvPreservedXmm = 0;

  switch (cconv)
  {
    case CALL_CONV_CDECL:
      break;
    case CALL_CONV_STDCALL:
      _calleePopsStack = true;
      break;
    case CALL_CONV_MSTHISCALL:
      _cconvArgumentsGp[0] = (REG_ECX & REGCODE_MASK);
      _calleePopsStack = true;
      break;
    case CALL_CONV_MSFASTCALL:
      _cconvArgumentsGp[0] = (REG_ECX & REGCODE_MASK);
      _cconvArgumentsGp[1] = (REG_EDX & REGCODE_MASK);
      _calleePopsStack = true;
      break;
    case CALL_CONV_BORLANDFASTCALL:
      _cconvArgumentsGp[0] = (REG_EAX & REGCODE_MASK);
      _cconvArgumentsGp[1] = (REG_EDX & REGCODE_MASK);
      _cconvArgumentsGp[2] = (REG_ECX & REGCODE_MASK);
      _cconvArgumentsDirection = ARGUMENT_DIR_LEFT_TO_RIGHT;
      _calleePopsStack = true;
      break;
    case CALL_CONV_GCCFASTCALL_2:
      _cconvArgumentsGp[0] = (REG_ECX & REGCODE_MASK);
      _cconvArgumentsGp[1] = (REG_EDX & REGCODE_MASK);
      _calleePopsStack = false;
      break;
    case CALL_CONV_GCCFASTCALL_3:
      _cconvArgumentsGp[0] = (REG_EDX & REGCODE_MASK);
      _cconvArgumentsGp[1] = (REG_ECX & REGCODE_MASK);
      _cconvArgumentsGp[2] = (REG_EAX & REGCODE_MASK);
      _calleePopsStack = false;
      break;

    default:
      // Illegal calling convention.
      ASMJIT_ASSERT(0);
  }
#else
  // [X64 calling conventions]
  switch(cconv)
  {
    case CALL_CONV_X64W:
      _cconvPreservedGp =
        (1 << (REG_RBX   & REGCODE_MASK)) |
        (1 << (REG_RSP   & REGCODE_MASK)) |
        (1 << (REG_RBP   & REGCODE_MASK)) |
        (1 << (REG_RSI   & REGCODE_MASK)) |
        (1 << (REG_RDI   & REGCODE_MASK)) |
        (1 << (REG_R12   & REGCODE_MASK)) |
        (1 << (REG_R13   & REGCODE_MASK)) |
        (1 << (REG_R14   & REGCODE_MASK)) |
        (1 << (REG_R15   & REGCODE_MASK)) ;
      _cconvPreservedXmm = 
        (1 << (REG_XMM6  & REGCODE_MASK)) |
        (1 << (REG_XMM7  & REGCODE_MASK)) |
        (1 << (REG_XMM8  & REGCODE_MASK)) |
        (1 << (REG_XMM9  & REGCODE_MASK)) |
        (1 << (REG_XMM10 & REGCODE_MASK)) |
        (1 << (REG_XMM11 & REGCODE_MASK)) |
        (1 << (REG_XMM12 & REGCODE_MASK)) |
        (1 << (REG_XMM13 & REGCODE_MASK)) |
        (1 << (REG_XMM14 & REGCODE_MASK)) |
        (1 << (REG_XMM15 & REGCODE_MASK)) ;

      _cconvArgumentsGp[0] = (REG_RCX  & REGCODE_MASK);
      _cconvArgumentsGp[1] = (REG_RDX  & REGCODE_MASK);
      _cconvArgumentsGp[2] = (REG_R8   & REGCODE_MASK);
      _cconvArgumentsGp[3] = (REG_R9   & REGCODE_MASK);
      _cconvArgumentsXmm[0] = (REG_XMM0 & REGCODE_MASK);
      _cconvArgumentsXmm[1] = (REG_XMM1 & REGCODE_MASK);
      _cconvArgumentsXmm[2] = (REG_XMM2 & REGCODE_MASK);
      _cconvArgumentsXmm[3] = (REG_XMM3 & REGCODE_MASK);
      break;

    case CALL_CONV_X64U:
      _cconvPreservedGp =
        (1 << (REG_RBX   & REGCODE_MASK)) |
        (1 << (REG_RSP   & REGCODE_MASK)) |
        (1 << (REG_RBP   & REGCODE_MASK)) |
        (1 << (REG_R12   & REGCODE_MASK)) |
        (1 << (REG_R13   & REGCODE_MASK)) |
        (1 << (REG_R14   & REGCODE_MASK)) |
        (1 << (REG_R15   & REGCODE_MASK)) ;
      _cconvPreservedXmm = 0;

      _cconvArgumentsGp[0] = (REG_RDI  & REGCODE_MASK);
      _cconvArgumentsGp[1] = (REG_RSI  & REGCODE_MASK);
      _cconvArgumentsGp[2] = (REG_RDX  & REGCODE_MASK);
      _cconvArgumentsGp[3] = (REG_RCX  & REGCODE_MASK);
      _cconvArgumentsGp[4] = (REG_R8   & REGCODE_MASK);
      _cconvArgumentsGp[5] = (REG_R9   & REGCODE_MASK);
      _cconvArgumentsXmm[0] = (REG_XMM0 & REGCODE_MASK);
      _cconvArgumentsXmm[1] = (REG_XMM1 & REGCODE_MASK);
      _cconvArgumentsXmm[2] = (REG_XMM2 & REGCODE_MASK);
      _cconvArgumentsXmm[3] = (REG_XMM3 & REGCODE_MASK);
      _cconvArgumentsXmm[4] = (REG_XMM4 & REGCODE_MASK);
      _cconvArgumentsXmm[5] = (REG_XMM5 & REGCODE_MASK);
      _cconvArgumentsXmm[6] = (REG_XMM6 & REGCODE_MASK);
      _cconvArgumentsXmm[7] = (REG_XMM7 & REGCODE_MASK);
      break;

    default:
      // Illegal calling convention.
      ASMJIT_ASSERT(0);
  }
#endif
}

void Function::_setArguments(const UInt32* _args, SysUInt count)
{
  ASMJIT_ASSERT(count <= 32);

  SysInt i;

  SysInt gpnPos = 0;
  SysInt xmmPos = 0;
  SysInt stackOffset = 0;

  UInt32 args[32];
  memcpy(args, _args, count * sizeof(UInt32));

  _variables.clear();

  for (i = 0; i < (SysInt)count; i++)
  {
    Variable* v = _compiler->newObject<Variable>(this, args[i]);
    v->_refCount = 1; // Arguments are never freed or reused.

    // Set variable name only if logger is present.
    if (compiler()->logger() != NULL)
    {
      // Default argument name is argINDEX.
      char name[32];
      sprintf(name, "arg%d", (int)i);
      v->setName(name);
    }

    _variables.append(v);
  }

  _argumentsCount = count;
  if (!_args) return;

#if defined(ASMJIT_X86)
  // ==========================================================================
  // [X86 Calling Conventions]

  // Register arguments (Integer), always left-to-right
  for (i = 0; i != count; i++)
  {
    UInt32 a = args[i];
    if (isIntegerVariable(a) && gpnPos < 32 && _cconvArgumentsGp[gpnPos] != 0xFFFFFFFF)
    {
      UInt8 reg = _cconvArgumentsGp[gpnPos++] | REG_GPN;
      UInt8 size = variableInfo[a].size;
      Variable* v = _variables[i];

      v->setAll(a, size, VARIABLE_STATE_REGISTER, 10, reg, NO_REG, 0);
      v->_changed = true;
      _allocReg(reg, v);

      _state.gp[reg & 0x0F] = v;
      args[i] = VARIABLE_TYPE_NONE;
    }
  }

  // Stack arguments
  bool ltr = _cconvArgumentsDirection == ARGUMENT_DIR_LEFT_TO_RIGHT;
  SysInt istart = ltr ? 0 : (SysInt)count - 1;
  SysInt iend   = ltr ? (SysInt)count : -1;
  SysInt istep  = ltr ? 1 : -1;

  for (i = istart; i != iend; i += istep)
  {
    UInt32 a = args[i];

    if (isIntegerVariable(a))
    {
      UInt8 size = variableInfo[a].size;
      Variable* v = _variables[i];

      stackOffset -= 4;

      v->setAll(a, size, VARIABLE_STATE_MEMORY, 20, NO_REG, NO_REG, stackOffset);
      v->_stackArgument = true;
      args[i] = VARIABLE_TYPE_NONE;
    }
    else if (isFloatArgument(a))
    {
      UInt8 size = variableInfo[a].size;
      stackOffset -= size;

      _variables[i]->setAll(a, size, VARIABLE_STATE_MEMORY, 20, NO_REG, NO_REG, stackOffset);
      _variables[i]->_stackArgument = true;
      args[i] = VARIABLE_TYPE_NONE;
    }
  }
  // ==========================================================================
#else
  // ==========================================================================
  // [X64 Calling Conventions]

  // Windows 64-bit specific
  if (cconv() == CALL_CONV_X64W)
  {
    SysInt max = count < 4 ? count : 4;

    // Register arguments (Integer / FP), always left to right
    for (i = 0; i != max; i++)
    {
      UInt32 a = args[i];
      if (isIntegerVariable(a))
      {
        UInt8 reg = _cconvArgumentsGp[i] | REG_GPN;
        UInt8 size = variableInfo[a].size;
        Variable* v = _variables[i];

        v->setAll(a, size, VARIABLE_STATE_REGISTER, 20, reg, NO_REG, 0);
        v->_changed = true;
        _allocReg(reg, v);

        _state.gp[reg & 0x0F] = v;
        args[i] = VARIABLE_TYPE_NONE;
      }
      else if (isFloatArgument(a))
      {
        UInt8 reg = _cconvArgumentsXmm[i] | REG_XMM;
        UInt8 size = variableInfo[a].size;
        Variable* v = _variables[i];

        v->setAll(a, size, VARIABLE_STATE_REGISTER, 20, reg, NO_REG, 0);
        v->_changed = true;
        _allocReg(reg, v);

        _state.xmm[reg & 0x0F] = v;
        args[i] = VARIABLE_TYPE_NONE;
      }
    }

    // Stack arguments
    for (i = count-1; i != -1; i--)
    {
      UInt32 a = args[i];
      if (isIntegerVariable(a))
      {
        UInt8 size = variableInfo[a].size;
        Variable* v = _variables[i];

        stackOffset -= 8; // Always 8 bytes

        v->setAll(a, size, VARIABLE_STATE_MEMORY, 20, NO_REG, NO_REG, stackOffset);
        v->_stackArgument = true;
        args[i] = VARIABLE_TYPE_NONE;
      }
      else if (isFloatArgument(a))
      {
        UInt8 size = variableInfo[a].size;
        Variable* v = _variables[i];

        stackOffset -= size;

        v->setAll(a, size, VARIABLE_STATE_MEMORY, 20, NO_REG, NO_REG, stackOffset);
        v->_stackArgument = true;
        args[i] = VARIABLE_TYPE_NONE;
      }
    }

    // 32 bytes shadow space (X64W calling convention specific).
    stackOffset -= 4 * 8;
  }
  // All others
  else
  {
    // Register arguments (Integer), always left to right
    for (i = 0; i != count; i++)
    {
      UInt32 a = args[i];
      if (isIntegerVariable(a) && gpnPos < 32 && _cconvArgumentsGp[gpnPos] != 0xFFFFFFFF)
      {
        UInt8 reg = _cconvArgumentsGp[gpnPos++] | REG_GPN;
        UInt8 size = variableInfo[a].size;
        Variable* v = _variables[i];

        v->setAll(a, size, VARIABLE_STATE_REGISTER, 20, reg, NO_REG, 0);
        v->_changed = true;
        _allocReg(reg, v);

        _state.gp[reg & 0x0F] = v;
        args[i] = VARIABLE_TYPE_NONE;
      }
    }

    // Register arguments (FP), always left to right
    for (i = 0; i != count; i++)
    {
      UInt32 a = args[i];
      if (isFloatArgument(a))
      {
        UInt8 reg = _cconvArgumentsXmm[xmmPos++] | REG_XMM;
        UInt8 size = variableInfo[a].size;
        Variable* v = _variables[i];

        v->setAll(a, size, VARIABLE_STATE_REGISTER, 20, reg, NO_REG, 0);
        v->_changed = true;
        _allocReg(reg, v);

        _state.xmm[reg & 0x0F] = v;
        args[i] = VARIABLE_TYPE_NONE;
      }
    }

    // Stack arguments
    for (i = count-1; i != -1; i--)
    {
      UInt32 a = args[i];
      if (isIntegerVariable(a))
      {
        UInt8 size = variableInfo[a].size;
        Variable* v = _variables[i];

        stackOffset -= 8;

        v->setAll(a, size, VARIABLE_STATE_MEMORY, 20, NO_REG, NO_REG, stackOffset);
        v->_stackArgument = true;
        args[i] = VARIABLE_TYPE_NONE;
      }
      else if (isFloatArgument(a))
      {
        UInt8 size = variableInfo[a].size;
        Variable* v = _variables[i];

        stackOffset -= size;

        v->setAll(a, size, VARIABLE_STATE_MEMORY, 20, NO_REG, NO_REG, stackOffset);
        v->_stackArgument = true;
        args[i] = VARIABLE_TYPE_NONE;
      }
    }
  }
  // ==========================================================================
#endif // ASMJIT_X86, ASMJIT_X64

  // Modify stack offset (all function parameters will be in positive stack
  // offset that is never zero).
  for (i = 0; i < (SysInt)count; i++)
  {
    _variables[i]->_stackOffset += sizeof(SysInt) - stackOffset;
  }

  _argumentsStackSize = (UInt32)(-stackOffset);
}

Variable* Function::newVariable(UInt8 type, UInt8 priority, UInt8 preferredRegisterCode)
{
  Variable* v;

  // First look to unused variables
  SysUInt i;
  for (i = 0; i < _variables.length(); i++)
  {
    v = _variables[i];
    if (v->refCount() == 0 && v->reusable() && v->type() == type) 
    {
      v->_preferredRegisterCode = preferredRegisterCode;
      v->_priority = priority;
      return v;
    }
  }

  // If there is no variable that can be reused, create new one.
  v = compiler()->newObject<Variable>(this, type);
  v->_preferredRegisterCode = preferredRegisterCode;
  v->_priority = priority;

  // Set variable name only if logger is present.
  if (compiler()->logger())
  {
    // Default variable name is varINDEX.
    char name[32];
    sprintf(name, "var%d", (int)i);
    v->setName(name);
  }

  _variables.append(v);

  // Alloc register if priority is zero
  if (priority == 0) alloc(v);

  return v;
}

bool Function::alloc(Variable* v, UInt8 mode, UInt8 preferredRegisterCode)
{
  ASMJIT_ASSERT(compiler() == v->compiler());

  UInt32 i;

  // Preferred register code
  UInt8 pref = (preferredRegisterCode != NO_REG) 
    ? preferredRegisterCode 
    : v->_preferredRegisterCode;

  // Last register code
  UInt8 home = v->homeRegisterCode();

  // New register code.
  UInt8 code = NO_REG;

  // Spill candidate.
  Variable* spillCandidate = NULL;

  // --------------------------------------------------------------------------
  // [Already Allocated]
  // --------------------------------------------------------------------------

  // Go away if variable is already allocated
  if (v->state() == VARIABLE_STATE_REGISTER)
  {
    UInt8 oldIndex = v->registerCode() & 0xF;
    UInt8 newIndex = pref & 0xF;

    // Preferred register is none or same as currently allocated one, this is
    // best case
    if (pref == NO_REG || oldIndex == newIndex)
    {
      _postAlloc(v, mode);
      return true;
    }

    if (isIntegerVariable(v->type()))
    {
      Variable* other = _state.gp[newIndex];

      if (other)
      {
        if (other->priority() != 0)
        {
          // Exchange instead of spill/alloc.
          _exchangeGp(v, mode, other);
        }
        else
        {
          // TODO: Error handling, finished in See AsmJit-1.0
          ASMJIT_ASSERT(0);
          return false;
        }
      }
      else
      {
        _moveGp(v, newIndex);
      }

      _postAlloc(v, mode);
      return true;
    }
  }

  // --------------------------------------------------------------------------
  // [Find Unused GP]
  // --------------------------------------------------------------------------
  UInt8 clazz = variableInfo[v->type()].clazz;

  if (clazz & VariableInfo::CLASS_GP)
  {
    // preferred register
    if (pref != NO_REG)
    {
      // esp/rsp can't be never allocated
      ASMJIT_ASSERT((pref & REGCODE_MASK) != RID_ESP);

      if ((_usedGpRegisters & (1U << (pref & REGCODE_MASK))) == 0)
      {
        code = pref;
      }
      else
      {
        // Spill register we need
        spillCandidate = _state.gp[pref & REGCODE_MASK];

        // Can't alloc register that is manually masked (marked as used but
        // no variable exists). Yeah, this is possible, but not recommended.
        if (spillCandidate == NULL)
        {
          // TODO: Error handling
          ASMJIT_ASSERT(0);
        }

        // Jump to spill part of allocation
        goto L_spill;
      }
    }

    // Home register code
    if (code == NO_REG && home != NO_REG)
    {
      if ((_usedGpRegisters & (1U << (home & REGCODE_MASK))) == 0)
      {
        code = home;
      }
    }

    // We start from 1, because EAX/RAX register is sometimes explicitly
    // needed. So we trying to prevent register reallocation.
    if (code == NO_REG)
    {
      for (i = 1; i < NUM_REGS; i++)
      {
        UInt32 mask = (1U << i);
        if ((_usedGpRegisters & mask) == 0 && (i != RID_EBP || allocableEbp()) && i != RID_ESP)
        {
          // Convenience to alloc registers from positions 0 to 15
          if (code != NO_REG && (_cconvPreservedGp & mask) == 1) continue;

          if (v->type() == VARIABLE_TYPE_INT32)
            code = i | REG_GPD;
          else
            code = i | REG_GPQ;

          // If current register is preserved, we should try to find different
          // one that is not. This can save one push / pop in prolog / epilog.
          if ((_cconvPreservedGp & mask) == 0) break;
        }
      }
    }

    // If not found, try EAX/RAX
    if (code == NO_REG && (_usedGpRegisters & 1) == 0)
    {
      if (v->type() == VARIABLE_TYPE_INT32)
        code = RID_EAX | REG_GPD;
      else
        code = RID_EAX | REG_GPQ;
    }
  }

  // --------------------------------------------------------------------------
  // [Find Unused MM]
  // --------------------------------------------------------------------------

  else if (clazz & VariableInfo::CLASS_MM)
  {
    // preferred register
    if (pref != NO_REG)
    {
      if ((_usedMmRegisters & (1U << (pref & 0x7))) == 0)
      {
        code = pref;
      }
      else
      {
        // Spill register we need
        spillCandidate = _state.mm[pref & REGCODE_MASK];

        // Can't alloc register that is manually masked (marked as used but
        // no variable exists). Yeah, this is possible, but not recommended.
        if (spillCandidate == NULL)
        {
          // TODO: Error handling
          ASMJIT_ASSERT(0);
        }

        // Jump to spill part of allocation
        goto L_spill;
      }
    }

    // Home register code
    if (code == NO_REG && home != NO_REG)
    {
      if ((_usedMmRegisters & (1U << (home & REGCODE_MASK))) == 0)
      {
        code = home;
      }
    }

    if (code == NO_REG)
    {
      for (i = 0; i < 8; i++)
      {
        UInt32 mask = (1U << i);
        if ((_usedMmRegisters & mask) == 0)
        {
          code = i | REG_MM;
          break;
        }
      }
    }
  }

  // --------------------------------------------------------------------------
  // [Find Unused XMM]
  // --------------------------------------------------------------------------

  else if (clazz & VariableInfo::CLASS_XMM)
  {
    // preferred register
    if (pref != NO_REG)
    {
      if ((_usedXmmRegisters & (1U << (pref & REGCODE_MASK))) == 0)
      {
        code = pref;
      }
      else
      {
        // Spill register we need
        spillCandidate = _state.xmm[pref & REGCODE_MASK];

        // Can't alloc register that is manually masked (marked as used but
        // no variable exists). Yeah, this is possible, but not recommended.
        if (spillCandidate == NULL)
        {
          // TODO: Error handling
          ASMJIT_ASSERT(0);
        }

        // Jump to spill part of allocation
        goto L_spill;
      }
    }

    // Home register code
    if (code == NO_REG && home != NO_REG)
    {
      if ((_usedXmmRegisters & (1U << (home & REGCODE_MASK))) == 0)
      {
        code = home;
      }
    }

    if (code == NO_REG)
    {
      for (i = 0; i < NUM_REGS; i++)
      {
        UInt32 mask = (1U << i);
        if ((_usedXmmRegisters & mask) == 0)
        {
          // Convenience to alloc registers from positions 0 to 15
          if (code != NO_REG && (_cconvPreservedXmm & mask) == 1) continue;

          code = i | REG_XMM;

          // If current register is preserved, we should try to find different
          // one that is not. This can save one push / pop in prolog / epilog.
          if ((_cconvPreservedXmm & mask) == 0) break;
        }
      }
    }
  }

  // --------------------------------------------------------------------------
  // [Spill]
  // --------------------------------------------------------------------------

  // If register is still not found, spill some variable
  if (code == NO_REG)
  {
    if (!spillCandidate) spillCandidate = _getSpillCandidate(v->type());

    // Spill candidate not found.
    if (!spillCandidate)
    {
      // TODO: Error handling
      ASMJIT_ASSERT(0);
    }

L_spill:

    // Prevented variables can't be spilled. _getSpillCandidate() never returns
    // prevented variables, but when jumping to L_spill it can happen.
    if (isPrevented(spillCandidate))
    {
      // TODO: Error handling
      ASMJIT_ASSERT(0);
    }

    // Can't alloc register that is used in other variable and its priority is
    // zero. Zero priority variable can't be spilled.
    if (spillCandidate->priority() == 0)
    {
      // TODO: Error handling
      ASMJIT_ASSERT(0);
    }

    code = spillCandidate->registerCode();
    spill(spillCandidate);
  }

  // --------------------------------------------------------------------------
  // [Finish]
  // --------------------------------------------------------------------------

  _allocAs(v, mode, code);
  _postAlloc(v, mode);
  return true;
}

bool Function::spill(Variable* v)
{
  ASMJIT_ASSERT(compiler() == v->compiler());

  removePrevented(v);

  if (v->state() == VARIABLE_STATE_UNUSED) return true;
  if (v->state() == VARIABLE_STATE_MEMORY) return true;

  if (v->state() == VARIABLE_STATE_REGISTER)
  {
    if (v->priority() == 0)
    {
      return false;
    }

    if (v->changed())
    {
      if (v->isCustom())
      {
        if (v->_spillFn) v->_spillFn(v);
      }
      else
      {
        // Inline comment - Spill variable.
        if (compiler()->logger())
        {
          char comment[128];
          sprintf(comment, "spill %s", v->name());
          compiler()->_inlineComment(comment);
        }

        switch (v->type())
        {
          case VARIABLE_TYPE_INT32:
            compiler()->mov(*v->_memoryOperand, mk_gpd(v->registerCode()));
            break;

#if defined(ASMJIT_X64)
          case VARIABLE_TYPE_INT64:
            compiler()->mov(*v->_memoryOperand, mk_gpq(v->registerCode()));
            break;
#endif // ASMJIT_X64

          case VARIABLE_TYPE_X87_FLOAT:
            // TODO: NOT IMPLEMENTED
            break;

          case VARIABLE_TYPE_X87_DOUBLE:
            // TODO: NOT IMPLEMENTED
            break;

          case VARIABLE_TYPE_XMM_FLOAT:
            compiler()->movss(*v->_memoryOperand, mk_xmm(v->registerCode()));
            break;

          case VARIABLE_TYPE_XMM_DOUBLE:
            compiler()->movsd(*v->_memoryOperand, mk_xmm(v->registerCode()));
            break;

          case VARIABLE_TYPE_XMM_FLOAT_4:
            // Alignment is not guaranted for naked functions in 32 bit mode
            if (naked())
              compiler()->movups(*v->_memoryOperand, mk_xmm(v->registerCode()));
            else
              compiler()->movaps(*v->_memoryOperand, mk_xmm(v->registerCode()));
            break;

          case VARIABLE_TYPE_XMM_DOUBLE_2:
            // Alignment is not guaranted for naked functions in 32 bit mode
            if (naked())
              compiler()->movupd(*v->_memoryOperand, mk_xmm(v->registerCode()));
            else
              compiler()->movapd(*v->_memoryOperand, mk_xmm(v->registerCode()));
            break;

          case VARIABLE_TYPE_MM:
            compiler()->movq(*v->_memoryOperand, mk_mm(v->registerCode()));
            break;

          case VARIABLE_TYPE_XMM:
            // Alignment is not guaranted for naked functions in 32 bit mode
            if (naked())
              compiler()->movdqu(*v->_memoryOperand, mk_xmm(v->registerCode()));
            else
              compiler()->movdqa(*v->_memoryOperand, mk_xmm(v->registerCode()));
            break;
        }

        // Inline comment - Spill variable.
        if (compiler()->logger())
        {
          compiler()->_inlineComment(NULL, 0);
        }

        v->_memoryAccessCount++;
        v->_globalMemoryAccessCount++;
      }

      v->setChanged(false);
    }

    _freeReg(v->registerCode());
    v->_registerCode = NO_REG;

    v->_state = VARIABLE_STATE_MEMORY;
    v->_spillCount++;
    v->_globalSpillCount++;
  }

  return true;
}

void Function::unuse(Variable* v)
{
  ASMJIT_ASSERT(compiler() == v->compiler());
  if (v->state() == VARIABLE_STATE_UNUSED) return;

  if (v->state() == VARIABLE_STATE_REGISTER)
  {
    _freeReg(v->registerCode());
    v->_registerCode = NO_REG;
  }

  v->_state = VARIABLE_STATE_UNUSED;

  v->_spillCount = 0;
  v->_registerAccessCount = 0;
  v->_memoryAccessCount = 0;

  v->_lifeId++;

  v->_preferredRegisterCode = NO_REG;
  v->_homeRegisterCode = NO_REG;
  v->_priority = 10;
  v->_changed = 0;

  v->_allocFn = NULL;
  v->_spillFn = NULL;
  v->_dataPtr = NULL;
  v->_dataInt = 0;
}

void Function::spillAll()
{
  _spillAll(0, 16+8+16);
}

void Function::spillAllGp()
{
  _spillAll(0, 16);
}

void Function::spillAllMm()
{
  _spillAll(16, 8);
}

void Function::spillAllXmm()
{
  _spillAll(16+8, 16);
}

void Function::_spillAll(SysUInt start, SysUInt end)
{
  SysUInt i;

  for (i = start; i < end; i++)
  {
    Variable* v = _state.regs[i];
    if (v) spill(v);
  }
}

void Function::spillRegister(const BaseReg& reg)
{
  SysUInt i = reg.index();
  Variable *v;

  switch (reg.type())
  {
    case REG_GPB:
    case REG_GPW:
    case REG_GPD:
    case REG_GPQ:
      v = _state.gp[i];
      break;
    case REG_MM:
      v = _state.mm[i];
      break;
    case REG_XMM:
      v = _state.xmm[i];
      break;
    default:
      return;
  }

  if (v) spill(v);
}

static SysInt getFreeRegs(UInt32 regs, SysUInt max)
{
  SysUInt n = 0;
  SysUInt i;
  UInt32 mask = 1;

  for (i = 0; i < max; i++, mask <<= 1)
  {
    if ((regs & mask) == 0) n++;
  }
  return n;
}

SysInt Function::numFreeGp() const
{
  SysInt n = getFreeRegs(_usedGpRegisters, NUM_REGS);

  if ((_usedGpRegisters & (1 << RID_ESP)) == 0) n--;
  if ((_usedGpRegisters & (1 << RID_EBP)) == 0 && !allocableEbp()) n--;

  return n;
}

SysInt Function::numFreeMm() const
{
  return getFreeRegs(_usedMmRegisters, 8);
}

SysInt Function::numFreeXmm() const
{
  return getFreeRegs(_usedXmmRegisters, NUM_REGS);
}

bool Function::isPrevented(Variable* v)
{
  return _usePrevention && _prevented.indexOf(v) != (SysUInt)-1;
}

void Function::addPrevented(Variable* v)
{
  if (!_usePrevention) return;

  SysUInt i = _prevented.indexOf(v);
  if (i == (SysUInt)-1) _prevented.append(v);
}

void Function::removePrevented(Variable* v)
{
  if (!_usePrevention) return;

  SysUInt i = _prevented.indexOf(v);
  if (i != (SysUInt)-1) _prevented.removeAt(i);
}

void Function::clearPrevented()
{
  _prevented.clear();
}

static UInt32 getSpillScore(Variable* v)
{
  if (v->priority() == 0) return 0;

  // Priority is main factor.
  UInt32 p = ((UInt32)v->priority() << 24) - ((1U << 24) / 2);

  // Each register access means lower probability of spilling
  p -= (UInt32)v->registerAccessCount();

  // Each memory access means higher probability of spilling
  p += (UInt32)v->memoryAccessCount();

  return p;
}

Variable* Function::_getSpillCandidate(UInt8 type)
{
  Variable* candidate = NULL;
  Variable* v;
  SysUInt i, len = _variables.length();

  UInt32 candidateScore = 0;
  UInt32 variableScore;

  UInt8 clazz = variableInfo[type].clazz;

  if (clazz & VariableInfo::CLASS_GP)
  {
    for (i = 0; i < len; i++)
    {
      v = _variables[i];
      if ((v->type() == VARIABLE_TYPE_INT32 || v->type() == VARIABLE_TYPE_INT64) &&
          (v->state() == VARIABLE_STATE_REGISTER && v->priority() > 0) &&
          (!isPrevented(v)))
      {
        variableScore = getSpillScore(v);
        if (variableScore > candidateScore) { candidateScore = variableScore; candidate = v; }
      }
    }
  }
  else if (clazz & VariableInfo::CLASS_X87)
  {
    // TODO: Not implemented.
  }
  else if (clazz & VariableInfo::CLASS_MM)
  {
    for (i = 0; i < len; i++)
    {
      v = _variables[i];
      if ((v->type() == VARIABLE_TYPE_MM) &&
          (v->state() == VARIABLE_STATE_REGISTER && v->priority() > 0) &&
          (!isPrevented(v)))
      {
        variableScore = getSpillScore(v);
        if (variableScore > candidateScore) { candidateScore = variableScore; candidate = v; }
      }
    }
  }
  else if (clazz & VariableInfo::CLASS_XMM)
  {
    for (i = 0; i < len; i++)
    {
      v = _variables[i];
      if ((v->type() == VARIABLE_TYPE_XMM) &&
          (v->state() == VARIABLE_STATE_REGISTER && v->priority() > 0) &&
          (!isPrevented(v)))
      {
        variableScore = getSpillScore(v);
        if (variableScore > candidateScore) { candidateScore = variableScore; candidate = v; }
      }
    }
  }

  return candidate;
}

void Function::_allocAs(Variable* v, UInt8 mode, UInt32 code)
{
  // true if we must copy content from memory to register before we can use it
  bool copy = (v->state() == VARIABLE_STATE_MEMORY);
  UInt8 old = v->_registerCode;

  v->_state = VARIABLE_STATE_REGISTER;
  v->_registerCode = code;

  _allocReg(code, v);

  // Inline comment - Alloc variable.
  if (compiler()->logger())
  {
    char comment[128];
    sprintf(comment, "alloc %s", v->name());
    compiler()->_inlineComment(comment);
  }

  if (v->isCustom())
  {
    if (v->_allocFn && mode != VARIABLE_ALLOC_WRITE) v->_allocFn(v);
  }
  else if (copy && mode != VARIABLE_ALLOC_WRITE)
  {
    switch (v->type())
    {
      case VARIABLE_TYPE_INT32:
      {
        Register dst = mk_gpd(v->_registerCode);
        if (old != NO_REG)
          compiler()->mov(dst, mk_gpd(old));
        else
          compiler()->mov(dst, *v->_memoryOperand);        
        break;
      }

#if defined(ASMJIT_X64)
      case VARIABLE_TYPE_INT64:
      {
        Register dst = mk_gpq(v->_registerCode);
        if (old != NO_REG)
          compiler()->mov(dst, mk_gpq(old));
        else
          compiler()->mov(dst, *v->_memoryOperand);
        break;
      }
#endif // ASMJIT_X64

      case VARIABLE_TYPE_X87_FLOAT:
      {
        // TODO: NOT IMPLEMENTED
        break;
      }

      case VARIABLE_TYPE_X87_DOUBLE:
      {
        // TODO: NOT IMPLEMENTED
        break;
      }

      case VARIABLE_TYPE_XMM_FLOAT:
      {
        XMMRegister dst = mk_xmm(v->_registerCode);
        if (old != NO_REG)
          compiler()->movss(dst, mk_xmm(old));
        else
          compiler()->movss(dst, *v->_memoryOperand);
        break;
      }

      case VARIABLE_TYPE_XMM_DOUBLE:
      {
        XMMRegister dst = mk_xmm(v->_registerCode);
        if (old != NO_REG)
          compiler()->movsd(dst, mk_xmm(old));
        else
          compiler()->movsd(dst, *v->_memoryOperand);
        break;
      }

      case VARIABLE_TYPE_XMM_FLOAT_4:
      {
        XMMRegister dst = mk_xmm(v->_registerCode);
        if (old != NO_REG)
          compiler()->movaps(dst, mk_xmm(old));
        // Alignment is not guaranted for naked functions in 32 bit mode
        // FIXME: And what about 64 bit mode ?
        else if (naked())
          compiler()->movups(dst, *v->_memoryOperand);
        else
          compiler()->movaps(dst, *v->_memoryOperand);
        break;
      }

      case VARIABLE_TYPE_XMM_DOUBLE_2:
      {
        XMMRegister dst = mk_xmm(v->_registerCode);
        if (old != NO_REG)
          compiler()->movapd(dst, mk_xmm(old));
        // Alignment is not guaranted for naked functions in 32 bit mode
        // FIXME: And what about 64 bit mode ?
        else if (naked())
          compiler()->movupd(dst, *v->_memoryOperand);
        else
          compiler()->movapd(dst, *v->_memoryOperand);
        break;
      }

      case VARIABLE_TYPE_MM:
      {
        MMRegister dst = mk_mm(v->_registerCode);
        if (old != NO_REG)
          compiler()->movq(dst, mk_mm(old));
        else
          compiler()->movq(dst, *v->_memoryOperand);
        break;
      }

      case VARIABLE_TYPE_XMM:
      {
        XMMRegister dst = mk_xmm(v->_registerCode);
        if (old != NO_REG)
          compiler()->movdqa(dst, mk_xmm(old));
        // Alignment is not guaranted for naked functions in 32 bit mode
        // FIXME: And what about 64 bit mode ?
        else if (naked())
          compiler()->movdqu(dst, *v->_memoryOperand);
        else
          compiler()->movdqa(dst, *v->_memoryOperand);
        break;
      }
    }

    if (old != NO_REG)
    {
      v->_registerAccessCount++;
      v->_globalRegisterAccessCount++;
    }
    else
    {
      v->_memoryAccessCount++;
      v->_globalMemoryAccessCount++;
    }
  }

  // Inline comment - Alloc variable.
  if (compiler()->logger())
  {
    compiler()->_inlineComment(NULL, 0);
  }
}

void Function::_allocReg(UInt8 code, Variable* v)
{
  UInt32 type = code & REGTYPE_MASK;
  UInt32 mask = 1U << (code & REGCODE_MASK);

  switch (type)
  {
    case REG_GPB:
    case REG_GPW:
    case REG_GPD:
    case REG_GPQ:
      useGpRegisters(mask);
      modifyGpRegisters(mask);
      _state.gp[code & 0x0F] = v;
      break;
    case REG_MM:
      useMmRegisters(mask);
      modifyMmRegisters(mask);
      _state.mm[code & 0x0F] = v;
      break;
    case REG_XMM:
      useXmmRegisters(mask);
      modifyXmmRegisters(mask);
      _state.xmm[code & 0x0F] = v;
      break;
  }

  // Set home code, Compiler is able to reuse it again.
  v->_homeRegisterCode = code;
}

void Function::_freeReg(UInt8 code)
{
  UInt32 type = code & REGTYPE_MASK;
  UInt32 mask = 1U << (code & REGCODE_MASK);

  switch (type)
  {
    case REG_GPB:
    case REG_GPW:
    case REG_GPD:
    case REG_GPQ:
      unuseGpRegisters(mask);
      _state.gp[code & 0x0F] = NULL;
      break;
    case REG_MM:
      unuseMmRegisters(mask);
      _state.mm[code & 0x0F] = NULL;
      break;
    case REG_XMM:
      unuseXmmRegisters(mask);
      _state.xmm[code & 0x0F] = NULL;
      break;
  }
}

void Function::_moveGp(Variable* v, UInt8 code)
{
  ASMJIT_ASSERT(v->state() == VARIABLE_STATE_REGISTER);

  UInt8 dstCode = code;
  UInt8 srcCode = v->registerCode();

  UInt8 dstIndex = dstCode & REGCODE_MASK;
  UInt8 srcIndex = srcCode & REGCODE_MASK;

  Register dstReg = mk_gpn(dstIndex);
  Register srcReg = mk_gpn(srcIndex);

  compiler()->mov(dstReg, srcReg);

  v->_registerCode = (code & REGTYPE_MASK) | dstCode;

  _state.gp[dstCode] = v;
  _state.gp[srcCode] = NULL;

  // Statistics.
  v->_registerAccessCount++;
  v->_globalRegisterAccessCount++;
}

void Function::_exchangeGp(Variable* v, UInt8 mode, Variable* other)
{
  ASMJIT_ASSERT(v->state() == VARIABLE_STATE_REGISTER);
  ASMJIT_ASSERT(other->state() == VARIABLE_STATE_REGISTER);

  UInt8 code1 = v->registerCode();
  UInt8 code2 = other->registerCode();

  UInt8 type1 = code1 & REGTYPE_MASK;
  UInt8 type2 = code2 & REGTYPE_MASK;

  UInt8 index1 = code1 & REGCODE_MASK;
  UInt8 index2 = code2 & REGCODE_MASK;

  // Make sure that register classes match, we can't exchange for example
  // general purpose register with sse one
  ASMJIT_ASSERT(type1 <= REG_GPQ && type2 <= REG_GPQ);

  Register reg1 = mk_gpn(index1);
  Register reg2 = mk_gpn(index2);

  if (mode == VARIABLE_ALLOC_WRITE)
  {
    // If we are completely rewriting variable, we can use only mov to save
    // 'other'. This can be faster or not than using xchg.
    compiler()->mov(reg1, reg2);
  }
  else
  {
    // Standard exchange instruction supported by x86 architecture.
    compiler()->xchg(reg1, reg2);
  }

  // Swap registers
  v->_registerCode = index2 | type1;
  other->_registerCode = index1 | type2;

  // Update state
  _state.gp[index1] = other;
  _state.gp[index2] = v;

  // Statistics
  v->_registerAccessCount++;
  v->_globalRegisterAccessCount++;

  other->_registerAccessCount++;
  other->_globalRegisterAccessCount++;
}

void Function::_postAlloc(Variable* v, UInt8 mode)
{
  // Mark variable as changed if needed
  if ((mode & VARIABLE_ALLOC_WRITE) != 0) v->_changed = true;

  // Add variable to prevented ones. This will be cleared when instruction
  // is emitted.
  addPrevented(v);
}

SysInt Function::countOfGpRegistersToBeSaved() const ASMJIT_NOTHROW
{
  SysInt count = 0;

  for (int i = 0; i < NUM_REGS; i++)
  {
    if ((modifiedGpRegisters() & (1U << i)) && (cconvPreservedGp() & (1U << i)) && (i != (REG_NSP & REGCODE_MASK)) )
    {
      count++;
    }
  }
  return count;
}

SysInt Function::countOfXmmRegistersToBeSaved() const ASMJIT_NOTHROW
{
  SysInt count = 0;

  for (int i = 0; i < NUM_REGS; i++)
  {
    if ((modifiedXmmRegisters() & (1U << i)) && (cconvPreservedXmm() & (1U << i)))
    {
      count++;
    }
  }

  return count;
}

State *Function::saveState()
{
  State* s = compiler()->newObject<State>(this);
  State::saveFunctionState(&s->_data, this);
  return s;
}

void Function::restoreState(State* s)
{
  ASMJIT_ASSERT(s->_function == this);

  // Stop prevention
  _usePrevention = false;

  // make local copy of function state
  State::Data f_d;
  State::Data& s_d = s->_data;

  State::saveFunctionState(&f_d, this);

  SysInt base;
  SysInt i;

  // Spill registers
  for (base = 0, i = 0; i < 16+8+16; i++)
  {
    if (i == 16 || i == 24) base = i;

    State::Entry* from = &f_d.regs[i];
    State::Entry* to   = &s_d.regs[i];

    Variable* from_v = from->v;
    Variable* to_v = to->v;

    if (from_v != to_v)
    {
      UInt8 regIndex = (UInt8)(i - base);

      // Spill register
      if (from_v != NULL) 
      {
        // Here is important step. It can happen that variable that was saved
        // in state currently not exists. We can check for it by comparing
        // saved lifeId with current variable lifeIf. If IDs are different,
        // variables not match. Another optimization is that we will spill 
        // variable only if it's used in context we need. If it's unused, there
        // is no reason to save it on the stack.
        if (from->lifeId != from_v->lifeId() || from_v->state() == VARIABLE_STATE_UNUSED)
        {
          // Optimization, do not spill it, we can simply abandon it
          _freeReg(getVariableRegisterCode(from_v->type(), regIndex));

          // TODO: Is this right way? We spilled variable manually, but I'm
          // not sure if I can set its state to MEMORY.

          // This will prevent to reset unused variable to be memory variable.
          if (from_v->state() == VARIABLE_STATE_REGISTER)
          {
            from_v->_state = VARIABLE_STATE_MEMORY;
          }
        }
        else
        {
          // Variables match, do normal spill
          spill(from_v);
        }
      }
    }
  }

  // Alloc registers
  for (base = 0, i = 0; i < 16+8+16; i++)
  {
    if (i == 16 || i == 24) base = i;

    State::Entry* from = &f_d.regs[i];
    State::Entry* to   = &s_d.regs[i];

    Variable* from_v = from->v;
    Variable* to_v = to->v;

    if (from_v != to_v)
    {
      UInt8 regIndex = (UInt8)(i - base);

      // Alloc register
      if (to_v != NULL) 
      {
        UInt8 code = getVariableRegisterCode(to_v->type(), regIndex);
        _allocAs(to_v, VARIABLE_ALLOC_READ, code);
      }
    }

    if (to_v)
    {
      to_v->_changed = to->changed;
    }
  }

  // Update masks
  _usedGpRegisters  = s->_data.usedGpRegisters;
  _usedMmRegisters  = s->_data.usedMmRegisters;
  _usedXmmRegisters = s->_data.usedXmmRegisters;

  // Restore and clear prevention
  _usePrevention = false;
  clearPrevented();
}

void Function::setState(State* s)
{
  ASMJIT_ASSERT(s->_function == this);

  for (SysUInt i = 0; i < 16+8+16; i++)
  {
    Variable* old = _state.regs[i];
    Variable* v = s->_data.regs[i].v;

    // This method is dirrerent to restoreState(), because we are not spilling
    // and allocating registers. We need to actualize all variables that was
    // changed by setState(). This means allocated and spilled variables. This
    // is reason why we are modifiyng 'old'
    if (v != old && old)
    {
      if (old->state() == VARIABLE_STATE_REGISTER)
      {
        old->_state = VARIABLE_STATE_MEMORY;
        old->_registerCode = NO_REG;
        old->_changed = false;
      }
    }

    if (v)
    {
      v->_state = s->_data.regs[i].state;
      v->_changed = s->_data.regs[i].changed;
    }

    _state.regs[i] = v;
  }

  // Update masks
  _usedGpRegisters  = s->_data.usedGpRegisters;
  _usedMmRegisters  = s->_data.usedMmRegisters;
  _usedXmmRegisters = s->_data.usedXmmRegisters;

  // Clear prevention
  s->_function->clearPrevented();
}

void Function::_jmpAndRestore(Compiler* c, Label* label)
{
  JumpAndRestore* jr = (JumpAndRestore*)label->_compilerData;
  Function* f = jr->from->_function;

  // Save internal state (we don't want to modify it)
  State backup(c, f);
  State::saveFunctionState(&backup._data, f);

  do {
    // Working variables we need
    State* from = jr->from;
    State* to = jr->to;

    bool isJmp = jr->instruction->code() == INST_JMP;
    bool modifiedState;

    // Emit code to the end (need to save old position) or if instructions is
    // simple jmp()( we can inline state restore before it.
    Emittable* old = c->setCurrentEmittable(isJmp ? jr->instruction->prev() : c->lastEmittable());
    Emittable* first = c->currentEmittable();

    f->setState(from);
    f->restoreState(to);

    Emittable* last = c->currentEmittable();
    modifiedState = old != last;

    // If state was modified and it isn't a jmp(), redirect jump
    if (modifiedState && !isJmp)
    {
      Label* L_block = c->newLabel();

      // Bind label to start of restore block
      c->setCurrentEmittable(first);
      c->align(sizeof(SysInt));
      c->bind(L_block);

      // Jump back from end of the block
      c->setCurrentEmittable(last);
      c->jmp(label);

      // Patch instruction jump target to our new label
      jr->instruction->_o[0] = L_block;
    }

    // Set pointer back
    c->setCurrentEmittable(old);

    // Next JumpAndRestore record
    jr = jr->next;
  } while (jr);

  // Clear data, this is not longer needed
  label->_compilerData = NULL;

  // Restore internal state
  f->setState(&backup);
}

// ============================================================================
// [AsmJit::Prolog]
// ============================================================================

static inline SysInt alignTo16Bytes(SysInt x)
{
  return (x + 15) & ~15;
}

static SysInt getStackSize(Function* f, SysInt stackAdjust)
{
  // Get stack size needed for store all variables and to save all used
  // registers. AlignedStackSize is stack size adjusted for aligning.
  SysInt stackSize = 
    alignTo16Bytes(f->variablesStackSize()) + f->prologEpilogStackSize();

#if defined(ASMJIT_X86)
  SysInt stackAlignment = f->stackAlignmentSize();
#else
  SysInt stackAlignment = 16;
#endif

  if (stackAlignment)
  {
    stackSize = (stackSize + stackAlignment - 1) & ~(stackAlignment - 1);
  }

  return stackSize;
}

Prolog::Prolog(Compiler* c, Function* f) ASMJIT_NOTHROW :
  Emittable(c, EMITTABLE_PROLOGUE), 
  _function(f)
{
}

Prolog::~Prolog() ASMJIT_NOTHROW
{
}

void Prolog::emit(Assembler& a)
{
  Function* f = function();
  ASMJIT_ASSERT(f);

  // In 64-bit mode the stack is aligned to 16 bytes by default.
  bool isStackAlignedTo16Bytes = sizeof(SysInt) == 8;

  // How many bytes to add to stack to make it aligned to 16 bytes.
  SysInt stackAdjust = (f->naked()) ? ((sizeof(SysInt) == 8) ? 8 : 12) : ((sizeof(SysInt) == 8) ? 0 : 8);

  // Calculate stack size with stack adjustment. This will give us proper
  // count of bytes to subtract from esp/rsp.
  SysInt stackSize = getStackSize(f, stackAdjust);
  SysInt stackSubtract = stackSize;

  int i;

  // Emit prolog (but don't do it if function is set to be naked).
  //
  // Also see the stackAdjust variable. If function is naked (so prolog and
  // epilog will not contain "push ebp" and "mov ebp, esp", we need to adjust
  // stack by 8 bytes in 64-bit mode (this will give us that stack will remain
  // aligned to 16 bytes).
  if (!f->naked())
  {
    a.push(nbp);
    a.mov(nbp, nsp);
  }

  // Save GP registers using PUSH/POP.
  if (f->prologEpilogPushPop())
  {
    for (i = 0; i < NUM_REGS; i++)
    {
      if ((f->modifiedGpRegisters() & (1U << i)) && (f->cconvPreservedGp() & (1U << i)) && (i != (REG_NSP & REGCODE_MASK)) )
      {
        a.push(mk_gpn(i));
      }
    }
    stackSubtract -= f->countOfGpRegistersToBeSaved() * sizeof(SysInt);
  }

  if (!f->naked())
  {
    if (stackSubtract) a.sub(nsp, stackSubtract);

#if defined(ASMJIT_X86)
    // Manual alignment. This is a bit complicated if we don't want to use
    // ebp/rpb register and standard prolog.
    if (stackSize && f->stackAlignmentSize())
    {
      // stackAlignmentSize can be 8 or 16
      a.and_(nsp, -((Int32)f->stackAlignmentSize()));
      isStackAlignedTo16Bytes = true;
    }
#endif // ASMJIT_X86
  }

  SysInt nspPos = alignTo16Bytes(f->variablesStackSize());
  if (f->naked()) nspPos -= stackSize;

  // Save XMM registers using MOVDQA/MOVDQU.
  for (i = 0; i < NUM_REGS; i++)
  {
    if ((f->modifiedXmmRegisters() & (1U << i)) && (f->cconvPreservedXmm() & (1U << i)))
    {
      if (isStackAlignedTo16Bytes)
        a.movdqa(dqword_ptr(nsp, nspPos), mk_xmm(i));
      else
        a.movdqu(dqword_ptr(nsp, nspPos), mk_xmm(i));
      nspPos += 16;
    }
  }

  // Save GP registers using MOV.
  if (!f->prologEpilogPushPop())
  {
    for (i = 0; i < NUM_REGS; i++)
    {
      if ((f->modifiedGpRegisters() & (1U << i)) && (f->cconvPreservedGp() & (1U << i)) && (i != (REG_NSP & REGCODE_MASK)) )
      {
        a.mov(sysint_ptr(nsp, nspPos), mk_gpn(i));
        nspPos += sizeof(SysInt);
      }
    }
  }

  // After prolog, bind label
  if (_label) a.bind(_label);
}

// ============================================================================
// [AsmJit::Epilog]
// ============================================================================

Epilog::Epilog(Compiler* c, Function* f) ASMJIT_NOTHROW :
  Emittable(c, EMITTABLE_EPILOGUE),
  _function(f)
{
}

Epilog::~Epilog() ASMJIT_NOTHROW
{
}

void Epilog::emit(Assembler& a)
{
  Function* f = function();
  ASMJIT_ASSERT(f);

  const CpuInfo* ci = cpuInfo();

  // In 64-bit mode the stack is aligned to 16 bytes by default.
  bool isStackAlignedTo16Bytes = sizeof(SysInt) == 8;

  // How many bytes to add to stack to make it aligned to 128-bits.
  SysInt stackAdjust = (f->naked() && sizeof(SysInt) == 8) ? 8 : 0;

  // Calculate stack size with stack adjustment. This will give us proper
  // count of bytes to subtract from esp/rsp.
  SysInt stackSize = getStackSize(f, stackAdjust);

  int i;

#if defined(ASMJIT_X86)
  if (!f->naked() && stackSize && f->stackAlignmentSize())
  {
    isStackAlignedTo16Bytes = true;
  }
#endif // ASMJIT_X86

  // First bind label (Function::_exitLabel) before the epilog.
  if (_label) a.bind(_label);

  SysInt nspPos = alignTo16Bytes(f->variablesStackSize());
  if (f->naked()) nspPos -= stackSize;

  // Restore XMM registers using MOV.
  for (i = 0; i < NUM_REGS; i++)
  {
    if ((f->modifiedXmmRegisters() & (1U << i)) && (f->cconvPreservedXmm() & (1U << i)))
    {
      if (isStackAlignedTo16Bytes)
        a.movdqa(mk_xmm(i), dqword_ptr(nsp, nspPos));
      else
        a.movdqu(mk_xmm(i), dqword_ptr(nsp, nspPos));
      nspPos += 16;
    }
  }

  // Restore GP registers using MOV.
  if (!f->prologEpilogPushPop())
  {
    for (i = 0; i < NUM_REGS; i++)
    {
      if ((f->modifiedGpRegisters() & (1U << i)) && (f->cconvPreservedGp() & (1U << i)) && (i != (REG_NSP & REGCODE_MASK)) )
      {
        a.mov(mk_gpn(i), sysint_ptr(nsp, nspPos));
        nspPos += sizeof(SysInt);
      }
    }
  }
  // Restore GP registers using PUSH/POP.
  else
  {
    if (!f->naked())
    {
      SysInt stackAdd = stackSize - (f->countOfGpRegistersToBeSaved() * sizeof(SysInt));
      if (stackAdd != 0) a.add(nsp, stackAdd);
    }

    for (i = NUM_REGS; i >= 0; i--)
    {
      if ((f->modifiedGpRegisters() & (1U << i)) && (f->cconvPreservedGp() & (1U << i)) && (i != (REG_NSP & REGCODE_MASK)) )
      {
        a.pop(mk_gpn(i));
      }
    }
  }

  // Emms
  if (f->emms()) a.emms();

  // Sfence / Lfence / Mfence
  if ( f->sfence() && !f->lfence()) a.sfence(); // Only sfence
  if (!f->sfence() &&  f->lfence()) a.lfence(); // Only lfence
  if ( f->sfence() &&  f->lfence()) a.mfence(); // MFence == SFence & LFence

  // Use epilog code (if needed)
  if (!f->naked())
  {
    bool emitLeave = (f->optimizedPrologEpilog() &&  ci->vendorId == CpuInfo::Vendor_AMD);

    if (emitLeave)
    {
      a.leave();
    }
    else
    {
      a.mov(nsp, nbp);
      a.pop(nbp);
    }
  }

  // Return using correct instruction.
  if (f->calleePopsStack())
    a.ret((Int16)f->argumentsStackSize());
  else
    a.ret();
}

// ============================================================================
// [AsmJit::Target]
// ============================================================================

Target::Target(Compiler* c, Label* target) ASMJIT_NOTHROW :
  Emittable(c, EMITTABLE_TARGET), 
  _target(target)
{
}

Target::~Target() ASMJIT_NOTHROW
{
}

void Target::emit(Assembler& a)
{
  a.bind(_target);
}

// ============================================================================
// [AsmJit::JumpTable]
// ============================================================================

JumpTable::JumpTable(Compiler* c) ASMJIT_NOTHROW :
  Emittable(c, EMITTABLE_TARGET),
  _target(c->newLabel())
{
}

JumpTable::~JumpTable() ASMJIT_NOTHROW
{
}

void JumpTable::emit(Assembler& a)
{
}

void JumpTable::postEmit(Assembler& a)
{
  a.align(sizeof(SysInt));

#if defined(ASMJIT_X64)
  // help with RIP addressing
  a._embedLabel(_target);
#endif

  a.bind(_target);

  SysUInt i, len = _labels.length();
  for (i = 0; i < len; i++)
  {
    Label* label = _labels[i];
    if (label)
      a._embedLabel(label);
    else
      a.dsysint(0);
  }
}

Label* JumpTable::addLabel(Label* target, SysInt pos)
{
  if (!target) target = compiler()->newLabel();

  if (pos != -1)
  {
    while (_labels.length() <= (SysUInt)pos) _labels.append(NULL);
    _labels[(SysUInt)pos] = target;
  }
  else
  {
    _labels.append(target);
  }
    
  return target;
}

// ============================================================================
// [AsmJit::CompilerCore - Construction / Destruction]
// ============================================================================

CompilerCore::CompilerCore() ASMJIT_NOTHROW :
  _first(NULL),
  _last(NULL),
  _current(NULL),
  _currentFunction(NULL),
  _labelIdCounter(1),
  _inlineCommentBuffer(NULL)
{
  _jumpTableLabel = newLabel();
}

CompilerCore::~CompilerCore() ASMJIT_NOTHROW
{
  delAll(_first);
}

// ============================================================================
// [AsmJit::CompilerCore - Buffer]
// ============================================================================

void CompilerCore::clear() ASMJIT_NOTHROW
{
  delAll(_first);

  _first = NULL;
  _last = NULL;
  _current = NULL;

  _zone.freeAll();

  _operands.clear();
  _jumpTableLabel = newLabel();
  _jumpTableData.clear();
}

void CompilerCore::free() ASMJIT_NOTHROW
{
  clear();
  _operands.free();
  _jumpTableData.free();
}

// ============================================================================
// [AsmJit::Compiler - Emittables]
// ============================================================================

void CompilerCore::addEmittable(Emittable* emittable) ASMJIT_NOTHROW
{
  ASMJIT_ASSERT(emittable != NULL);
  ASMJIT_ASSERT(emittable->_prev == NULL);
  ASMJIT_ASSERT(emittable->_next == NULL);

  if (_current == NULL)
  {
    if (!_first)
    {
      _first = emittable;
      _last = emittable;
    }
    else
    {
      emittable->_next = _first;
      _first->_prev = emittable;
      _first = emittable;
    }
  }
  else
  {
    Emittable* prev = _current;
    Emittable* next = _current->_next;

    emittable->_prev = prev;
    emittable->_next = next;

    prev->_next = emittable;
    if (next)
      next->_prev = emittable;
    else
      _last = emittable;
  }

  _current = emittable;
}

void CompilerCore::removeEmittable(Emittable* emittable) ASMJIT_NOTHROW
{
  Emittable* prev = emittable->_prev;
  Emittable* next = emittable->_next;

  if (_first == emittable) { _first = next; } else { prev->_next = next; }
  if (_last  == emittable) { _last  = prev; } else { next->_prev = prev; }

  emittable->_prev = NULL;
  emittable->_next = NULL;

  if (emittable == _current) _current = prev;
}

Emittable* CompilerCore::setCurrentEmittable(Emittable* current) ASMJIT_NOTHROW
{
  Emittable* old = _current;
  _current = current;
  return old;
}

// ============================================================================
// [AsmJit::Compiler - Logging]
// ============================================================================

void CompilerCore::comment(const char* fmt, ...) ASMJIT_NOTHROW
{
  char buf[1024];
  char* p = buf;

  if (fmt)
  {
    *p++ = ';';
    *p++ = ' ';

    va_list ap;
    va_start(ap, fmt);
    p += vsnprintf(p, 1020, fmt, ap);
    va_end(ap);
  }

  *p++ = '\n';
  *p   = '\0';

  addEmittable(newObject<Comment>(buf));
}

// ============================================================================
// [AsmJit::Compiler - Function Builder]
// ============================================================================

Function* CompilerCore::newFunction_(UInt32 cconv, const UInt32* args, SysUInt count) ASMJIT_NOTHROW
{
  ASMJIT_ASSERT(_currentFunction == NULL);

  Function* f = _currentFunction = newObject<Function>();
  f->setPrototype(cconv, args, count);

  addEmittable(f);

  Prolog* e = newProlog(f);
  e->_label = f->_prologLabel;

  return f;
}

Function* CompilerCore::endFunction() ASMJIT_NOTHROW
{
  ASMJIT_ASSERT(_currentFunction != NULL);
  Function* f = _currentFunction;

  // Clear prevention (this is probably not needed anymore)
  f->clearPrevented();

  Epilog* e = newEpilog(f);
  e->_label = f->_exitLabel;

  _currentFunction = NULL;
  return f;
}

Prolog* CompilerCore::newProlog(Function* f) ASMJIT_NOTHROW
{
  Prolog* e = newObject<Prolog>(f);
  addEmittable(e);
  return e;
}

Epilog* CompilerCore::newEpilog(Function* f) ASMJIT_NOTHROW
{
  Epilog* e = newObject<Epilog>(f);
  addEmittable(e);
  return e;
}

// ==========================================================================
// [AsmJit::Compiler - Registers allocator / Variables]
// ==========================================================================

Variable* CompilerCore::argument(SysInt i) ASMJIT_NOTHROW
{
  return currentFunction()->argument(i);
}

Variable* CompilerCore::newVariable(UInt8 type, UInt8 priority, UInt8 preferredRegister) ASMJIT_NOTHROW
{
  return currentFunction()->newVariable(type, priority, preferredRegister);
}

bool CompilerCore::alloc(Variable* v, UInt8 mode, UInt8 preferredRegister) ASMJIT_NOTHROW
{
  return currentFunction()->alloc(v, mode, preferredRegister);
}

bool CompilerCore::spill(Variable* v) ASMJIT_NOTHROW
{
  return currentFunction()->spill(v);
}

void CompilerCore::unuse(Variable* v) ASMJIT_NOTHROW
{
  return currentFunction()->unuse(v);
}

void CompilerCore::spillAll() ASMJIT_NOTHROW
{
  return currentFunction()->spillAll();
}

void CompilerCore::spillAllGp() ASMJIT_NOTHROW
{
  return currentFunction()->spillAllGp();
}

void CompilerCore::spillAllMm() ASMJIT_NOTHROW
{
  return currentFunction()->spillAllMm();
}

void CompilerCore::spillAllXmm() ASMJIT_NOTHROW
{
  return currentFunction()->spillAllXmm();
}

void CompilerCore::spillRegister(const BaseReg& reg) ASMJIT_NOTHROW
{
  return currentFunction()->spillRegister(reg);
}

SysInt CompilerCore::numFreeGp() const ASMJIT_NOTHROW
{
  return _currentFunction->numFreeGp();
}

SysInt CompilerCore::numFreeMm() const ASMJIT_NOTHROW
{
  return _currentFunction->numFreeMm();
}

SysInt CompilerCore::numFreeXmm() const ASMJIT_NOTHROW
{
  return _currentFunction->numFreeXmm();
}

bool CompilerCore::isPrevented(Variable* v) ASMJIT_NOTHROW
{
  return currentFunction()->isPrevented(v);
}

void CompilerCore::addPrevented(Variable* v) ASMJIT_NOTHROW
{
  return currentFunction()->addPrevented(v);
}

void CompilerCore::removePrevented(Variable* v) ASMJIT_NOTHROW
{
  return currentFunction()->removePrevented(v);
}

void CompilerCore::clearPrevented() ASMJIT_NOTHROW
{
  currentFunction()->clearPrevented();
}

// ==========================================================================
// [AsmJit::Compiler - State]
// ==========================================================================

State* CompilerCore::saveState() ASMJIT_NOTHROW
{
  return currentFunction()->saveState();
}

void CompilerCore::restoreState(State* state) ASMJIT_NOTHROW
{
  currentFunction()->restoreState(state);
}

void CompilerCore::setState(State* state) ASMJIT_NOTHROW
{
  currentFunction()->setState(state);
}

// ============================================================================
// [AsmJit::Compiler - Labels]
// ============================================================================

Label* CompilerCore::newLabel() ASMJIT_NOTHROW
{
  Label* label = new(_zoneAlloc(sizeof(Label))) Label((UInt16)(_labelIdCounter++));
  _registerOperand(label);
  return label;
}

// ============================================================================
// [AsmJit::Compiler - Jump Table]
// ============================================================================

JumpTable* CompilerCore::newJumpTable() ASMJIT_NOTHROW
{
  JumpTable* e = newObject<JumpTable>();
  addEmittable(e);
  return e;
}

// ============================================================================
// [AsmJit::Compiler - Memory Management]
// ============================================================================

void CompilerCore::_registerOperand(Operand* op) ASMJIT_NOTHROW
{
  op->_operandId = _operands.length();
  _operands.append(op);
}

// ============================================================================
// [AsmJit::Compiler - Jumps / Calls]
// ============================================================================

void CompilerCore::jumpToTable(JumpTable* jt, const Register& index) ASMJIT_NOTHROW
{
#if defined(ASMJIT_X64)
  // 64 bit mode: Complex address can't be used, because SIB byte not allows
  // to use RIP (relative addressing). SIB byte is always generated for 
  // complex addresses.
  // address form: [jumpTable + index * 8]
  shl(index, imm(3));                // index *= 8
  add(index, ptr(jt->target(), -8)); // index += jumpTable base address
  jmp(ptr(index));                   // jmp [index]
#else
  // 32 bit mode: Straighforward implementation, we are using complex address
  // form: [jumpTable + index * 4]
  jmp(ptr(jt->target(), index, TIMES_4));
#endif
}

SysInt CompilerCore::_addTarget(void* target) ASMJIT_NOTHROW
{
  SysInt id = _jumpTableData.length() * sizeof(SysInt);
  _jumpTableData.append(target);
  return id;
}

// jmpAndRestore

void CompilerCore::_jmpAndRestore(UInt32 code, Label* label, State* state) ASMJIT_NOTHROW
{
  JumpAndRestore* jr = (JumpAndRestore*)_zoneAlloc(sizeof(JumpAndRestore));
  jr->next = (JumpAndRestore*)label->_compilerData;
  jr->from = currentFunction()->saveState();
  jr->to = state;
  label->_compilerData = (void*)jr;

  emitX86(code, label);
  jr->instruction = reinterpret_cast<Instruction*>(_current);
}

// ============================================================================
// [AsmJit::Compiler - Intrinsics]
// ============================================================================

void CompilerCore::op_var32(UInt32 code, const Int32Ref& a) ASMJIT_NOTHROW
{
  if (a.state() == VARIABLE_STATE_REGISTER)
  {
    Register ar = a.r32();
    emitX86(code, &ar);
  }
  else
  {
    emitX86(code, &a.m());
  }
}

void CompilerCore::op_reg32_var32(UInt32 code, const Register& a, const Int32Ref& b) ASMJIT_NOTHROW
{
  if (b.state() == VARIABLE_STATE_REGISTER)
  {
    Register br = b.r32();
    emitX86(code, &a, &br);
  }
  else
  {
    emitX86(code, &a, &b.m());
  }
}

void CompilerCore::op_var32_reg32(UInt32 code, const Int32Ref& a, const Register& b) ASMJIT_NOTHROW
{
  if (a.state() == VARIABLE_STATE_REGISTER)
  {
    Register ar = a.r32();
    emitX86(code, &ar, &b);
  }
  else
  {
    emitX86(code, &a.m(), &b);
  }
}

void CompilerCore::op_var32_imm(UInt32 code, const Int32Ref& a, const Immediate& b) ASMJIT_NOTHROW
{
  if (a.state() == VARIABLE_STATE_REGISTER)
  {
    Register ar = a.r32();
    emitX86(code, &ar, &b);
  }
  else
  {
    emitX86(code, &a.m(), &b);
  }
}

#if defined(ASMJIT_X64)
void CompilerCore::op_var64(UInt32 code, const Int64Ref& a) ASMJIT_NOTHROW
{
  if (a.state() == VARIABLE_STATE_REGISTER)
  {
    Register ar = a.r64();
    emitX86(code, &ar);
  }
  else
  {
    emitX86(code, &a.m());
  }
}

void CompilerCore::op_reg64_var64(UInt32 code, const Register& a, const Int64Ref& b) ASMJIT_NOTHROW
{
  if (b.state() == VARIABLE_STATE_REGISTER)
  {
    Register br = b.r64();
    emitX86(code, &a, &br);
  }
  else
  {
    emitX86(code, &a, &b.m());
  }
}

void CompilerCore::op_var64_reg64(UInt32 code, const Int64Ref& a, const Register& b) ASMJIT_NOTHROW
{
  if (a.state() == VARIABLE_STATE_REGISTER)
  {
    Register ar = a.r64();
    emitX86(code, &ar, &b);
  }
  else
  {
    emitX86(code, &a.m(), &b);
  }
}

void CompilerCore::op_var64_imm(UInt32 code, const Int64Ref& a, const Immediate& b) ASMJIT_NOTHROW
{
  if (a.state() == VARIABLE_STATE_REGISTER)
  {
    Register ar = a.r64();
    emitX86(code, &ar, &b);
  }
  else
  {
    emitX86(code, &a.m(), &b);
  }
}
#endif // ASMJIT_X64

// ============================================================================
// [AsmJit::Compiler - EmitX86]
// ============================================================================

void CompilerCore::_inlineComment(const char* _text, SysInt len) ASMJIT_NOTHROW
{
  if (len < 0) len = strlen(_text);

  if (len > 0)
  {
    if (len > MAX_INLINE_COMMENT_SIZE - 1) len = MAX_INLINE_COMMENT_SIZE - 1;

    char* text = (char*)_zoneAlloc((len + 1 + sizeof(SysInt)-1) & ~(sizeof(SysInt)-1));
    memcpy(text, _text, len + 1);

    _inlineCommentBuffer = text;
  }
  else
  {
    _inlineCommentBuffer = NULL;
  }
}

void CompilerCore::_emitX86(UInt32 code, const Operand* o1, const Operand* o2, const Operand* o3) ASMJIT_NOTHROW
{
  addEmittable(newObject<Instruction>(code, o1, o2, o3, _inlineCommentBuffer));
  _inlineCommentBuffer = NULL;

  // We can clear last used register, because instruction was emitted.
  if (currentFunction()) currentFunction()->clearPrevented();
}

// ============================================================================
// [AsmJit::Compiler - Embed]
// ============================================================================

void CompilerCore::_embed(const void* data, SysUInt size) ASMJIT_NOTHROW
{
  // Align capacity to 16 bytes
  SysUInt capacity = (size + 15) & ~15;

  EmbeddedData* e = 
    new(_zoneAlloc(sizeof(EmbeddedData) - sizeof(void*) + capacity)) 
      EmbeddedData(reinterpret_cast<Compiler*>(this), capacity, data, size);
  addEmittable(e);
}

// ============================================================================
// [AsmJit::Compiler - Align]
// ============================================================================

void CompilerCore::align(SysInt m) ASMJIT_NOTHROW
{
  addEmittable(newObject<Align>(m));
}

// ============================================================================
// [AsmJit::Compiler - Bind]
// ============================================================================

void CompilerCore::bind(Label* label) ASMJIT_NOTHROW
{
  // JumpAndRestore is delayed to bind()
  if (label->_compilerData) Function::_jmpAndRestore(reinterpret_cast<Compiler*>(this), label);

  addEmittable(newObject<Target>(label));
}

// ============================================================================
// [AsmJit::Compiler - Make]
// ============================================================================

void* CompilerCore::make(MemoryManager* memoryManager, UInt32 allocType) ASMJIT_NOTHROW
{
  Assembler a;
  a._properties = _properties;
  serialize(a);

  if (a.error())
  {
    if (_logger)
    {
      _logger->logFormat("; Compiler failed: %s (%u).\n\n",
        errorCodeToString(a.error()), (unsigned int)a.error());
    }

    setError(a.error());
    return NULL;
  }
  else
  {
    if (_logger)
    {
      _logger->logFormat("; Compiler successful (wrote %u bytes).\n\n",
        (unsigned int)a.codeSize());
    }

    return a.make(memoryManager, allocType);
  }
}

// Logger switcher used in Compiler::serialize().
struct ASMJIT_HIDDEN LoggerSwitcher
{
  LoggerSwitcher(Assembler* a, Compiler* c) ASMJIT_NOTHROW :
    a(a),
    logger(a->logger())
  {
    // Set compiler logger.
    if (!logger && c->logger()) a->setLogger(c->logger());
  }

  ~LoggerSwitcher() ASMJIT_NOTHROW
  {
    // Restore logger.
    a->setLogger(logger);
  }

  Assembler* a;
  Logger* logger;
};

void CompilerCore::serialize(Assembler& a) ASMJIT_NOTHROW
{
  LoggerSwitcher loggerSwitcher(&a, reinterpret_cast<Compiler*>(this));
  Emittable* cur;

  // Prepare (prepare action can append emittable).
  for (cur = _first; cur; cur = cur->next()) cur->prepare();

  // Emit and postEmit.
  for (cur = _first; cur; cur = cur->next()) cur->emit(a);
  for (cur = _first; cur; cur = cur->next()) cur->postEmit(a);

  // Jump table.
  SysUInt i, len;
  a.bind(_jumpTableLabel);

  len = _jumpTableData.length();
  for (i = 0; i < len; i++)
  {
    a.dptr(_jumpTableData[i]);
  }
}

// ============================================================================
// [AsmJit::Compiler - Construction / Destruction]
// ============================================================================

Compiler::Compiler() ASMJIT_NOTHROW {}
Compiler::~Compiler() ASMJIT_NOTHROW {}

} // AsmJit namespace

// [Warnings-Pop]
#include "WarningsPop.h"

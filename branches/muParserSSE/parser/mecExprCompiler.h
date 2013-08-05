/*
              __________                                   _________ ____________________
   _____  __ _\______   \_____ _______  ______ ___________/   _____//   _____/\_   _____/
  /     \|  |  \     ___/\__  \\_  __ \/  ___// __ \_  __ \_____  \ \_____  \  |    __)_ 
 |  Y Y  \  |  /    |     / __ \|  | \/\___ \\  ___/|  | \/        \/        \ |        \
 |__|_|  /____/|____|    (____  /__|  /____  >\___  >__| /_______  /_______  //_______  /
       \/                     \/           \/     \/             \/        \/         \/ 
 
  Copyright (C) 2011 Ingo Berg

  Permission is hereby granted, free of charge, to any person obtaining a copy of this 
  software and associated documentation files (the "Software"), to deal in the Software
  without restriction, including without limitation the rights to use, copy, modify, 
  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
  permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or 
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/
#ifndef MEC_EXPR_COMPILER_H
#define MEC_EXPR_COMPILER_H

#include <memory>
#include <list>

//--- ASMJit compiler---------------------------------------------------------------------
#include "AsmJit/Assembler.h"
#include "AsmJit/Compiler.h"
#include "AsmJit/MemoryManager.h"
#include "AsmJit/Logger.h"

//--- muParser framework --------------------------------------------------------------
#include "mecDef.h"
#include "mecReversePolishNotation.h"
#include "mecStack.h"


namespace mec
{
  class ExprCompiler
  {
  public:
    ExprCompiler();
    ExprCompiler(const ExprCompiler &ref);
   ~ExprCompiler();

    void Bind(SPackedToken *pRPN);
    value_type (*Compile(int nHighestReg))();
    void SetLastRegister(int n);

  private:

    struct SJumpTag
    {
      SJumpTag(AsmJit::Label* a_Label, int a_StackPos, int a_Len)
        :Label(a_Label)
        ,StackPos(a_StackPos)
        ,Len(a_Len)
      {}

      AsmJit::Label* Label;
      int StackPos;
      int Len;
    };

    typedef int int_ptr;

    SPackedToken *m_pRPN;
    value_type m_buf;

    exprfun_type m_pExpr;
    int  m_nStackPos;
    std::auto_ptr<AsmJit::Compiler> m_pCompiler;
    AsmJit::XMMRegister xmm[8];

    int PrepareFunctionArguments(int argc);
    bool LoadArguments(AsmJit::XMMRegister &arg1, AsmJit::XMMRegister &arg2);
    bool LoadArgument(AsmJit::XMMRegister &arg);
    bool LoadArgumentEx(AsmJit::XMMRegister &arg1, AsmJit::XMMRegister &arg2);

    void Push(value_type *v);

    void GenericFunction(void *pFun, int argc, bool bSaveReg);
    void BinOp(ECmdCode eCode);
    void BinOpCompare(ECmdCode eCode);
    void BinOpLogic(ECmdCode eCode);
    void PowerOf();
    void IntrinsicFPUFunction(ECmdCode eCode);
    void IntrinsicSSEFunction(ECmdCode eCode);
    void ReleaseExpr();
    void SaveSSERegisters();
    void RestoreSSERegisters();
    
    void If(int nJumpLen);
    void Else(int nJumpLen);
    void EndIf();

    void InsertJumpLabels();

    // static constants for commonly used values
    static const value_type c_zero;
    static const value_type c_one;
    static const int m_nMXCSR = 0x7e03;
    //static const int m_nMXCSR = 0x7f85;
    int m_nLastRegister;

    std::list< SJumpTag > m_vIfLabels;
    value_type m_xmm_saved[16];
  };
}

#endif

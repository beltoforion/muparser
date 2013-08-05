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
#include "mecExprCompiler.h"

#include <cassert>
#include <cmath>
#include <stdexcept>

using namespace AsmJit;
using namespace std;


namespace mec
{

  //---------------------------------------------------------------------------------------------
  __declspec(naked) void debugbreak()
  {
    puts("halt");
  }

  const value_type ExprCompiler::c_zero = 0;
  const value_type ExprCompiler::c_one = 1;

  //---------------------------------------------------------------------------------------------
  ExprCompiler::ExprCompiler()
    :m_pRPN(NULL)
    ,m_buf(0)
    ,m_pExpr(NULL)
    ,m_nStackPos(-1)
    ,m_nLastRegister(5)
    ,m_vIfLabels()
  {
    // copy asmjit SSE register classes into an array for easier
    // access in automatically generated code.
    xmm[0] = xmm0;
    xmm[1] = xmm1;
    xmm[2] = xmm2;
    xmm[3] = xmm3;
    xmm[4] = xmm4;
    xmm[5] = xmm5;
    xmm[6] = xmm6;
    xmm[7] = xmm7;
  }

  //---------------------------------------------------------------------------------------------
  ExprCompiler::ExprCompiler(const ExprCompiler &ref)
  {
    m_pRPN = ref.m_pRPN;
    m_nStackPos = ref.m_nStackPos;
    m_nLastRegister = ref.m_nLastRegister;
  
    ReleaseExpr();
    m_pExpr = ref.m_pExpr;
  }

  //---------------------------------------------------------------------------------------------
  ExprCompiler::~ExprCompiler()
  {
    ReleaseExpr();
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::Bind(SPackedToken *pRPN)
  {
    ReleaseExpr();
    m_pRPN = pRPN;
    m_buf = 0;
    m_nStackPos = -1;
    m_pExpr = NULL;
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::ReleaseExpr()
  {
    if (m_pExpr!=NULL)
      AsmJit::MemoryManager::global()->free((void*)m_pExpr);
  }

  //---------------------------------------------------------------------------------------------
  exprfun_type ExprCompiler::Compile(int nHighesReg)
  {
    assert(nHighesReg<=5);
    m_nLastRegister = nHighesReg;
    m_pCompiler.reset(new Compiler());
    Compiler &c = *m_pCompiler;

    m_nStackPos = -1;
    assert(m_pRPN);

    FILE *pLog = NULL;
    FileLogger logger;
    if (g_DbgDumpCmdCode)
    {
      pLog = fopen("asmout.asm", "w");
      logger.setStream(pLog);
      c.setLogger(&logger);
    }

    // MXCSR Register auf truncate setzen, sonst funktioniert die
    // modulo implementierung nicht.
    //c.mov(ecx, Immediate((int_ptr)&m_nMXCSR));
    //c.ldmxcsr(dword_ptr(ecx));

    // Tell compiler the function prototype we want. It allocates variables representing
    // function arguments that can be accessed through Compiler or Function instance.
    AsmJit::Function &f = *c.newFunction(CALL_CONV_DEFAULT, BuildFunction0());
    f.setNaked(true);

    value_type v = 0;
    
    // Transform the bytecode into a compiled expression
    for (SPackedToken *pTok = m_pRPN; ; ++pTok)
    {
      ECmdCode eCmd = (ECmdCode)pTok->m_eCode;

      switch (eCmd)
      {        
      case  cmLE:   
      case  cmGE:   
      case  cmNEQ: 
      case  cmEQ:    
      case  cmLT:  
      case  cmGT:    BinOpCompare(eCmd); break;

      case  cmAND: 
      case  cmOR:    BinOpLogic(eCmd);   break;
      
      case  cmMIN:
      case  cmMAX:
      case  cmSUB:  
      case  cmMUL:  
      case  cmDIV:  
//      case  cmMOD:
      case  cmADD:   BinOp(eCmd); break;      

      case  cmSIN:
      case  cmCOS:
      case  cmTAN:   IntrinsicFPUFunction(eCmd); break;
      case  cmABS:
      case  cmSQRT:  IntrinsicSSEFunction(eCmd); break;

      // if-then-else
      case  cmIF:    If(pTok->Jmp.offset);
                     break;
      case  cmELSE:  Else(pTok->Jmp.offset);
                     break;
      case  cmENDIF: EndIf();
                     break;

      // value and variable tokens
      case  cmVAR:   Push (pTok->m_pVar); break;
      case  cmVAL:   Push(&pTok->m_fVal); break;

      // Next is treatment of numeric functions
      case  cmFUNC:  GenericFunction(pTok->Fun.m_pFun, pTok->Fun.m_nArgc, true);
                     break;

      case  cmEND:
              {
                // The result is located in xmm0, move it to m_compiledResult 
                // member variable and from there move it to the FPU
                // stack where the return value should be
                c.comment("Move final result to the FPU stack");
                if (m_nLastRegister>=0)
                {
                  c.mov(ecx, Immediate((int_ptr)&m_buf));
                  c.movss(dword_ptr(ecx), xmm0);
                  c.fld(dword_ptr(ecx));
                }
                else
                {
                  c.fld(dword_ptr(esp));
                  c.add(esp, sizeof(value_type));
                }
              }
              break;
      } // switch (eCmd)

      InsertJumpLabels();
      
      if (eCmd==cmEND)
        break;
    }

    c.endFunction();
    
    ReleaseExpr();
    m_pExpr = function_cast<exprfun_type>(c.make());

    if (pLog!=NULL)
    {
      fflush(pLog);
      m_pCompiler.reset(0);
    }

    return m_pExpr;
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::InsertJumpLabels()
  {
    Compiler &c = *m_pCompiler;
    std::list< SJumpTag > tmp;
    std::list< SJumpTag >::reverse_iterator it = m_vIfLabels.rbegin();
    for (; it!=m_vIfLabels.rend(); ++it)
    {
      SJumpTag &jmp = *it;
      jmp.Len--;

      if (jmp.Len==-1)
      {
        m_nStackPos = jmp.StackPos;
        c.bind(jmp.Label);
      }
      else
        tmp.push_back(jmp);
    }
    m_vIfLabels = tmp;
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::Push(value_type *v)
  {
    Compiler &c = *m_pCompiler;
    m_nStackPos++;
    assert(m_nStackPos>=0);

    // 6 xmm registers are used for storing the values of the calculation stack
    // 2 xmm registers are used for calculating temporary results
    // all other values will be pushed to the stack

    if (m_nStackPos<=m_nLastRegister)
    {
      // The lowermost 6 Values of the calculation stack are stored directly in
      // the xmm registers
      c.comment("Pushing value %lf to sse register xmm%d", *v, m_nStackPos);
      c.mov(ecx, Immediate((int_ptr)v));
      c.movss(xmm[m_nStackPos], dword_ptr(ecx));
    }
    else
    {
      // The 6 lowermost registeres are full (apart from the two registers
      // needed for temporary calculations) All new values will be pushed 
      // to the stack now
      c.comment("SSE registers occupied; Pushing value %lf to stack", *v);
      c.sub(esp, sizeof(value_type));
      c.mov(ecx, Immediate((int_ptr)v));
      c.movss(xmm[m_nLastRegister+1], dword_ptr(ecx));
      c.movss(dword_ptr(esp), xmm[m_nLastRegister+1]);
    }
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::If(int nJumpLen)
  {
    Compiler &c = *m_pCompiler;

    XMMRegister arg;
    bool bPushToStack = LoadArgument(arg);
    --m_nStackPos;

    if (bPushToStack)
    {
      c.add(esp, sizeof(value_type));
    }

    // Test arg for zero
    c.comment("If-then-else; Test arg for zero");
    c.mov(ecx, Immediate((int_ptr)&ExprCompiler::c_zero));
    c.ucomiss(arg, dword_ptr(ecx));

    Label* lbIf = c.newLabel();
    m_vIfLabels.push_back( SJumpTag(lbIf, m_nStackPos, nJumpLen) );

    c.je(lbIf);
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::Else(int nJumpLen)
  {
    Compiler &c = *m_pCompiler;

    Label* lbElse = c.newLabel();
    c.jmp(lbElse);
    m_vIfLabels.push_back( SJumpTag(lbElse, m_nStackPos, nJumpLen) );
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::EndIf()
  {
    //Compiler &c = *m_pCompiler;
    //std::pair<Label*, int> p = m_vIfLabels.pop();
    //Label* lbElse = p.first;
    //c.bind(lbElse);
  }

  //---------------------------------------------------------------------------------------------
  bool ExprCompiler::LoadArguments(XMMRegister &arg1, XMMRegister &arg2)
  {
    Compiler &c = *m_pCompiler;
    bool bPushToStack = false;
    if (m_nStackPos<m_nLastRegister)
    {
      arg1 = xmm[m_nStackPos];
      arg2 = xmm[m_nStackPos+1];
    }
    else if (m_nStackPos==m_nLastRegister)
    {
      c.comment("Loading single argument from stack into xmm[%d]"
                ,m_nLastRegister+1);
      c.movss(xmm[m_nLastRegister+1], dword_ptr(esp));
      c.add(esp, sizeof(value_type));
      arg1 = xmm[m_nLastRegister];
      arg2 = xmm[m_nLastRegister+1];
    }
    else
    {
      c.comment("Loading arguments from stack into xmm%d and xmm%d"
                ,m_nLastRegister+1
                ,m_nLastRegister+2);
      c.movss(xmm[m_nLastRegister+2], dword_ptr(esp));
      c.add(esp, sizeof(value_type));
      c.movss(xmm[m_nLastRegister+1], dword_ptr(esp));
      arg1 = xmm[m_nLastRegister+1];
      arg2 = xmm[m_nLastRegister+2];
      bPushToStack = true;
    }

    return bPushToStack;
  }

  //---------------------------------------------------------------------------------------------
  bool ExprCompiler::LoadArgument(XMMRegister &arg)
  {
    Compiler &c = *m_pCompiler;
    bool bPushToStack = false;
    if (m_nStackPos<=m_nLastRegister)
    {
      arg = xmm[m_nStackPos];
    }
    else
    {
      c.comment("Loading arguments from stack int xmm%d", m_nLastRegister+1);
      c.movss(xmm[m_nLastRegister+1], dword_ptr(esp));
      arg = xmm[m_nLastRegister+1];
      bPushToStack = true;
    }

    return bPushToStack;
  }

  //---------------------------------------------------------------------------------------------
  bool ExprCompiler::LoadArgumentEx(XMMRegister &arg1, XMMRegister &arg2)
  {
    Compiler &c = *m_pCompiler;
    bool bPushToStack = false;
    if (m_nStackPos<=m_nLastRegister)
    {
      arg1 = xmm[m_nStackPos];
      arg2 = xmm[m_nStackPos+1];
    }
    else
    {
      c.movss(xmm[m_nLastRegister+1], dword_ptr(esp));
      arg1 = xmm[m_nLastRegister+1];
      arg2 = xmm[m_nLastRegister+2];
      bPushToStack = true;
    }

    return bPushToStack;
  }

  //---------------------------------------------------------------------------------------------
  /** \brief Loads a number of function arguments onto the stack properly ordered for stdcall 
             functions. 
      \return The number of values that need to be popped from the stack for the cleanup.
  */
  int ExprCompiler::PrepareFunctionArguments(int argc)
  {
    if (argc==0)
    {
      m_nStackPos++;
      return 0;
    }

    Compiler &c = *m_pCompiler;
    int nNumToRemove=0;

    c.comment("Preparing function arguments for an stdcall with %d arguments", argc);

    if (m_nStackPos<=m_nLastRegister)
    {
      c.comment("- All function parameters are located in xmm registers");
      c.comment("- Moving them to the stack entirely.");

      // 2 arguments -> stackpos==1; wg. 0 basiertem array
      assert(argc<=(m_nStackPos+1));
  
      c.sub(esp, argc*sizeof(value_type));

      // all function arguments are located in xmm registers.
      for (int i=m_nStackPos, ct=0; i>(m_nStackPos-argc); --i)
        c.movss(dword_ptr(esp, (argc-1-ct++)*sizeof(value_type)), xmm[i]);

      nNumToRemove = argc;
    }
    else if (m_nStackPos>m_nLastRegister)
    {
      c.comment("- Calculation stack is partly located in the CPU stack");
      c.comment("- Extending stack by %d values", (argc-1));
//      nNumToRemove = 2 * (argc - 1);
      nNumToRemove = 1;
      for (int i=1; i<argc; ++i)
      {
        int pos = m_nStackPos - i;
        if (pos>m_nLastRegister)
        {
          c.comment("- Pushing argument to the stack (esp+%d)", (int)(i*sizeof(value_type)));
          c.push(dword_ptr(esp, (int)((2*i-1)*sizeof(value_type))));
          nNumToRemove+=2;

        }
        else
        {
          c.comment("- Copying argument from xmm%d to the stack", pos);
          c.sub(esp, sizeof(value_type));
          c.movss(dword_ptr(esp/*, pos*sizeof(value_type)*/), xmm[pos]);
          nNumToRemove++;
        }
//        nNumToRemove = argc + argc - 1;
      }
    }

    m_nStackPos -= (argc-1);
    return nNumToRemove;
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::SaveSSERegisters()
  {
    if (m_nLastRegister<0)
      return;

    Compiler &c = *m_pCompiler;
    int reg_num = std::min(m_nLastRegister, 
                           (int)(sizeof(m_xmm_saved)/sizeof(value_type)) );

    // Anm.: m_nStackPos wird ohnehin mit dem Ergebnis überschrieben, alles 
    //       andere muß gesichert werden
    int sz = std::min(m_nStackPos-1, reg_num);
    if (sz<0)
      return;

    c.comment("Saving %d SSE registers", sz);
    for (int i=0; i<=sz; ++i)
    {
      c.mov(ecx, Immediate((int_ptr)&m_xmm_saved[i]));
      c.movss(dword_ptr(ecx), xmm[i]);
    }
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::RestoreSSERegisters()
  {
    if (m_nLastRegister<0)
      return;

    Compiler &c = *m_pCompiler;
    int reg_num = std::min(m_nLastRegister, 
                           (int)(sizeof(m_xmm_saved)/sizeof(value_type)) );
    int sz = std::min(m_nStackPos-1, reg_num);
    if (sz<0)
      return;

    c.comment("Restoring %d SSE registers", sz);
    for (int i=0; i<=sz; ++i)
    {
      c.mov(ecx, Immediate((int_ptr)&m_xmm_saved[i]));
      c.movss(xmm[i], dword_ptr(ecx));
    }
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::GenericFunction(void *pFun, int argc, bool bSaveReg)
  {
    Compiler &c = *m_pCompiler;

    int nNumToRemove = PrepareFunctionArguments(argc);

    c.comment("call the function and balance the stack");
    
    if (bSaveReg)
      SaveSSERegisters();

    c.call(pFun);

    if (bSaveReg)
      RestoreSSERegisters();

    c.comment("Removing function arguments from the stack");
    if (nNumToRemove>0)
      c.add(esp, nNumToRemove*sizeof(value_type));

    // Move the return value to where it belongs, either
    // an sse register or the stack
    if (m_nStackPos<=m_nLastRegister)
    {
      c.comment("Move the return value to xmm%d", m_nStackPos);
      c.mov(ecx, Immediate((int_ptr)&m_buf));
      c.fstp(dword_ptr(ecx));
      c.movss(xmm[m_nStackPos], dword_ptr(ecx));
    }
    else
    {
      c.comment("Move the return value to esp");
      c.sub(esp, sizeof(value_type));
      c.fstp(dword_ptr(esp));
    }
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::IntrinsicFPUFunction(ECmdCode eCode)  
  {
    Compiler &c = *m_pCompiler;
    XMMRegister arg;
    bool bPushToStack = LoadArgument(arg);

    // Load value into the 
    c.comment("Move argument from sse register into buffer variable");
    c.mov(ecx, Immediate((int_ptr)&m_buf));
    c.movss(dword_ptr(ecx), arg);

    switch(eCode)
    {
    case cmSIN:
         c.comment("Performing sin operation");
         c.fld(dword_ptr(ecx));
         c.fsin();
         break;

    case cmCOS:
         c.comment("Performing cos operation");
         c.fld(dword_ptr(ecx));
         c.fcos();
         break;

    case cmTAN:
         c.comment("Performing tan operation");
         c.fld(dword_ptr(ecx));
         c.fptan();
         c.fstp(st(0));
         break;
    }

    c.comment("Move result back into sse register");
    c.fstp(dword_ptr(ecx));
    c.movss(arg, dword_ptr(ecx));

    if (bPushToStack)
    {
      c.comment("Moving result back to stack");
      c.movss(dword_ptr(esp), arg);
    }
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::IntrinsicSSEFunction(ECmdCode eCode)  
  {
    Compiler &c = *m_pCompiler;
    XMMRegister arg, arg2 = xmm[7];

    bool bPushToStack = LoadArgument(arg);

    switch(eCode)
    {
    case cmABS:
         c.comment("Performing abs operation");
         {
           Label* lbExit = c.newLabel();
           c.mov(edx, Immediate(0));
           c.cvtsi2ss(arg2, edx);
           c.comiss(arg, arg2);
           c.ja(lbExit);
             c.mov(edx, Immediate(-1));
             c.cvtsi2ss(arg2, edx);
             c.mulss(arg, arg2);
           c.bind(lbExit);
         }
         break;

    case cmSQRT:
         c.comment("Performing sqrt operation");
         c.sqrtss(arg, arg);
         break;
    }

    if (bPushToStack)
    {
      c.comment("Moving result back to stack");
      c.movss(dword_ptr(esp), arg);
    }
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::BinOpCompare(ECmdCode eCode)  
  {
    Compiler &c = *m_pCompiler;
    m_nStackPos = m_nStackPos - 1;
    assert(m_nStackPos>=0);

    XMMRegister arg1, arg2;
    bool bPushToStack = LoadArguments(arg1, arg2);

    Label* label1 = c.newLabel();
    Label* label2 = c.newLabel();
    c.comiss(arg1, arg2);

    c.comment("Performing logical binary operation");
    switch(eCode)
    {
    case cmEQ:  c.jz(label1);  break;
    case cmNEQ: c.jnz(label1); break;
    case cmGT:  c.ja(label1);  break;
    case cmLT:  c.jb(label1);  break;
    case cmGE:  c.jae(label1); break;
    case cmLE:  c.jbe(label1); break;
    }

    c.mov(ecx, Immediate((int_ptr)&ExprCompiler::c_zero)) ;
    c.jmp(label2);

    // Label 1
    c.bind(label1);
    c.mov(ecx, Immediate((int_ptr)&ExprCompiler::c_one));

    // Label 2
    c.bind(label2);
    c.movss(arg1, dword_ptr(ecx));

    if (bPushToStack)
    {
      c.comment("All registers occupied; Push result to stack");
      c.movss(dword_ptr(esp), arg1);
    }    
  }


  //---------------------------------------------------------------------------------------------
  void ExprCompiler::BinOpLogic(ECmdCode eCode)  
  {
    Compiler &c = *m_pCompiler;
    m_nStackPos = m_nStackPos - 1;
    assert(m_nStackPos>=0);

    XMMRegister reg1, reg2, reg3 = xmm[7];
    bool bPushToStack = LoadArguments(reg1, reg2);

    Label* lbShortCut = c.newLabel();
    Label* lbExit = c.newLabel();
    int nShortCutResult;

    // 1. Argument mit 0 vergleichen
    c.mov(ecx, Immediate((int_ptr)&ExprCompiler::c_zero));
    c.ucomiss(reg1, dword_ptr(ecx));
    switch(eCode)
    {
    case cmOR:  c.jne(lbShortCut); nShortCutResult=1; break;
    case cmAND: c.je(lbShortCut);  nShortCutResult=0; break;
    }

    // 2. Argument mit 0 vergleichen
    c.ucomiss(reg2, dword_ptr(ecx));
    switch(eCode)
    {
    case cmOR:  c.jne(lbShortCut); nShortCutResult=1; break;
    case cmAND: c.je(lbShortCut);  nShortCutResult=0; break;
    }

    // Beide Argumente sind ungleich 0 -> eins zurück geben
    c.mov(edx, Immediate(!nShortCutResult));
    c.jmp(lbExit);

    // Mindestens ein Argument ist 0 -> 0 zurück geben
    c.bind(lbShortCut);
    c.mov(edx, Immediate(nShortCutResult));

    c.bind(lbExit);
    c.cvtsi2ss(reg1, edx);
    if (bPushToStack)
      c.movss(dword_ptr(esp), reg1);
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::PowerOf()
  {
    Compiler &c = *m_pCompiler;
    float (*pow_ff)(float, float) = std::pow;
    //float (*pow_fi)(float, int) = std::pow;

    int nNumToRemove = PrepareFunctionArguments(2);

    c.comment("calling power function");
    
    SaveSSERegisters();
    c.call(pow_ff);
    RestoreSSERegisters();

    c.comment("Removing function arguments from the stack");
    if (nNumToRemove>0)
      c.add(esp, nNumToRemove*sizeof(value_type));

    // Move the return value to where it belongs, either
    // an sse register or the stack
    if (m_nStackPos<=m_nLastRegister)
    {
      c.comment("Move the return value to xmm%d", m_nStackPos);
      c.mov(ecx, Immediate((int_ptr)&m_buf));
      c.fstp(dword_ptr(ecx));
      c.movss(xmm[m_nStackPos], dword_ptr(ecx));
    }
    else
    {
      c.comment("Move the return value to esp");
      c.sub(esp, sizeof(value_type));
      c.fstp(dword_ptr(esp));
    }
  }

  //---------------------------------------------------------------------------------------------
  void ExprCompiler::BinOp(ECmdCode eCode)
  {
    Compiler &c = *m_pCompiler;
    m_nStackPos = m_nStackPos - 1;
    assert(m_nStackPos>=0);

    XMMRegister arg1, arg2;
    bool bPushToStack = LoadArguments(arg1, arg2);

    c.comment("Performing binary operation");
    switch (eCode)
    {
    case cmMIN: c.minss(arg1, arg2); break;
    case cmMAX: c.maxss(arg1, arg2); break;
    case cmADD: c.addss(arg1, arg2); break;
    case cmSUB: c.subss(arg1, arg2); break;
    case cmMUL: c.mulss(arg1, arg2); break;
    case cmDIV: c.divss(arg1, arg2); break;

/*
    // Mod ist ein wenig komplizierter:
    case cmMOD: 
         {  
                static int buf;

                // mxcsr register speichern, überschreiben
                // um "truncate" zu aktivieren
                //c.mov(ecx, Immediate((int_ptr)&buf));
                //c.stmxcsr(dword_ptr(ecx));

                //c.mov(ecx, Immediate((int_ptr)&m_nMXCSR));
                //c.ldmxcsr(dword_ptr(ecx));

                c.comment("Saving arg1 in buffer variable");
                c.mov(ecx, Immediate((int_ptr)&m_buf));
                c.movss(dword_ptr(ecx), arg1);      

                c.divss(arg1, arg2);      //divss xmm0, xmm1
                c.cvtss2si(edx, arg1);    //cvtss2si edx, xmm0
                c.cvtsi2ss(arg1, edx);    //cvtsi2ss xmm0, edx          // von edx zurück in xmm0 kopieren 

                c.mulss(arg2, arg1);
                c.movss(arg1, dword_ptr(ecx));
                c.subss(arg1, arg2);

                // mxcsr zurücksetzen
                //c.mov(ecx, Immediate((int_ptr)&buf));
                //c.ldmxcsr(dword_ptr(ecx));
         }
         break;
*/      
    default:  throw runtime_error("Unexpected operator code");
    }

    if (bPushToStack)
    {
      c.comment("All registers occupied; Push result to stack");
      c.movss(dword_ptr(esp), xmm[m_nLastRegister+1]);
    }
  }
}
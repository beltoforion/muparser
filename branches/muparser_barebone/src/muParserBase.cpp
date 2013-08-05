/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
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

#include "muParserBase.h"
#include "muParserTemplateMagic.h"

//--- Standard includes ------------------------------------------------------------------------
#include <cassert>
#include <cmath>
#include <memory>
#include <vector>
#include <deque>
#include <sstream>
#include <locale>

#ifdef MUP_USE_OPENMP
  #include <omp.h>
#endif

using namespace std;

/** \file
    \brief This file contains the basic implementation of the muparser engine.
*/

namespace mu
{
  bool ParserBase::g_DbgDumpCmdCode = false;
  bool ParserBase::g_DbgDumpStack = false;

  //------------------------------------------------------------------------------
  /** \brief Identifiers for built in binary operators. 

      When defining custom binary operators with #AddOprt(...) make sure not to choose 
      names conflicting with these definitions. 
  */
  const char_type* ParserBase::c_DefaultOprt[] = 
  { 
    _T("<="), _T(">="),  _T("!="), 
    _T("=="), _T("<"),   _T(">"), 
    _T("+"),  _T("-"),   _T("*"), 
    _T("/"),  _T("^"),   _T("&&"), 
    _T("||"), _T("("),   _T(")"), 0 
  };

  // Infix operators
  ParserBase::SInfixOp ParserBase::s_defInfixOp[] = { { _T("-"), NEG },
                                                      nullptr };
  // Binary operators
  ParserBase::SBinOp ParserBase::s_defBinOp[] = { { _T("<="), LE,   oaLEFT,  prCMP     },
                                                  { _T(">="), GE,   oaLEFT,  prCMP     },
                                                  { _T("!="), NEQ,  oaLEFT,  prCMP     },
                                                  { _T("=="), EQ,   oaLEFT,  prCMP     },
                                                  { _T("<"),  LT,   oaLEFT,  prCMP     },
                                                  { _T(">"),  GT,   oaLEFT,  prCMP     },
                                                  { _T("+"),  ADD,  oaLEFT,  prADD_SUB },
                                                  { _T("-"),  SUB,  oaLEFT,  prADD_SUB },
                                                  { _T("*"),  MUL,  oaLEFT,  prMUL_DIV },
                                                  { _T("/"),  DIV,  oaLEFT,  prMUL_DIV },
                                                  { _T("^"),  POW,  oaRIGHT, prPOW     },
                                                  { _T("&&"), LAND, oaLEFT,  prLAND    },
                                                  { _T("||"), LOR,  oaLEFT,  prLOR     },
                                                  nullptr };

  // Functions
  ParserBase::SFunction ParserBase::s_defFun[] = { { _T("sin"),   SIN,   1 },
                                                   { _T("cos"),   COS,   1 },
                                                   { _T("tan"),   TAN,   1 },
                                                   { _T("asin"),  ASIN,  1 },
                                                   { _T("acos"),  ACOS,  1 },
                                                   { _T("atan"),  ATAN,  1 },
                                                   { _T("atan2"), ATAN2, 1 },
                                                   { _T("sinh"),  SINH,  1 },
                                                   { _T("cosh"),  COSH,  1 },
                                                   { _T("tanh"),  TANH,  1 },
                                                   { _T("asinh"), ASINH, 1 },
                                                   { _T("acosh"), ACOSH, 1 },
                                                   { _T("atanh"), ATANH, 1 },
                                                   { _T("log2"),  LOG2,  1 },
                                                   { _T("log10"), LOG10, 1 },
                                                   { _T("log"),   LOG10, 1 },
                                                   { _T("ln"),    LN,    1 },
                                                   { _T("exp"),   EXP,   1 },
                                                   { _T("sqrt"),  SQRT,  1 },
                                                   { _T("sign"),  SIGN,  1 },
                                                   { _T("rint"),  RINT,  1 },
                                                   { _T("abs"),   ABS,   1 },
                                                   nullptr };

  //------------------------------------------------------------------------------
  /** \brief Constructor.
      \param a_szFormula the formula to interpret.
      \throw ParserException if a_szFormula is nullptr.
  */
  ParserBase::ParserBase()
    :m_pParseFormula(&ParserBase::ParseString)
    ,m_vRPN()
    ,m_pTokenReader()
    ,m_FunDef()
    ,m_InfixOprtDef()
    ,m_ConstDef()
    ,m_VarDef()
    ,m_sNameChars()
    ,m_sInfixOprtChars()
    ,m_vStackBuffer()
    ,m_nFinalResultIdx(0)
  {
    InitTokenReader();
  }

  //---------------------------------------------------------------------------
  /** \brief Copy constructor. 

    Tha parser can be safely copy constructed but the bytecode is reset during
    copy construction.
  */
  ParserBase::ParserBase(const ParserBase &a_Parser)
    :m_pParseFormula(&ParserBase::ParseString)
    ,m_vRPN()
    ,m_pTokenReader()
    ,m_FunDef()
    ,m_InfixOprtDef()
    ,m_ConstDef()
    ,m_VarDef()
    ,m_sNameChars()
    ,m_sInfixOprtChars()
  {
    m_pTokenReader.reset(new token_reader_type(this));
    Assign(a_Parser);


  }

  //---------------------------------------------------------------------------
  ParserBase::~ParserBase()
  {}

  //---------------------------------------------------------------------------
  /** \brief Assignement operator. 

    Implemented by calling Assign(a_Parser). Self assignement is suppressed.
    \param a_Parser Object to copy to this.
    \return *this
    \throw nothrow
  */
  ParserBase& ParserBase::operator=(const ParserBase &a_Parser)
  {
    Assign(a_Parser);
    return *this;
  }

  //---------------------------------------------------------------------------
  /** \brief Copy state of a parser object to this. 

    Clears Variables and Functions of this parser.
    Copies the states of all internal variables.
    Resets parse function to string parse mode.

    \param a_Parser the source object.
  */
  void ParserBase::Assign(const ParserBase &a_Parser)
  {
    if (&a_Parser==this)
      return;

    // Don't copy bytecode instead cause the parser to create new bytecode
    // by resetting the parse function.
    ReInit();

    m_ConstDef        = a_Parser.m_ConstDef;         // Copy user define constants
    m_VarDef          = a_Parser.m_VarDef;           // Copy user defined variables
    m_vStackBuffer    = a_Parser.m_vStackBuffer;
    m_nFinalResultIdx = a_Parser.m_nFinalResultIdx;
    m_pTokenReader.reset(a_Parser.m_pTokenReader->Clone(this));

    // Copy function and operator callbacks
    m_FunDef = a_Parser.m_FunDef;             // Copy function definitions
    m_InfixOprtDef = a_Parser.m_InfixOprtDef; // unary operators for infix notation

    m_sNameChars = a_Parser.m_sNameChars;
    m_sInfixOprtChars = a_Parser.m_sInfixOprtChars;
  }

  //---------------------------------------------------------------------------
  /** \brief Initialize the token reader. 

    Create new token reader object and submit pointers to function, operator,
    constant and variable definitions.

    \post m_pTokenReader.get()!=0
    \throw nothrow
  */
  void ParserBase::InitTokenReader()
  {
    m_pTokenReader.reset(new token_reader_type(this));
  }

  //---------------------------------------------------------------------------
  /** \brief Reset parser to string parsing mode and clear internal buffers.

      Clear bytecode, reset the token reader.
      \throw nothrow
  */
  void ParserBase::ReInit() const
  {
    m_pParseFormula = &ParserBase::ParseString;
    m_vRPN.clear();
    m_pTokenReader->ReInit();
  }

  //---------------------------------------------------------------------------
  /** \brief Returns the version of muparser. 
      \param eInfo A flag indicating whether the full version info should be 
                   returned or not.

    Format is as follows: "MAJOR.MINOR (COMPILER_FLAGS)" The COMPILER_FLAGS
    are returned only if eInfo==pviFULL.
  */
  string_type ParserBase::GetVersion(EParserVersionInfo eInfo) const
  {
    string_type sCompileTimeSettings;
    
    stringstream_type ss;

    ss << MUP_VERSION;

    if (eInfo==pviFULL)
    {
      ss << _T(" (") << MUP_VERSION_DATE;
      ss << std::dec << _T("; ") << sizeof(void*)*8 << _T("BIT");

#ifdef _DEBUG
      ss << _T("; DEBUG");
#else 
      ss << _T("; RELEASE");
#endif

#ifdef _UNICODE
      ss << _T("; UNICODE");
#else
  #ifdef _MBCS
      ss << _T("; MBCS");
  #else
      ss << _T("; ASCII");
  #endif
#endif

#ifdef MUP_USE_OPENMP
      ss << _T("; OPENMP");
//#else
//      ss << _T("; NO_OPENMP");
#endif

      ss << _T(")");
    }

    return ss.str();
  }

  //---------------------------------------------------------------------------
  /** \brief Add a value parsing function. 
      
      When parsing an expression muParser tries to detect values in the expression
      string using different valident callbacks. Thuis it's possible to parse
      for hex values, binary values and floating point values. 
  */
  void ParserBase::AddValIdent(identfun_type a_pCallback)
  {
    m_pTokenReader->AddValIdent(a_pCallback);
  }

  //---------------------------------------------------------------------------
  /** \brief Add a function or operator callback to the parser. */
  void ParserBase::AddCallback( const string_type &a_strName,
                                const ParserCallback &a_Callback, 
                                funmap_type &a_Storage,
                                const char_type *a_szCharSet )
  {
    if (a_Callback.GetAddr()==0)
        Error(ecINVALID_FUN_PTR);

    const funmap_type *pFunMap = &a_Storage;

    // Check for conflicting operator or function names
    if ( pFunMap!=&m_FunDef && m_FunDef.find(a_strName)!=m_FunDef.end() )
      Error(ecNAME_CONFLICT, -1, a_strName);

    if ( pFunMap!=&m_InfixOprtDef && m_InfixOprtDef.find(a_strName)!=m_InfixOprtDef.end() )
      Error(ecNAME_CONFLICT, -1, a_strName);

    CheckOprt(a_strName, a_Callback, a_szCharSet);
    a_Storage[a_strName] = a_Callback;
    ReInit();
  }

  //---------------------------------------------------------------------------
  /** \brief Check if a name contains invalid characters. 

      \throw ParserException if the name contains invalid charakters.
  */
  void ParserBase::CheckOprt(const string_type &a_sName,
                             const ParserCallback &a_Callback,
                             const string_type &a_szCharSet) const
  {
    if ( !a_sName.length() ||
        (a_sName.find_first_not_of(a_szCharSet)!=string_type::npos) ||
        (a_sName[0]>='0' && a_sName[0]<='9'))
    {
      switch(a_Callback.GetCode())
      {
      case cmOPRT_INFIX:   Error(ecINVALID_INFIX_IDENT, -1, a_sName);
      default:             Error(ecINVALID_NAME, -1, a_sName);
      }
    }
  }

  //---------------------------------------------------------------------------
  /** \brief Check if a name contains invalid characters. 

      \throw ParserException if the name contains invalid charakters.
  */
  void ParserBase::CheckName(const string_type &a_sName,
                             const string_type &a_szCharSet) const
  {
    if ( !a_sName.length() ||
        (a_sName.find_first_not_of(a_szCharSet)!=string_type::npos) ||
        (a_sName[0]>='0' && a_sName[0]<='9'))
    {
      Error(ecINVALID_NAME);
    }
  }

  //---------------------------------------------------------------------------
  /** \brief Set the formula. 
      \param a_strFormula Formula as string_type
      \throw ParserException in case of syntax errors.

      Triggers first time calculation thus the creation of the bytecode and
      scanning of used variables.
  */
  void ParserBase::SetExpr(const string_type &a_sExpr)
  {
    // <ibg> 20060222: Bugfix for Borland-Kylix:
    // adding a space to the expression will keep Borlands KYLIX from going wild
    // when calling tellg on a stringstream created from the expression after 
    // reading a value at the end of an expression. (mu::Parser::IsVal function)
    // (tellg returns -1 otherwise causing the parser to ignore the value)
    string_type sBuf(a_sExpr + _T(" ") );
    m_pTokenReader->SetFormula(sBuf);
    ReInit();
  }

  //---------------------------------------------------------------------------
  /** \brief Get the default symbols used for the built in operators. 
      \sa c_DefaultOprt
  */
  const char_type** ParserBase::GetOprtDef() const
  {
    return (const char_type **)(&c_DefaultOprt[0]);
  }

  //---------------------------------------------------------------------------
  /** \brief Define the set of valid characters to be used in names of
             functions, variables, constants.
  */
  void ParserBase::DefineNameChars(const char_type *a_szCharset)
  {
    m_sNameChars = a_szCharset;
  }

  //---------------------------------------------------------------------------
  /** \brief Define the set of valid characters to be used in names of
             infix operators.
  */
  void ParserBase::DefineInfixOprtChars(const char_type *a_szCharset)
  {
    m_sInfixOprtChars = a_szCharset;
  }

  //---------------------------------------------------------------------------
  /** \brief Virtual function that defines the characters allowed in name identifiers. 
      \sa #ValidOprtChars, #ValidPrefixOprtChars
  */ 
  const char_type* ParserBase::ValidNameChars() const
  {
    assert(m_sNameChars.size());
    return m_sNameChars.c_str();
  }

  //---------------------------------------------------------------------------
  /** \brief Virtual function that defines the characters allowed in infix operator definitions.
      \sa #ValidNameChars, #ValidOprtChars
  */
  const char_type* ParserBase::ValidInfixOprtChars() const
  {
    assert(m_sInfixOprtChars.size());
    return m_sInfixOprtChars.c_str();
  }

  //---------------------------------------------------------------------------
  /** \brief Initialize user defined functions. 
   
    Calls the virtual functions InitFun(), InitConst().
  */
  void ParserBase::Init()
  {
    InitCharSets();
    InitFun();
    InitConst();
  }

  //---------------------------------------------------------------------------
  /** \brief Add a user defined operator. 
      \post Will reset the Parser to string parsing mode.
      \param [in] a_sName  operator Identifier 
      \param [in] a_pFun  Operator callback function
      \param [in] a_iPrec  Operator Precedence (default=prSIGN)
      \param [in] a_bAllowOpt  True if operator is volatile (default=false)
      \sa EPrec
  */
  void ParserBase::DefineInfixOprt(const string_type &a_sName, 
                                  fun_type1 a_pFun, 
                                  int a_iPrec, 
                                  bool a_bAllowOpt)
  {
    AddCallback(a_sName, 
                ParserCallback(a_pFun, a_bAllowOpt, a_iPrec, cmOPRT_INFIX), 
                m_InfixOprtDef, 
                ValidInfixOprtChars() );
  }

  //---------------------------------------------------------------------------
  /** \brief Add a user defined variable. 
      \param [in] a_sName the variable name
      \param [in] a_pVar A pointer to the variable vaule.
      \post Will reset the Parser to string parsing mode.
      \throw ParserException in case the name contains invalid signs or a_pVar is nullptr.
  */
  void ParserBase::DefineVar(const string_type &a_sName, value_type *a_pVar)
  {
    if (a_pVar==0)
      Error(ecINVALID_VAR_PTR);

    // Test if a constant with that names already exists
    if (m_ConstDef.find(a_sName)!=m_ConstDef.end())
      Error(ecNAME_CONFLICT);

    CheckName(a_sName, ValidNameChars());
    m_VarDef[a_sName] = a_pVar;
    ReInit();
  }

  //---------------------------------------------------------------------------
  /** \brief Add a user defined constant. 
      \param [in] a_sName The name of the constant.
      \param [in] a_fVal the value of the constant.
      \post Will reset the Parser to string parsing mode.
      \throw ParserException in case the name contains invalid signs.
  */
  void ParserBase::DefineConst(const string_type &a_sName, value_type a_fVal)
  {
    CheckName(a_sName, ValidNameChars());
    m_ConstDef[a_sName] = a_fVal;
    ReInit();
  }

  //---------------------------------------------------------------------------
  /** \brief Get operator priority.
      \throw ParserException if a_Oprt is no operator code
  */
  int ParserBase::GetOprtPrecedence(const token_type &a_Tok) const
  {
    switch (a_Tok.GetCode())
    {
    // built in operators
    case cmEND:      return -5;
    case cmARG_SEP:  return -4;
    case cmLAND:     return  prLAND;
    case cmLOR:      return  prLOR;
    case cmLT:
    case cmGT:
    case cmLE:
    case cmGE:
    case cmNEQ:
    case cmEQ:       return  prCMP; 
    case cmADD:
    case cmSUB:      return  prADD_SUB;
    case cmMUL:
    case cmDIV:      return  prMUL_DIV;
    case cmPOW:      return  prPOW;

    // user defined binary operators
    case cmOPRT_INFIX: 
                     return a_Tok.GetPri();
    default:  Error(ecINTERNAL_ERROR, 5);
              return 999;
    }  
  }

  //---------------------------------------------------------------------------
  /** \brief Get operator priority.
      \throw ParserException if a_Oprt is no operator code
  */
  EOprtAssociativity ParserBase::GetOprtAssociativity(const token_type &a_Tok) const
  {
    switch (a_Tok.GetCode())
    {
    case cmLAND:
    case cmLOR:
    case cmLT:
    case cmGT:
    case cmLE:
    case cmGE:
    case cmNEQ:
    case cmEQ: 
    case cmADD:
    case cmSUB:
    case cmMUL:
    case cmDIV:      return oaLEFT;
    case cmPOW:      return oaRIGHT;
    default:         return oaNONE;
    }  
  }


  //---------------------------------------------------------------------------
  /** \brief Return a map containing the used variables only. */
  const varmap_type& ParserBase::GetVar() const
  {
    return m_VarDef;
  }

  //---------------------------------------------------------------------------
  /** \brief Return a map containing all parser constants. */
  const valmap_type& ParserBase::GetConst() const
  {
    return m_ConstDef;
  }

  //---------------------------------------------------------------------------
  /** \brief Retrieve the formula. */
  const string_type& ParserBase::GetExpr() const
  {
    return m_pTokenReader->GetExpr();
  }

  //---------------------------------------------------------------------------
  /** \brief Apply a function token. 
      \param iArgCount Number of Arguments actually gathered used only for multiarg functions.
      \post The result is pushed to the value stack
      \post The function token is removed from the stack
      \throw exception_type if Argument count does not mach function requirements.
  */
  void ParserBase::ApplyFunc( ParserStack<token_type> &a_stOpt,
                              ParserStack<token_type> &a_stVal, 
                              int a_iArgCount) const
  { 
    assert(m_pTokenReader.get());

    // Operator stack empty or does not contain tokens with callback functions
    if (a_stOpt.empty() || a_stOpt.top().GetFuncAddr()==0 )
      return;

    token_type funTok = a_stOpt.pop();
    assert(funTok.GetFuncAddr());

    // Binary operators must rely on their internal operator number
    // since counting of operators relies on commas for function arguments
    // binary operators do not have commas in their expression
    int iArgCount = a_iArgCount;

    // determine how many parameters the function needs. To remember iArgCount includes the 
    // string parameter whilst GetArgCount() counts only numeric parameters.
    int iArgRequired = funTok.GetArgCount();

    // Thats the number of numerical parameters
    int iArgNumerical = iArgCount;

    if (funTok.GetArgCount()>=0 && iArgCount>iArgRequired) 
      Error(ecTOO_MANY_PARAMS, m_pTokenReader->GetPos()-1, funTok.GetAsString());

    if (iArgCount<iArgRequired )
      Error(ecTOO_FEW_PARAMS, m_pTokenReader->GetPos()-1, funTok.GetAsString());

    // Collect the numeric function arguments from the value stack and store them
    // in a vector
    std::vector<token_type> stArg;  
    for (int i=0; i<iArgNumerical; ++i)
    {
      stArg.push_back( a_stVal.pop() );
    }

    switch(funTok.GetCode())
    {
    case  cmFUNC_BULK: 
          m_vRPN.AddBulkFun(funTok.GetFuncAddr(), (int)stArg.size()); 
          break;

    case  cmOPRT_INFIX:
    case  cmFUNC:
          if (funTok.GetArgCount()==-1 && iArgCount==0)
            Error(ecTOO_FEW_PARAMS, m_pTokenReader->GetPos(), funTok.GetAsString());

          m_vRPN.AddFun(funTok.GetFuncAddr(), (funTok.GetArgCount()==-1) ? -iArgNumerical : iArgNumerical);
          break;
    }

    // Push dummy value representing the function result to the stack
    token_type token;
    token.SetVal(1);  
    a_stVal.push(token);
  }

  //---------------------------------------------------------------------------
  /** \brief Performs the necessary steps to write code for
             the execution of binary operators into the bytecode. 
  */
  void ParserBase::ApplyBinOprt(ParserStack<token_type> &a_stOpt,
                                ParserStack<token_type> &a_stVal) const
  {
    MUP_ASSERT(a_stVal.size()>=2);
    token_type valTok1 = a_stVal.pop(),
                valTok2 = a_stVal.pop(),
                optTok  = a_stOpt.pop(),
                resTok; 

    m_vRPN.AddOp(optTok.GetCode());

    resTok.SetVal(1);
    a_stVal.push(resTok);
  }

  //---------------------------------------------------------------------------
  /** \brief Apply a binary operator. 
      \param a_stOpt The operator stack
      \param a_stVal The value stack
  */
  void ParserBase::ApplyRemainingOprt(ParserStack<token_type> &stOpt,
                                      ParserStack<token_type> &stVal) const
  {
    while (stOpt.size() && 
           stOpt.top().GetCode() != cmBO)
    {
      token_type tok = stOpt.top();
      switch (tok.GetCode())
      {
      case cmOPRT_INFIX:
      case cmLE:
      case cmGE:
      case cmNEQ:
      case cmEQ:
      case cmLT:
      case cmGT:
      case cmADD:
      case cmSUB:
      case cmMUL:
      case cmDIV:
      case cmPOW:
      case cmLAND:
      case cmLOR:
          if (stOpt.top().GetCode()==cmOPRT_INFIX)
            ApplyFunc(stOpt, stVal, 1);
          else
            ApplyBinOprt(stOpt, stVal);
          break;

      default:
          Error(ecINTERNAL_ERROR);
      }
    }
  }

  //---------------------------------------------------------------------------
  /** \brief Parse the command code.
      \sa ParseString(...)

      Command code contains precalculated stack positions of the values and the
      associated operators. The Stack is filled beginning from index one the 
      value at index zero is not used at all.
  */
  value_type ParserBase::ParseCmdCode() const
  {
    return ParseCmdCodeBulk(0, 0);
  }

  //---------------------------------------------------------------------------
  /** \brief Evaluate the RPN. 
      \param nOffset The offset added to variable addresses (for bulk mode)
      \param nThreadID OpenMP Thread id of the calling thread
  */
  value_type ParserBase::ParseCmdCodeBulk(int nOffset, int nThreadID) const
  {
    assert(nThreadID<=s_MaxNumOpenMPThreads);

    // Note: The check for nOffset==0 and nThreadID here is not necessary but 
    //       brings a minor performance gain when not in bulk mode.
    value_type *Stack = ((nOffset==0) && (nThreadID==0)) ? &m_vStackBuffer[0] : &m_vStackBuffer[nThreadID * (m_vStackBuffer.size() / s_MaxNumOpenMPThreads)];
    int sidx(0);
    for (const SToken *pTok = m_vRPN.GetBase(); pTok->Cmd!=cmEND ; ++pTok)
    {
      switch (pTok->Cmd)
      {
      // built in binary operators
      case  cmLE:   --sidx; Stack[sidx]  = Stack[sidx] <= Stack[sidx+1]; continue;
      case  cmGE:   --sidx; Stack[sidx]  = Stack[sidx] >= Stack[sidx+1]; continue;
      case  cmNEQ:  --sidx; Stack[sidx]  = Stack[sidx] != Stack[sidx+1]; continue;
      case  cmEQ:   --sidx; Stack[sidx]  = Stack[sidx] == Stack[sidx+1]; continue;
      case  cmLT:   --sidx; Stack[sidx]  = Stack[sidx] < Stack[sidx+1];  continue;
      case  cmGT:   --sidx; Stack[sidx]  = Stack[sidx] > Stack[sidx+1];  continue;
      case  cmADD:  --sidx; Stack[sidx] += Stack[1+sidx]; continue;
      case  cmSUB:  --sidx; Stack[sidx] -= Stack[1+sidx]; continue;
      case  cmMUL:  --sidx; Stack[sidx] *= Stack[1+sidx]; continue;
      case  cmDIV:  --sidx; Stack[sidx] /= Stack[1+sidx]; continue;
      case  cmPOW: 
              --sidx; Stack[sidx] = MathImpl<value_type>::Pow(Stack[sidx], Stack[1+sidx]);
              continue;

      case  cmLAND: --sidx; Stack[sidx]  = Stack[sidx] && Stack[sidx+1]; continue;
      case  cmLOR:  --sidx; Stack[sidx]  = Stack[sidx] || Stack[sidx+1]; continue;

      // value and variable tokens
      case  cmVAR:    Stack[++sidx] = *(pTok->Val.ptr + nOffset);  continue;
      case  cmVAL:    Stack[++sidx] =  pTok->Val.data2;  continue;

      // Next is treatment of numeric functions
      case  cmFUNC:
            {
              int iArgCount = pTok->Fun.argc;

              // switch according to argument count
              switch(iArgCount)  
              {
              case 0: sidx += 1; Stack[sidx] = (*(fun_type0)pTok->Fun.ptr)(); continue;
              case 1:            Stack[sidx] = (*(fun_type1)pTok->Fun.ptr)(Stack[sidx]);   continue;
              case 2: sidx -= 1; Stack[sidx] = (*(fun_type2)pTok->Fun.ptr)(Stack[sidx], Stack[sidx+1]); continue;
              case 3: sidx -= 2; Stack[sidx] = (*(fun_type3)pTok->Fun.ptr)(Stack[sidx], Stack[sidx+1], Stack[sidx+2]); continue;
              default:
                Error(ecINTERNAL_ERROR, 1);
              }
            }

         case  cmFUNC_BULK:
              {
                int iArgCount = pTok->Fun.argc;

                // switch according to argument count
                switch(iArgCount)  
                {
                case 0: sidx += 1; Stack[sidx] = (*(bulkfun_type0 )pTok->Fun.ptr)(nOffset, nThreadID); continue;
                case 1:            Stack[sidx] = (*(bulkfun_type1 )pTok->Fun.ptr)(nOffset, nThreadID, Stack[sidx]); continue;
                case 2: sidx -= 1; Stack[sidx] = (*(bulkfun_type2 )pTok->Fun.ptr)(nOffset, nThreadID, Stack[sidx], Stack[sidx+1]); continue;
                case 3: sidx -= 2; Stack[sidx] = (*(bulkfun_type3 )pTok->Fun.ptr)(nOffset, nThreadID, Stack[sidx], Stack[sidx+1], Stack[sidx+2]); continue;
                default:
                  Error(ecINTERNAL_ERROR, 2);
                  continue;
                }
              }

        default:
              Error(ecINTERNAL_ERROR, 3);
              return 0;
      } // switch CmdCode
    } // for all bytecode tokens

    return Stack[m_nFinalResultIdx];  
  }

  //---------------------------------------------------------------------------
  void ParserBase::CreateRPN() const
  {
    if (!m_pTokenReader->GetExpr().length())
      Error(ecUNEXPECTED_EOF, 0);

    ParserStack<token_type> stOpt, stVal;
    ParserStack<int> stArgCount;
    token_type opta, opt;  // for storing operators
    token_type val, tval;  // for storing value
    string_type strBuf;    // buffer for string function arguments

    ReInit();
    
    // The outermost counter counts the number of seperated items
    // such as in "a=10,b=20,c=c+a"
    stArgCount.push(1);
    
    for(;;)
    {
      opt = m_pTokenReader->ReadNextToken();

      switch (opt.GetCode())
      {
        //
        // Next three are different kind of value entries
        //
        case cmVAR:
                stVal.push(opt);
                m_vRPN.AddVar( static_cast<value_type*>(opt.GetVar()) );
                break;

        case cmVAL:
		            stVal.push(opt);
                m_vRPN.AddVal( opt.GetVal() );
                break;

        case cmARG_SEP:
                if (stArgCount.empty())
                  Error(ecUNEXPECTED_ARG_SEP, m_pTokenReader->GetPos());

                ++stArgCount.top();
                // fallthrough intentional (no break!)

        case cmEND:
                ApplyRemainingOprt(stOpt, stVal);
                break;

       case cmBC:
                {
                  // The argument count for parameterless functions is zero
                  // by default an opening bracket sets parameter count to 1
                  // in preparation of arguments to come. If the last token
                  // was an opening bracket we know better...
                  if (opta.GetCode()==cmBO)
                    --stArgCount.top();
                  
                  ApplyRemainingOprt(stOpt, stVal);

                  // Check if the bracket content has been evaluated completely
                  if (stOpt.size() && stOpt.top().GetCode()==cmBO)
                  {
                    // if opt is ")" and opta is "(" the bracket has been evaluated, now its time to check
                    // if there is either a function or a sign pending
                    // neither the opening nor the closing bracket will be pushed back to
                    // the operator stack
                    // Check if a function is standing in front of the opening bracket, 
                    // if yes evaluate it afterwards check for infix operators
                    assert(stArgCount.size());
                    int iArgCount = stArgCount.pop();
                    
                    stOpt.pop(); // Take opening bracket from stack

                    if (iArgCount>1 && ( stOpt.size()==0 || 
                                        (stOpt.top().GetCode()!=cmFUNC && 
                                         stOpt.top().GetCode()!=cmFUNC_BULK) ) )
                      Error(ecUNEXPECTED_ARG, m_pTokenReader->GetPos());
                    
                    // The opening bracket was popped from the stack now check if there
                    // was a function before this bracket
                    if (stOpt.size() && 
                        stOpt.top().GetCode()!=cmOPRT_INFIX && 
                        stOpt.top().GetFuncAddr()!=0)
                    {
                      ApplyFunc(stOpt, stVal, iArgCount);
                    }
                  }
                } // if bracket content is evaluated
                break;

        //
        // Next are the binary operator entries
        //
        case cmLAND:
        case cmLOR:
        case cmLT:
        case cmGT:
        case cmLE:
        case cmGE:
        case cmNEQ:
        case cmEQ:
        case cmADD:
        case cmSUB:
        case cmMUL:
        case cmDIV:
        case cmPOW:
                // A binary operator (user defined or built in) has been found. 
                while ( stOpt.size() && stOpt.top().GetCode() != cmBO)
                {
                  int nPrec1 = GetOprtPrecedence(stOpt.top()),
                      nPrec2 = GetOprtPrecedence(opt);

                  if (stOpt.top().GetCode()==opt.GetCode())
                  {

                    // Deal with operator associativity
                    EOprtAssociativity eOprtAsct = GetOprtAssociativity(opt);
                    if ( (eOprtAsct==oaRIGHT && (nPrec1 <= nPrec2)) || 
                         (eOprtAsct==oaLEFT  && (nPrec1 <  nPrec2)) )
                    {
                      break;
                    }
                  }
                  else if (nPrec1 < nPrec2)
                  {
                    // In case the operators are not equal the precedence decides alone...
                    break;
                  }
                  
                  if (stOpt.top().GetCode()==cmOPRT_INFIX)
                    ApplyFunc(stOpt, stVal, 1);
                  else
                    ApplyBinOprt(stOpt, stVal);
                } // while ( ... )

                // The operator can't be evaluated right now, push back to the operator stack
                stOpt.push(opt);
                break;

        //
        // Last section contains functions and operators implicitely mapped to functions
        //
        case cmBO:
                stArgCount.push(1);
                stOpt.push(opt);
                break;

        case cmOPRT_INFIX:
        case cmFUNC:
        case cmFUNC_BULK:
                stOpt.push(opt);
                break;

        default:  Error(ecINTERNAL_ERROR, 3);
      } // end of switch operator-token

      opta = opt;

      if ( opt.GetCode() == cmEND )
      {
        m_vRPN.Finalize();
        break;
      }

      if (ParserBase::g_DbgDumpStack)
      {
        StackDump(stVal, stOpt);
        m_vRPN.AsciiDump();
      }
    } // while (true)

    if (ParserBase::g_DbgDumpCmdCode)
      m_vRPN.AsciiDump();

    // get the last value (= final result) from the stack
    MUP_ASSERT(stArgCount.size()==1);
    m_nFinalResultIdx = stArgCount.top();
    if (m_nFinalResultIdx==0)
      Error(ecINTERNAL_ERROR, 9);

    if (stVal.size()==0)
      Error(ecEMPTY_EXPRESSION);

    m_vStackBuffer.resize(m_vRPN.GetMaxStackSize() * s_MaxNumOpenMPThreads);
  }

  //---------------------------------------------------------------------------
  /** \brief One of the two main parse functions.
      \sa ParseCmdCode(...)

    Parse expression from input string. Perform syntax checking and create 
    bytecode. After parsing the string and creating the bytecode the function 
    pointer #m_pParseFormula will be changed to the second parse routine the 
    uses bytecode instead of string parsing.
  */
  value_type ParserBase::ParseString() const
  {
    try
    {
      CreateRPN();
      m_pParseFormula = &ParserBase::ParseCmdCode;
      return (this->*m_pParseFormula)(); 
    }
    catch(ParserError &exc)
    {
      exc.SetFormula(m_pTokenReader->GetExpr());
      throw;
    }
  }

  //---------------------------------------------------------------------------
  /** \brief Create an error containing the parse error position.

    This function will create an Parser Exception object containing the error text and
    its position.

    \param a_iErrc [in] The error code of type #EErrorCodes.
    \param a_iPos [in] The position where the error was detected.
    \param a_strTok [in] The token string representation associated with the error.
    \throw ParserException always throws thats the only purpose of this function.
  */
  void  ParserBase::Error(EErrorCodes a_iErrc, int a_iPos, const string_type &a_sTok) const
  {
    throw exception_type(a_iErrc, a_sTok, m_pTokenReader->GetExpr(), a_iPos);
  }

  //------------------------------------------------------------------------------
  /** \brief Clear all user defined variables.
      \throw nothrow

      Resets the parser to string parsing mode by calling #ReInit.
  */
  void ParserBase::ClearVar()
  {
    m_VarDef.clear();
    ReInit();
  }

  //------------------------------------------------------------------------------
  /** \brief Remove a variable from internal storage.
      \throw nothrow

      Removes a variable if it exists. If the Variable does not exist nothing will be done.
  */
  void ParserBase::RemoveVar(const string_type &a_strVarName)
  {
    varmap_type::iterator item = m_VarDef.find(a_strVarName);
    if (item!=m_VarDef.end())
    {
      m_VarDef.erase(item);
      ReInit();
    }
  }

  //------------------------------------------------------------------------------
  /** \brief Clear all functions.
      \post Resets the parser to string parsing mode.
      \throw nothrow
  */
  void ParserBase::ClearFun()
  {
    m_FunDef.clear();
    ReInit();
  }

  //------------------------------------------------------------------------------
  /** \brief Clear all user defined constants.

      Both numeric and string constants will be removed from the internal storage.
      \post Resets the parser to string parsing mode.
      \throw nothrow
  */
  void ParserBase::ClearConst()
  {
    m_ConstDef.clear();
    ReInit();
  }

  //------------------------------------------------------------------------------
  /** \brief Clear the user defined Prefix operators. 
      \post Resets the parser to string parser mode.
      \throw nothrow
  */
  void ParserBase::ClearInfixOprt()
  {
    m_InfixOprtDef.clear();
    ReInit();
  }

  //---------------------------------------------------------------------------
  /** \brief Enable the dumping of bytecode amd stack content on the console. 
      \param bDumpCmd Flag to enable dumping of the current bytecode to the console.
      \param bDumpStack Flag to enable dumping of the stack content is written to the console.

     This function is for debug purposes only!
  */
  void ParserBase::EnableDebugDump(bool bDumpCmd, bool bDumpStack)
  {
    ParserBase::g_DbgDumpCmdCode = bDumpCmd;
    ParserBase::g_DbgDumpStack   = bDumpStack;
  }

  //------------------------------------------------------------------------------
  /** \brief Dump stack content. 

      This function is used for debugging only.
  */
  void ParserBase::StackDump(const ParserStack<token_type> &a_stVal, 
                             const ParserStack<token_type> &a_stOprt) const
  {
    ParserStack<token_type> stOprt(a_stOprt), 
                            stVal(a_stVal);

    mu::console() << _T("\nValue stack:\n");
    while ( !stVal.empty() ) 
    {
      token_type val = stVal.pop();
      mu::console() << _T(" ") << val.GetVal() << _T(" ");
    }
    mu::console() << "\nOperator stack:\n";

    while ( !stOprt.empty() )
    {
      if (stOprt.top().GetCode()<=cmBO) 
      {
        mu::console() << _T("OPRT_INTRNL \"")
                      << ParserBase::c_DefaultOprt[stOprt.top().GetCode()] 
                      << _T("\" \n");
      }
      else
      {
        switch(stOprt.top().GetCode())
        {
        case cmVAR:   mu::console() << _T("VAR\n");  break;
        case cmVAL:   mu::console() << _T("VAL\n");  break;
        case cmFUNC:  mu::console() << _T("FUNC \"") 
                                    << stOprt.top().GetAsString() 
                                    << _T("\"\n");   break;
        case cmFUNC_BULK:  mu::console() << _T("FUNC_BULK \"") 
                                         << stOprt.top().GetAsString() 
                                         << _T("\"\n");   break;
        case cmOPRT_INFIX: mu::console() << _T("OPRT_INFIX \"")
                                         << stOprt.top().GetAsString() 
                                         << _T("\"\n");      break;
        case cmEND:      mu::console() << _T("END\n");            break;
        case cmUNKNOWN:  mu::console() << _T("UNKNOWN\n");        break;
        case cmBO:       mu::console() << _T("BRACKET \"(\"\n");  break;
        case cmBC:       mu::console() << _T("BRACKET \")\"\n");  break;
        default:         mu::console() << stOprt.top().GetCode() << _T(" ");  break;
        }
      }	
      stOprt.pop();
    }

    mu::console() << dec << endl;
  }

  //------------------------------------------------------------------------------
  /** \brief Evaluate an expression containing comma seperated subexpressions 
      \param [out] nStackSize The total number of results available
      \return Pointer to the array containing all expression results

      This member function can be used to retriev all results of an expression
      made up of multiple comma seperated subexpressions (i.e. "x+y,sin(x),cos(y)")
  */
  value_type* ParserBase::Eval(int &nStackSize) const
  {
    (this->*m_pParseFormula)(); 
    nStackSize = m_nFinalResultIdx;

    // (for historic reasons the stack starts at position 1)
    return &m_vStackBuffer[1];
  }

  //---------------------------------------------------------------------------
  /** \brief Return the number of results on the calculation stack. 
  
    If the expression contains comma seperated subexpressions (i.e. "sin(y), x+y"). 
    There mey be more than one return value. This function returns the number of 
    available results.
  */
  int ParserBase::GetNumResults() const
  {
    return m_nFinalResultIdx;
  }

  //---------------------------------------------------------------------------
  /** \brief Calculate the result.

    A note on const correctness: 
    I consider it important that Calc is a const function.
    Due to caching operations Calc changes only the state of internal variables with one exception
    m_UsedVar this is reset during string parsing and accessible from the outside. Instead of making
    Calc non const GetUsedVar is non const because it explicitely calls Eval() forcing this update. 

    \pre A formula must be set.
    \pre Variables must have been set (if needed)

    \sa #m_pParseFormula
    \return The evaluation result
    \throw ParseException if no Formula is set or in case of any other error related to the formula.
  */
  value_type ParserBase::Eval() const
  {
    return (this->*m_pParseFormula)(); 
  }

  //---------------------------------------------------------------------------
  void ParserBase::Eval(value_type *results, int nBulkSize)
  {
    CreateRPN();

    int i = 0;

#ifdef MUP_USE_OPENMP
//#define DEBUG_OMP_STUFF
    #ifdef DEBUG_OMP_STUFF
    int *pThread = new int[nBulkSize];
    int *pIdx = new int[nBulkSize];
    #endif

    int nMaxThreads = std::min(omp_get_max_threads(), s_MaxNumOpenMPThreads);
    int nThreadID, ct=0;
    omp_set_num_threads(nMaxThreads);

    #pragma omp parallel for schedule(static, nBulkSize/nMaxThreads) private(nThreadID)
    for (i=0; i<nBulkSize; ++i)
    {
      nThreadID = omp_get_thread_num();
      results[i] = ParseCmdCodeBulk(i, nThreadID);

      #ifdef DEBUG_OMP_STUFF
      #pragma omp critical
      {
        pThread[ct] = nThreadID;  
        pIdx[ct] = i; 
        ct++;
      }
      #endif
    }

#ifdef DEBUG_OMP_STUFF
    FILE *pFile = fopen("bulk_dbg.txt", "w");
    for (i=0; i<nBulkSize; ++i)
    {
      fprintf(pFile, "idx: %d  thread: %d \n", pIdx[i], pThread[i]);
    }
    
    delete [] pIdx;
    delete [] pThread;

    fclose(pFile);
#endif

#else
    for (i=0; i<nBulkSize; ++i)
    {
      results[i] = ParseCmdCodeBulk(i, 0);
    }
#endif

  }
} // namespace mu

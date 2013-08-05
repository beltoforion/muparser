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

#include "mecParserBase.h"

//--- Standard includes ------------------------------------------------------------------
#include <cassert>
#include <cmath>
#include <memory>
#include <vector>
#include <deque>
#include <sstream>
#include <locale>

//--- Parser framework -------------------------------------------------------------------
#include "mecDef.h"
#include "mecExprCompiler.h"

using namespace std;
using namespace AsmJit;

/** \file
    \brief This file contains the basic implementation of the muparser engine.
*/

namespace mec
{
  bool g_DbgDumpCmdCode = false;
  bool g_DbgDumpStack = false;

  //------------------------------------------------------------------------------
  /** \brief Identifiers for built in binary operators. 

      When defining custom binary operators with #AddOprt(...) make sure not to choose 
      names conflicting with these definitions. 
  */
  const char_type* ParserBase::c_DefaultOprt[] = 
  { 
    _T("<?"),
    _T(">?"),
    _T("<="), 
    _T(">="),  
    _T("!="), 
    _T("=="), 
    _T("<"),  
    _T(">"), 
    _T("&&"), 
    _T("||"), 
    _T("+"),  
    _T("-"),   
    _T("*"), 
    _T("/"),
//    _T("%"),
    _T("sin"),
    _T("cos"),
    _T("tan"),
    _T("abs"),
    _T("sqrt"),
    _T("("),  
    _T(")"), 
    _T("?"),  
    _T(":"),
    0 
  };

  //------------------------------------------------------------------------------
  /** \brief Constructor.
      \param a_szFormula the formula to interpret.
      \throw ParserException if a_szFormula is null.
  */
  ParserBase::ParserBase()
    :m_pParseFormula(&ParserBase::ParseString)
    ,m_vByteCode()
    ,m_pTokenReader()
    ,m_FunDef()
    ,m_PostOprtDef()
    ,m_InfixOprtDef()
    ,m_OprtDef()
    ,m_ConstDef()
    ,m_StrVarDef()
    ,m_VarDef()
    ,m_bOptimize(true)
    ,m_eEngine(peJIT)
    ,m_sNameChars()
    ,m_sOprtChars()
    ,m_sInfixOprtChars()
    ,m_nIfElseCounter(0)
    ,m_vStackBuffer()
    ,m_pStackZero(NULL)
    ,m_compiler()
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
    ,m_vByteCode()
    ,m_pTokenReader()
    ,m_FunDef()
    ,m_PostOprtDef()
    ,m_InfixOprtDef()
    ,m_OprtDef()
    ,m_ConstDef()
    ,m_StrVarDef()
    ,m_VarDef()
    ,m_bOptimize(true)
    ,m_eEngine(peJIT)
    ,m_sNameChars()
    ,m_sOprtChars()
    ,m_sInfixOprtChars()
    ,m_nIfElseCounter(0)
    ,m_compiler()
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

    m_ConstDef        = a_Parser.m_ConstDef;    // Copy user define constants
    m_VarDef          = a_Parser.m_VarDef;      // Copy user defined variables
    m_eEngine         = a_Parser.m_eEngine;
    m_StrVarDef       = a_Parser.m_StrVarDef;
    m_bOptimize       = a_Parser.m_bOptimize;
    m_nIfElseCounter  = a_Parser.m_nIfElseCounter;
    m_pTokenReader.reset(a_Parser.m_pTokenReader->Clone(this));

    // Copy function and operator callbacks
    m_FunDef          = a_Parser.m_FunDef;       // Copy function definitions
    m_PostOprtDef     = a_Parser.m_PostOprtDef;  // post value unary operators
    m_InfixOprtDef    = a_Parser.m_InfixOprtDef; // unary operators for infix notation
    m_OprtDef         = a_Parser.m_OprtDef;      // binary operators

    m_sNameChars      = a_Parser.m_sNameChars;
    m_sOprtChars      = a_Parser.m_sOprtChars;
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
  void ParserBase::ReInit() 
  {
    m_pCompiledFun = NULL;
    m_pParseFormula = &ParserBase::ParseString;
    m_vByteCode.clear();
    m_pTokenReader->ReInit();
    m_nIfElseCounter = 0;
  }

  //------------------------------------------------------------------------------
  /** \brief Enable or disable the formula optimization feature. 
      \post Resets the parser to string parser mode.
      \throw nothrow
  */
  void ParserBase::EnableOptimizer(bool a_bIsOn)
  {
    m_bOptimize = a_bIsOn;
    ReInit();
  }

  //---------------------------------------------------------------------------
  /** \brief Returns the version of muparser. 
  
    Format is as follows: "MAJOR.MINOR (OPTIONAL TEXT)"
  */
  string_type ParserBase::GetVersion() const
  {
    return MEC_VERSION;
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
  /** \brief Set a function that can create variable pointer for unknown expression variables. 
      \param a_pFactory A pointer to the variable factory.
      \param pUserData A user defined context pointer.
  */
  void ParserBase::SetVarFactory(facfun_type a_pFactory, void *pUserData)
  {
    m_pTokenReader->SetVarCreator(a_pFactory, pUserData);  
  }

  //---------------------------------------------------------------------------
  /** \brief Add a function or operator callback to the parser. */
  void ParserBase::AddCallback( const string_type &a_strName,
                                const Callback &a_Callback, 
                                funmap_type &a_Storage,
                                const char_type *a_szCharSet )
  {
    if (a_Callback.m_pFun==0)
        Error(ecINVALID_FUN_PTR);

    const funmap_type *pFunMap = &a_Storage;

    // Check for conflicting operator or function names
    if ( pFunMap!=&m_FunDef && m_FunDef.find(a_strName)!=m_FunDef.end() )
      Error(ecNAME_CONFLICT);

    if ( pFunMap!=&m_PostOprtDef && m_PostOprtDef.find(a_strName)!=m_PostOprtDef.end() )
      Error(ecNAME_CONFLICT);

    if ( pFunMap!=&m_InfixOprtDef && pFunMap!=&m_OprtDef && m_InfixOprtDef.find(a_strName)!=m_InfixOprtDef.end() )
      Error(ecNAME_CONFLICT);

    if ( pFunMap!=&m_InfixOprtDef && pFunMap!=&m_OprtDef && m_OprtDef.find(a_strName)!=m_OprtDef.end() )
      Error(ecNAME_CONFLICT);

    CheckName(a_strName, a_szCharSet);
    a_Storage[a_strName] = a_Callback;
    ReInit();
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
    // Check locale compatibility
    std::locale loc;
    if (m_pTokenReader->GetArgSep()==std::use_facet<numpunct<char_type> >(loc).decimal_point())
      Error(ecLOCALE);

    // <ibg> 20060222: Bugfix for Borland-Kylix:
    // adding a space to the expression will keep Borlands KYLIX from going wild
    // when calling tellg on a stringstream created from the expression after 
    // reading a value at the end of an expression. (mu::Parser::IsVal function)
    // (tellg returns -1 otherwise causing the parser to ignore the value)
    string_type sBuf(a_sExpr + _T(" ") );
    m_pTokenReader->SetExpr(sBuf);
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
             binary operators and postfix operators.
  */
  void ParserBase::DefineOprtChars(const char_type *a_szCharset)
  {
    m_sOprtChars = a_szCharset;
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
  /** \brief Virtual function that defines the characters allowed in operator definitions. 
      \sa #ValidNameChars, #ValidPrefixOprtChars
  */
  const char_type* ParserBase::ValidOprtChars() const
  {
    assert(m_sOprtChars.size());
    return m_sOprtChars.c_str();
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
  /** \brief Add a user defined operator. 
      \post Will reset the Parser to string parsing mode.
  */
  void ParserBase::DefinePostfixOprt(const string_type &a_sName, 
                                     fun_type1 a_pFun, 
                                     int flags)
  {
    AddCallback(a_sName, 
                Callback(a_pFun, prPOSTFIX, cmOPRT_POSTFIX),
                m_PostOprtDef, 
                ValidOprtChars() );
  }

  //---------------------------------------------------------------------------
  /** \brief Initialize user defined functions. 
   
    Calls the virtual functions InitFun(), InitConst() and InitOprt().
  */
  void ParserBase::Init()
  {
    InitCharSets();
    InitFun();
    InitConst();
    InitOprt();
  }

  //---------------------------------------------------------------------------
  /** \brief Add a user defined operator. 
      \post Will reset the Parser to string parsing mode.
      \param [in] a_sName  operator Identifier 
      \param [in] a_pFun  Operator callback function
      \param [in] a_iPrec  Operator Precedence (default=prSIGN)
      \sa EPrec
  */
  void ParserBase::DefineInfixOprt(const string_type &a_sName, 
                                   fun_type1 a_pFun, 
                                   int a_nPrec)
  {
    Callback cb;
    cb.m_pFun1     = a_pFun;
    cb.m_nPrec     = a_nPrec;
    cb.m_nArgc     = 1;
    AddCallback(a_sName, cb, m_InfixOprtDef, ValidOprtChars());
    //AddCallback(a_sName, 
    //            Callback(a_pFun, a_iPrec), 
    //            m_InfixOprtDef, 
    //            ValidOprtChars() );
  }


  //---------------------------------------------------------------------------
  /** \brief Define a binary operator. 
      \param [in] a_pFun Pointer to the callback function.
      \param [in] a_iPrec Precedence of the operator.
  */
  void ParserBase::DefineOprt(const string_type &a_sName, 
                              fun_type2 a_pFun, 
                              unsigned a_iPrec, 
                              EOprtAssociativity eAsc,
                              int flags)
  {
    // Check for conflicts with built in operator names
    for (int i=0; i<cmENDIF; ++i)
      if (a_sName == string_type(c_DefaultOprt[i]))
        Error(ecBUILTIN_OVERLOAD, -1, a_sName);

    AddCallback(a_sName, 
                Callback(a_pFun, a_iPrec, eAsc), 
                m_OprtDef, 
                ValidOprtChars() );
  }

  //---------------------------------------------------------------------------
  /** \brief Add a user defined variable. 
      \param [in] a_sName the variable name
      \param [in] a_pVar A pointer to the variable vaule.
      \post Will reset the Parser to string parsing mode.
      \pre [assert] a_fVar!=0
      \throw ParserException in case the name contains invalid signs.
  */
  void ParserBase::DefineVar(const string_type &a_sName, value_type *a_pVar)
  {
    if (a_pVar==0)
      Error(ecINVALID_VAR_PTR);

    // Test if a constant with that names already exists
    if (m_ConstDef.find(a_sName)!=m_ConstDef.end())
      Error(ecNAME_CONFLICT);

//    if (m_FunDef.find(a_sName)!=m_FunDef.end())
//      Error(ecNAME_CONFLICT);

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
  EOprtAssociativity ParserBase::GetOprtAssociativity(const Token &a_Tok) const
  {
    switch (a_Tok.GetCode())
    {
    case cmMIN:
    case cmMAX:
    case cmLT:
    case cmGT:
    case cmLE:
    case cmGE:
    case cmNEQ:
    case cmEQ: 
    case cmAND:
    case cmOR:
    case cmADD:
    case cmSUB:
    case cmMUL:
//    case cmMOD:
    case cmDIV:      return oaLEFT;

#ifdef MEC_INTRINSIC_POW
    case cmPOW:      return oaRIGHT;
#endif

    case cmOPRT_BIN: return a_Tok.GetCallback()->m_eOprtAsct;
    default:         return oaNONE;
    }  
  }

  //---------------------------------------------------------------------------
  /** \brief Get operator priority.
      \throw ParserException if a_Oprt is no operator code
  */
  int ParserBase::GetOprtPrecedence(const Token &a_Tok) const
  {
    switch (a_Tok.GetCode())
    {
    // built in operators
    case cmEND:      return -5;
    case cmARG_SEP:  return -4;
    case cmBO :	
    case cmBC :      return -2;

    case cmAND:
    case cmOR:       return  prLOGIC;  

    case cmMAX:
    case cmMIN:      
    case cmLT:
    case cmGT:
    case cmLE:
    case cmGE:
    case cmNEQ:
    case cmEQ:       return  prCMP; 
    case cmADD:
    case cmSUB:      return  prADD_SUB;
    case cmMUL:
//    case cmMOD:
    case cmDIV:      return  prMUL_DIV;

#ifdef MEC_INTRINSIC_POW
    case cmPOW:      return  prPOW;
#endif

    // user defined binary operators
    case cmOPRT_INFIX: 
    case cmOPRT_BIN: return a_Tok.GetCallback()->m_nPrec;
    default:  Error(ecINTERNAL_ERROR, 5);
              return 999;
    }  
  }

  //---------------------------------------------------------------------------
  /** \brief Return a map containing the used variables only. */
  const varmap_type& ParserBase::GetUsedVar() 
  {
    try
    {
      m_pTokenReader->IgnoreUndefVar(true);
      ParseString(); // implicitely create or update the map with the
                     // used variables stored in the token reader if not already done
      m_pTokenReader->IgnoreUndefVar(false);
    }
    catch(exception_type &e)
    {
      m_pTokenReader->IgnoreUndefVar(false);
      throw e;
    }
    
    // Make sure to stay in string parse mode, dont call ReInit()
    // because it deletes the array with the used variables
    m_pParseFormula = &ParserBase::ParseString;
    
    return m_pTokenReader->GetUsedVar();
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
  /** \brief Return prototypes of all parser functions.
      \return #m_FunDef
      \sa FunProt
      \throw nothrow
      
      The return type is a map of the public type #funmap_type containing the prototype
      definitions for all numerical parser functions. String functions are not part of 
      this map. The Prototype definition is encapsulated in objects of the class FunProt
      one per parser function each associated with function names via a map construct.
  */
  const funmap_type& ParserBase::GetFunDef() const
  {
    return m_FunDef;
  }

  //---------------------------------------------------------------------------
  /** \brief Retrieve the formula. */
  const string_type& ParserBase::GetExpr() const
  {
    return m_pTokenReader->GetFormula();
  }

  //---------------------------------------------------------------------------
  Token ParserBase::ApplyNumFunc(const Token &a_FunTok,
                                 const std::vector<Token> &a_vArg) 
  {
    Token  valTok;
    int  iArgCount = (unsigned)a_vArg.size();

    Callback *pCallback = a_FunTok.GetCallback();
    if (pCallback)
    {
      // Collect the function arguments from the value stack
      switch(pCallback->m_nArgc)
      {
      case 0: valTok.SetVal( pCallback->m_pFun0() );  break;
      case 1: valTok.SetVal( pCallback->m_pFun1(a_vArg[0].GetVal()) );  break;
      case 2: valTok.SetVal( pCallback->m_pFun2(a_vArg[1].GetVal(),
                                                a_vArg[0].GetVal()) );  break;
      case 3: valTok.SetVal( pCallback->m_pFun3(a_vArg[2].GetVal(), 
                                                a_vArg[1].GetVal(), 
                                                a_vArg[0].GetVal()) ); break;
      case 4: valTok.SetVal( pCallback->m_pFun4(a_vArg[3].GetVal(),
                                                a_vArg[2].GetVal(), 
                                                a_vArg[1].GetVal(),
                                                a_vArg[0].GetVal()) );  break;
      case 5: valTok.SetVal( pCallback->m_pFun5(a_vArg[4].GetVal(), 
                                                a_vArg[3].GetVal(), 
                                                a_vArg[2].GetVal(), 
                                                a_vArg[1].GetVal(),
                                                a_vArg[0].GetVal()) );  break;
      default: Error(ecINTERNAL_ERROR, 6);
      }

      // Find out if the result will depend on a variable
      bool bVolatile = a_FunTok.IsFlagSet(flVOLATILE);
      for (int i=0; (bVolatile==false) && (i<iArgCount); ++i)
        bVolatile |= a_vArg[i].IsFlagSet(flVOLATILE);

      if (bVolatile)
        valTok.AddFlags(flVOLATILE);

      // Formula optimization
      if ( m_bOptimize && 
          !valTok.IsFlagSet(flVOLATILE) &&
          !a_FunTok.IsFlagSet(flVOLATILE) ) 
      {
        m_vByteCode.RemoveValEntries(iArgCount);
        m_vByteCode.AddVal( valTok.GetVal() );
      }
      else 
      { 
        // operation dosnt depends on a variable or the function is flagged unoptimizeable
        // we cant optimize here...
        m_vByteCode.AddFun(pCallback->m_pFun, iArgCount);
      }
    }
    else
    {
      // intrinsic function
      switch (a_FunTok.GetCode())
      {
      case cmSIN:  valTok.SetVal( sin(a_vArg[0].GetVal()) );  break;
      case cmCOS:  valTok.SetVal( cos(a_vArg[0].GetVal()) );  break;
      case cmTAN:  valTok.SetVal( tan(a_vArg[0].GetVal()) );  break;
      case cmABS:  valTok.SetVal( fabs(a_vArg[0].GetVal()) );  break;
      case cmSQRT: valTok.SetVal( sqrt(a_vArg[0].GetVal()) );  break;
      default: Error(ecINTERNAL_ERROR, 7);
      }

      // Find out if the result will depend on a variable
      if (a_vArg[0].IsFlagSet(flVOLATILE))
        valTok.AddFlags(flVOLATILE);

      if ( m_bOptimize && !valTok.IsFlagSet(flVOLATILE) ) 
      {
        m_vByteCode.RemoveValEntries(1);
        m_vByteCode.AddVal(valTok.GetVal());
      }
      else 
      { 
        m_vByteCode.AddFun(a_FunTok.GetCode());
      }
    }

    return valTok;
  }

  //---------------------------------------------------------------------------
  /** \brief Apply a function token. 
      \param iArgCount Number of Arguments actually gathered used only for multiarg functions.
      \post The result is pushed to the value stack
      \post The function token is removed from the stack
      \throw exception_type if Argument count does not mach function requirements.
  */
  void ParserBase::ApplyFunc(Stack<Token> &a_stOpt,
                             Stack<Token> &a_stVal, 
                             int a_iArgCount) 
  { 
    assert(m_pTokenReader.get());
    if (a_stOpt.empty())
      return;

    Token funTok = a_stOpt.top();
    Callback *pCallback = funTok.GetCallback(); 
    if (pCallback==NULL && (funTok.GetCode()<cmSIN || funTok.GetCode()>cmSQRT))
      return;

    a_stOpt.pop();

    // Binary operators must rely on their internal operator number
    // since counting of operators relies on commas for function arguments
    // binary operators do not have commas in their expression
    int iArgCount = (funTok.GetCode()==cmOPRT_BIN) ? funTok.GetCallback()->m_nArgc : a_iArgCount;
    int iArgRequired = funTok.GetArgCount();

    if (funTok.GetArgCount()>=0 && iArgCount>iArgRequired) 
      Error(ecTOO_MANY_PARAMS, m_pTokenReader->GetPos()-1, funTok.GetAsString());

    if (funTok.GetCode()!=cmOPRT_BIN && iArgCount<iArgRequired )
      Error(ecTOO_FEW_PARAMS, m_pTokenReader->GetPos()-1, funTok.GetAsString());

    // Collect the numeric function arguments from the value stack and store them
    // in a vector
    std::vector<Token> stArg;  
    for (int i=0; i<iArgCount; ++i)
    {
      stArg.push_back( a_stVal.pop() );
    }

    Token token( ApplyNumFunc(funTok, stArg) );
    a_stVal.push( token );
  }

  //---------------------------------------------------------------------------
  void ParserBase::ApplyRemainingOprt(Stack<Token> &stOpt,
                                      Stack<Token> &stVal) 
  {
    while (stOpt.size() && 
           stOpt.top().GetCode() != cmBO &&
           stOpt.top().GetCode() != cmIF &&
           stOpt.top().GetCode() != cmELSE)
    {
      Token tok = stOpt.top();
      switch (tok.GetCode())
      {
      case cmOPRT_INFIX:
      case cmOPRT_BIN:
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
      case cmAND:
//      case cmMOD:
      case cmOR:
          ApplyBinOprt(stOpt, stVal);
          break;

      //case cmELSE:
      //    ApplyIfElse(stOpt, stVal);
      //    m_vByteCode.AddIfElse(cmENDIF);
      //    break;

      default:
          Error(ecINTERNAL_ERROR);
      }
    }
  }

  //---------------------------------------------------------------------------
  void ParserBase::ApplyIfElse(Stack<Token> &a_stOpt,
                               Stack<Token> &a_stVal) 
  {
    // Check if there is an if Else clause to be calculated
    while (a_stOpt.size() && a_stOpt.top().GetCode()==cmELSE)
    {
      Token opElse = a_stOpt.pop();
      MEC_ASSERT(a_stOpt.size()>0);

      // Take the value associated with the else branch from the value stack
      Token vVal2 = a_stVal.pop();
      if (a_stOpt.top().GetCode()!=cmIF)
      {
        // There is a nested if-else. This needs to be solved recursuvely
        ApplyIfElse(a_stOpt, a_stVal);

        // The else and the associated value need to be pushed back to the stack
        a_stVal.push(vVal2);
        a_stOpt.push(opElse);
      }
      else
      {
        MEC_ASSERT(a_stOpt.size()>0);
        MEC_ASSERT(a_stVal.size()>=2);

        // it then else is a ternary operator Pop all three values from the value s
        // tack and just return the right value
        Token vVal1 = a_stVal.pop();
        Token vExpr = a_stVal.pop();

        a_stVal.push( (vExpr.GetVal()!=0) ? vVal1 : vVal2);
        // Result of if then else is always volatile, the
        // function optimizer won't handle it properly
        a_stVal.top().AddFlags(flVOLATILE);


        Token opIf = a_stOpt.pop();
        MEC_ASSERT(opElse.GetCode()==cmELSE);
        MEC_ASSERT(opIf.GetCode()==cmIF);

        m_vByteCode.AddIfElse(cmENDIF);
      }
    } // while pending if-else-clause found

/*
    MEC_ASSERT(a_stOpt.size()>0);
    MEC_ASSERT(a_stVal.size()>=3);

    // it then else is a ternary operator Pop all three values from the value s
    // tack and just return the right value
    Token vVal2 = a_stVal.pop();
    Token vVal1 = a_stVal.pop();
    Token vExpr = a_stVal.pop();

    a_stVal.push( (vExpr.GetVal()!=0) ? vVal1 : vVal2);
    // Result of if then else is always volatile, the
    // function optimizer won't handle it properly
    a_stVal.top().AddFlags(flVOLATILE);

    Token opElse = a_stOpt.pop();
    Token opIf = a_stOpt.pop();
    m_nIfElseCounter--;

    assert(opIf.GetCode()==cmIF);
*/
  }

  //---------------------------------------------------------------------------
  /** \brief Apply a binary operator. 
      \param a_stOpt The operator stack
      \param a_stVal The value stack
  */
  void ParserBase::ApplyBinOprt(Stack<Token> &a_stOpt,
                                Stack<Token> &a_stVal) 
  {
    if (a_stOpt.top().GetCode()==cmOPRT_INFIX)
    {
      // First check for presence of an infix operator
      ApplyFunc(a_stOpt, a_stVal, 1);
    }
    else
    {
      // user defined binary operator
      if (a_stOpt.top().GetCode()==cmOPRT_BIN)
      {
        ApplyFunc(a_stOpt, a_stVal, 2);
      }
      else
      {
        // internal binary operator
        MEC_ASSERT(a_stVal.size()>=2);

        Token valTok1 = a_stVal.pop(),
              valTok2 = a_stVal.pop(),
              optTok = a_stOpt.pop(),
              resTok; 

        value_type x = valTok2.GetVal(),
                   y = valTok1.GetVal();

        switch (optTok.GetCode())
        {
          // built in binary operators
        case cmMIN:  resTok.SetVal( (x < y) ? x : y);   break; 
        case cmMAX:  resTok.SetVal( (x > y) ? x : y );  break;
        case cmLT:   resTok.SetVal( x < y );            break;
        case cmGT:   resTok.SetVal( x > y );            break;
        case cmLE:   resTok.SetVal( x <= y );           break;
        case cmGE:   resTok.SetVal( x >= y );           break;
        case cmNEQ:  resTok.SetVal( x != y );           break;
        case cmEQ:   resTok.SetVal( x == y );           break;
        case cmAND:  resTok.SetVal( x && y );           break;
        case cmOR:   resTok.SetVal( x || y );           break;
        case cmADD:  resTok.SetVal( x + y );            break;
        case cmSUB:  resTok.SetVal( x - y );            break;
        case cmMUL:  resTok.SetVal( x * y );            break;
        case cmDIV:  resTok.SetVal( x / y );            break;
//        case cmMOD:  resTok.SetVal( x-((int)(x/y)*y) ); break;
        default:  Error(ecINTERNAL_ERROR, 8);
        }

        // Create the bytecode entries
        if (!m_bOptimize || 
            valTok1.IsFlagSet(flVOLATILE) || 
            valTok2.IsFlagSet(flVOLATILE) )
        {
          resTok.AddFlags(flVOLATILE);
          m_vByteCode.AddOp(optTok.GetCode());
        }
        else
        {
          m_vByteCode.RemoveValEntries(2);
          m_vByteCode.AddVal(resTok.GetVal());
        }

        a_stVal.push( resTok );
      }
    }
  }

#if !defined(NO_MICROSOFT_STYLE_INLINE_ASSEMBLY)
  //---------------------------------------------------------------------------
  /** \brief A bytecode parsing engine written in inline assembly. 
  
    This engine is used for debugging and development purposes. It ows
    its existance to the fact that i need it to develop the
    code for the jit engine. This code must be commented for a
    future linux version since GCC won't accept it.
  */
  value_type ParserBase::ParseCmdCodeASM() 
  {
#ifdef _DEBUG
    m_vStackBuffer.assign(m_vStackBuffer.size(), 0);
#endif

    int nIdx(0), nCmd(0), nBuf(0);

    static value_type val_zero = 0, 
                      val_one = 1;
    value_type *pStack = m_pStackZero; //&m_vStackBuffer[0];

    value_type (*pow_func_ff)(value_type, value_type) = pow;
    value_type (*pow_func_fi)(value_type, int) = pow;
    
    SPackedToken *pRPN = m_vByteCode.GetRPNBasePtr();

    __asm
    { 
      // Store the start address of the rpn in ebx
      // and the address of the stack in ecx
      mov ebx, pRPN
      mov ecx, pStack

    // The main loop
    loop_start:

      mov   pRPN, ebx

      // read the stack index and store it into nIdx
      mov   eax, [ebx]                  // EAX <- SPackedToken::m_nStackPos
      mov   nIdx, eax

      // Read the next command
      mov   eax, [ebx+size index_type]  // EAX <- SPackedToken::m_eCode
      mov   nCmd, eax

      cmp   eax, cmVAL
      jz   do_VAL
      cmp   eax, cmVAR
      jz   do_VAR
      cmp   eax, cmLT
      jz   do_LT
      cmp   eax, cmGT
      jz   do_GT
      cmp   eax, cmLE
      jz   do_LE
      cmp   eax, cmGE
      jz   do_GE
      cmp   eax, cmEQ
      jz   do_EQ
      cmp   eax, cmNEQ
      jz   do_NEQ
      cmp   eax, cmMIN
      jz   do_MIN
      cmp   eax, cmMAX
      jz   do_MAX
      cmp   eax, cmAND
      jz   do_AND
      cmp   eax, cmOR
      jz   do_OR
      cmp   eax, cmADD
      jz   do_ADD
      cmp   eax, cmSUB
      jz   do_SUB
      cmp   eax, cmMUL
      jz   do_MUL
      cmp   eax, cmDIV
      jz   do_DIV
      //cmp   eax, cmMOD
      //jz   do_MOD
      cmp   eax, cmFUNC
      jz   do_FUNC
      cmp   eax, cmSIN
      jz   do_SIN
      cmp   eax, cmCOS
      jz   do_COS
      cmp   eax, cmTAN
      jz   do_TAN
      cmp   eax, cmABS
      jz    do_ABS
      cmp   eax, cmSQRT
      jz    do_SQRT
      cmp   eax, cmIF
      jz    do_IF
      cmp   eax, cmELSE
      jz    do_ELSE
      cmp   eax, cmENDIF
      jz    do_ENDIF
      cmp   eax, cmEND
      jz   loop_end
      
      // never should get here...
      jmp  loop_end


  do_EQ: // ok
        movss xmm0, dword ptr [ecx] // move first argument to xmm0 
        sub  ecx, size value_type   // decrease value stack pointer to the 1st data item
        movss xmm1, dword ptr [ecx] // move second argument to xmm1
        comiss xmm0, xmm1           // compare xmm0 and xmm1; the result is either an all '1' or an all '0' bitmask
      jz is_equal
        movss xmm0, val_zero
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

      is_equal:
        movss xmm0, val_one
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item 

    do_NEQ: // ok
        movss xmm0, dword ptr [ecx] // move first argument to xmm0 
        sub  ecx, size value_type   // decrease value stack pointer to the 1st data item
        movss xmm1, dword ptr [ecx] // move second argument to xmm1
        comiss xmm0, xmm1           // compare xmm0 and xmm1; the result is either an all '1' or an all '0' bitmask

      jnz is_not_equal
        movss xmm0, val_zero
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp   loop_start  

      is_not_equal:
        movss xmm0, val_one
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

    do_LT: // ok
        movss xmm0, dword ptr [ecx] // move first argument to xmm0 
        sub  ecx, size value_type   // decrease value stack pointer to the 1st data item
        movss xmm1, dword ptr [ecx] // move second argument to xmm1
        comiss xmm0, xmm1           // compare xmm0 and xmm1; the result is either an all '1' or an all '0' bitmask

      ja is_less_than
        movss xmm0, val_zero
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

      is_less_than:
        movss xmm0, val_one
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

    do_GT: // ok
        movss xmm0, dword ptr [ecx] // move first argument to xmm0 
        sub  ecx, size value_type   // decrease value stack pointer to the 1st data item
        movss xmm1, dword ptr [ecx] // move second argument to xmm1
        comiss xmm0, xmm1           // compare xmm0 and xmm1; the result is either an all '1' or an all '0' bitmask
      jb is_greater_than
        movss xmm0, val_zero
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

      is_greater_than:
        movss xmm0, val_one
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

    do_LE: // ok
        movss xmm0, dword ptr [ecx] // move first argument to xmm0 
        sub  ecx, size value_type   // decrease value stack pointer to the 1st data item
        movss xmm1, dword ptr [ecx] // move second argument to xmm1
        comiss xmm0, xmm1           // compare xmm0 and xmm1; the result is either an all '1' or an all '0' bitmask
      jae is_less_or_equal
        movss xmm0, val_zero
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

      is_less_or_equal:
        movss xmm0, val_one
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

    do_GE: // ok
        movss xmm0, dword ptr [ecx] // move first argument to xmm0 
        sub  ecx, size value_type   // decrease value stack pointer to the 1st data item
        movss xmm1, dword ptr [ecx] // move second argument to xmm1
        comiss xmm0, xmm1           // compare xmm0 and xmm1; the result is either an all '1' or an all '0' bitmask
      jbe is_greater_or_equal
        movss xmm0, val_zero
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

      is_greater_or_equal:
        movss xmm0, val_one
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

    do_MIN: // ok
        movss xmm0, dword ptr [ecx]
        sub  ecx, size value_type    // decrease value stack pointer to the 1st data item
        movss xmm1, dword ptr [ecx]
        minss xmm0, xmm1
        movss [ecx], xmm0  
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

    do_MAX: // ok
        movss xmm0, dword ptr [ecx]
        sub  ecx, size value_type    // decrease value stack pointer to the 1st data item
        movss xmm1, dword ptr [ecx]
        maxss xmm0, xmm1
        movss [ecx], xmm0 
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

    do_AND: // ok
        movss xmm0, dword ptr [ecx]  // first value into xmm0
        sub   ecx, size value_type   // decrease value stack pointer to the 1st data item
        movss xmm1, val_zero         // zero into xmm2
        ucomiss xmm0, xmm1           // xmm0 mit 0 vergleichen
        je result_is_zero
        
        movss xmm0, dword ptr [ecx]  // first value into xmm0
        ucomiss xmm0, xmm1           // xmm0 mit 0 vergleichen
        je result_is_zero

        movss xmm0, val_one
        movss [ecx], xmm0
        add ebx, size SPackedToken   // advance bytecode to the next element
        jmp loop_start;

      result_is_zero:
        movss xmm0, val_zero
        movss [ecx], xmm0
        add ebx, size SPackedToken   // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

    do_OR: // ok
        movss xmm0, dword ptr [ecx]  // first value into xmm0
        sub   ecx, size value_type   // decrease value stack pointer to the 1st data item
        movss xmm1, dword ptr [ecx]  // second value into xmm1
        movss xmm2, val_zero         // zero into xmm2
        ucomiss xmm0, xmm2           // xmm0 mit 0 vergleichen
        jne result_is_one
        ucomiss xmm1, xmm2           // xmm2 mit 0 vergleichen
        jne result_is_one
        movss xmm0, val_zero
        movss [ecx], xmm0
        add ebx, size SPackedToken   // advance bytecode to the next element
        jmp loop_start;

      result_is_one:
        movss xmm0, val_one
        movss [ecx], xmm0
        add ebx, size SPackedToken   // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

    do_ADD: // ok
        movss xmm0, dword ptr [ecx]
        sub  ecx, size value_type    // decrease value stack pointer to the 1st data item
        movss xmm1, dword ptr [ecx]
        addss xmm0, xmm1
        movss [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp loop_start               // read next bytecode item

    do_SUB: // ok
      movss xmm1, dword ptr [ecx]
      sub  ecx, size value_type    // decrease value stack pointer to the 1st data item
      movss xmm0, dword ptr [ecx]
      subss xmm0, xmm1
      movss [ecx], xmm0 
      add ebx, size SPackedToken     // advance bytecode to the next element
      jmp loop_start               // read next bytecode item

    do_MUL: // ok
      movss xmm1, dword ptr [ecx]
      sub  ecx, size value_type    // decrease value stack pointer to the 1st data item
      movss xmm0, dword ptr [ecx]
      mulss xmm0, xmm1
      movss [ecx], xmm0 
      add ebx, size SPackedToken     // advance bytecode to the next element
      jmp loop_start               // read next bytecode item

    do_DIV: // ok
      movss xmm1, dword ptr [ecx]
      sub  ecx, size value_type    // decrease value stack pointer to the 1st data item
      movss xmm0, dword ptr [ecx]
      divss xmm0, xmm1
      movss [ecx], xmm0 
      add ebx, size SPackedToken     // advance bytecode to the next element
      jmp loop_start               // read next bytecode item

    do_MOD: // ok
      movss xmm1, dword ptr [ecx]
      sub  ecx, size value_type    // decrease value stack pointer to the 1st data item
      movss xmm0, dword ptr [ecx]
      divss xmm0, xmm1
      cvtss2si edx, xmm0          // xmm0 in ganzzahl int convertieren und in edx speichern
      cvtsi2ss xmm0, edx          // von edx zurück in xmm0 kopieren
      mulss xmm0, xmm1 
      movss xmm1, dword ptr [ecx]
      subss xmm1, xmm0
      movss [ecx], xmm1 
      add ebx, size SPackedToken     // advance bytecode to the next element
      jmp loop_start               // read next bytecode item

    do_VAR: // ok
      add ecx, size value_type       // set calculation stack pointer to the next element
      mov edx, dword ptr [ebx + 2*size index_type]    // move the address into edx
      movss xmm0, dword ptr [edx]    // Move address from bytecode into xmm0
      movss [ecx], xmm0              // move into stack
      add ebx, size SPackedToken     // advance bytecode to the next element
      jmp loop_start                 // read next bytecode item

    do_VAL: // ok
      add ecx, size value_type       // set calculation stack pointer to the next element
      movss xmm0, dword ptr [ebx + 2*size index_type] // Move address from bytecode into xmm0
      movss [ecx], xmm0              // move into stack
      add ebx, size SPackedToken     // advance bytecode to the next element
      jmp loop_start                 // read next bytecode item

    do_FUNC: // ok
        mov   edx, [ebx + 2*size index_type] // store number of arguments in edx
        mov   esi, [ebx + 2*size index_type] // store number of arguments in esi   
        mov   eax, [ebx + 3*size index_type] // Read the function pointer and store it into eax
                                  
      push_args:
                                      // about to be called; it needs to be saved
        test  edx, edx                // edx is the loop variable; check if loop variable is zero
        jz    arguments_pushed        // If edx is zero all parameters have been pushed
        sub   esp, size value_type    // make room on the stack for the function parameters
        fld   dword ptr [ecx]         // load value from edx into fpu stack
        fstp  dword ptr [esp]         // store value from the fpu stack into the call stack and pop fpu stack
        sub   ecx, size value_type
        sub   edx, 1
        jmp   push_args

      // execute the function
      arguments_pushed:
        mov   edi, ecx                // save the value of ecx it may be destroyed by the called function (i.e. sqrt)
        call  eax                     // call the function
        mov   eax, esi                // move number of arguments into eax
        mov   edx, size value_type
        imul  edx
        add   esp, eax                // balance the stack
        mov   ecx, edi
        add   ecx, size value_type    // Add new item to the parser value stack
        fstp  dword ptr [ecx]         // store return value in the parser value stack
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp   loop_start
    
      do_SIN: // ok
        fld   dword ptr [ecx]
        fsin  
        fstp  dword ptr [ecx]
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp   loop_start

      do_COS: // ok
        fld   dword ptr [ecx]
        fcos
        fstp  dword ptr [ecx]
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp   loop_start
      
      do_TAN: // ok
        fld   dword ptr [ecx]
        fptan
        fstp  ST(0)
        fstp  dword ptr [ecx]
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp   loop_start

      do_ABS: // ok
          movss  xmm1, dword ptr [ecx]
          movss  xmm0, val_zero        // zero into xmm2
          comiss xmm0, xmm1            // xmm0 mit 0 ver
          ja apply_sign
          movss [ecx], xmm1
          add ebx, size SPackedToken     // advance bytecode to the next element
          jmp loop_start;

        apply_sign:
          mov edx, -1
          cvtsi2ss xmm0, edx
          mulss xmm1, xmm0
          movss [ecx], xmm1
          add ebx, size SPackedToken     // advance bytecode to the next element
          jmp loop_start;

      do_SQRT: // ok
        movss  xmm1, dword ptr [ecx]
        sqrtss xmm0, xmm1
        movss  [ecx], xmm0
        add ebx, size SPackedToken     // advance bytecode to the next element
        jmp   loop_start

      do_IF:
        movss xmm0, dword ptr [ecx] // move first argument to xmm0 
        movss xmm1, val_zero
        sub ecx, size value_type       // set calculation stack pointer to the next element
        ucomiss xmm0, xmm1                    // compare xmm0 and xmm1; the result is either an all '1' or an all '0' bitmask
        jne if_branch

        // i+= tok.Jmp.offset;
        mov   eax, [ebx + 2*size index_type] 
        inc   eax
        mov   edx, size SPackedToken
        imul  edx
        add   ebx, eax                       // advance bytecode to the next element
        mov   pRPN, ebx
        jmp loop_start

        if_branch:
        add ebx, size SPackedToken           // advance bytecode to the next element
        jmp loop_start

      do_ELSE:
        // i += tok.Jmp.offset;
        sub ecx, size value_type       // set calculation stack pointer to the next element
        mov   eax, [ebx + 2*size index_type] // move number of arguments into eax
        mov   edx, size SPackedToken
        imul  edx
        add   ebx, eax                       // advance bytecode to the next element
        mov   pRPN, ebx
        jmp   loop_start

      do_ENDIF:
        add ebx, size SPackedToken           // advance bytecode to the next element
        jmp   loop_start

      loop_end:
    }

    return pStack[1];
  }
#endif // #if !defined(NO_MICROSOFT_STYLE_INLINE_ASSEMBLY)

  //---------------------------------------------------------------------------
  value_type ParserBase::ParseJIT() 
  {
    return m_pCompiledFun();
  }

  //---------------------------------------------------------------------------
  /** \brief Parse the command code.
      \sa ParseString()

      Command code contains precalculated stack positions of the values and the
      associated operators. The Stack is filled beginning from index one the 
      value at index zero is not used at all.
  */
  value_type ParserBase::ParseCmdCode() 
  {
    value_type *Stack = &m_vStackBuffer[0];
    ECmdCode eCode;
    bytecode_type idx(0);
    int i(-1);
    const SPackedToken *pRPNBase = m_vByteCode.GetRPNBasePtr();

    __start:

    ++i;
    const SPackedToken &tok = pRPNBase[i];
    idx   = tok.m_nStackPos;
    eCode = (ECmdCode)tok.m_eCode;

    switch (eCode)
    {
    case cmMIN:  Stack[idx]  = (Stack[idx] < Stack[idx+1]) ? Stack[idx] : Stack[idx+1]; goto __start;
    case cmMAX:  Stack[idx]  = (Stack[idx] > Stack[idx+1]) ? Stack[idx] : Stack[idx+1]; goto __start;
    case cmLE:   Stack[idx]  = Stack[idx] <= Stack[idx+1];  goto __start;
    case cmGE:   Stack[idx]  = Stack[idx] >= Stack[idx+1];  goto __start;
    case cmNEQ:  Stack[idx]  = Stack[idx] != Stack[idx+1];  goto __start;
    case cmEQ:   Stack[idx]  = Stack[idx] == Stack[idx+1];  goto __start;
    case cmLT:   Stack[idx]  = Stack[idx] < Stack[idx+1];   goto __start;
    case cmGT:   Stack[idx]  = Stack[idx] > Stack[idx+1];   goto __start;
    case cmAND:  Stack[idx]  = Stack[idx] && Stack[idx+1];  goto __start;
    case cmOR:   Stack[idx]  = Stack[idx] || Stack[idx+1];  goto __start;
    case cmADD:  Stack[idx] += Stack[1+idx]; goto __start;
    case cmSUB:  Stack[idx] -= Stack[1+idx]; goto __start;
    case cmMUL:  Stack[idx] *= Stack[1+idx]; goto __start;
    case cmDIV:  Stack[idx] /= Stack[1+idx]; goto __start;
//    case cmMOD:  Stack[idx] = Stack[idx]-((int)(Stack[idx]/Stack[1+idx])*Stack[1+idx]) ; goto __start;
    case cmSIN:  Stack[idx] = sin(Stack[idx]);  goto __start;
    case cmCOS:  Stack[idx] = cos(Stack[idx]);  goto __start;
    case cmTAN:  Stack[idx] = tan(Stack[idx]);  goto __start;
    case cmABS:  Stack[idx] = fabs(Stack[idx]); goto __start;
    case cmSQRT: Stack[idx] = sqrt(Stack[idx]); goto __start;

    // variable and value tokens
    case  cmVAR:  Stack[idx] = *(tok.m_pVar); goto __start;
    case  cmVAL:  Stack[idx] = tok.m_fVal;    goto __start;

    // Next is treatment of numeric functions
    case  cmFUNC:
          {
            int nArgCount = tok.Fun.m_nArgc;

            // switch according to argument count
            switch(nArgCount)  
            {
            case 0:  Stack[idx] = tok.Fun.m_pFun0 (); break;
            case 1:  Stack[idx] = tok.Fun.m_pFun1 (Stack[idx]); break;
            case 2:  Stack[idx] = tok.Fun.m_pFun2 (Stack[idx], Stack[idx+1]); break;
            case 3:  Stack[idx] = tok.Fun.m_pFun3 (Stack[idx], Stack[idx+1], Stack[idx+2]); break;
            case 4:  Stack[idx] = tok.Fun.m_pFun4 (Stack[idx], Stack[idx+1], Stack[idx+2], Stack[idx+3]); break;
            case 5:  Stack[idx] = tok.Fun.m_pFun5 (Stack[idx], Stack[idx+1], Stack[idx+2], Stack[idx+3], Stack[idx+4]); break;
            case 6:  Stack[idx] = tok.Fun.m_pFun6 (Stack[idx], Stack[idx+1], Stack[idx+2], Stack[idx+3], Stack[idx+4], Stack[idx+5]); break;
            case 7:  Stack[idx] = tok.Fun.m_pFun7 (Stack[idx], Stack[idx+1], Stack[idx+2], Stack[idx+3], Stack[idx+4], Stack[idx+5], Stack[idx+6]); break;
            case 8:  Stack[idx] = tok.Fun.m_pFun8 (Stack[idx], Stack[idx+1], Stack[idx+2], Stack[idx+3], Stack[idx+4], Stack[idx+5], Stack[idx+6], Stack[idx+7]); break;
            case 9:  Stack[idx] = tok.Fun.m_pFun9 (Stack[idx], Stack[idx+1], Stack[idx+2], Stack[idx+3], Stack[idx+4], Stack[idx+5], Stack[idx+6], Stack[idx+7], Stack[idx+8]); break;
            case 10: Stack[idx] = tok.Fun.m_pFun10(Stack[idx], Stack[idx+1], Stack[idx+2], Stack[idx+3], Stack[idx+4], Stack[idx+5], Stack[idx+6], Stack[idx+7], Stack[idx+8], Stack[idx+9]); break;
            }
          }
          goto __start;

      case  cmIF:
            if (Stack[idx]==0)
              i += tok.Jmp.offset;
            goto __start;

      case  cmELSE:
            i += tok.Jmp.offset;
            goto __start;

      case  cmENDIF:
            goto __start;

      case  cmEND:
	          return Stack[1];

	    default:
            Error(ecINTERNAL_ERROR, 2);
            return 0;
    }
  }

  //---------------------------------------------------------------------------
  /** \brief One of the two main parse functions.

  Parse expression from input string. Perform syntax checking and create bytecode.
  After parsing the string and creating the bytecode the function pointer 
  #m_pParseFormula will be changed to the second parse routine the uses bytecode instead of string parsing.

  \sa ParseCmdCode(), ParseValue()
  */
  value_type ParserBase::ParseString() 
  {
    if (!m_pTokenReader->GetFormula().length())
        Error(ecUNEXPECTED_EOF, 0);

    Stack<Token> stOpt, stVal;
    Stack<int> stArgCount;
    Token opta, opt;  // for storing operators
    Token val, tval;  // for storing value
    string_type strBuf;    // buffer for string function arguments

    ReInit();

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
                m_vByteCode.AddVar(opt.GetVar() ); 
                break;

        case cmVAL:
                stVal.push(opt);
                m_vByteCode.AddVal( opt.GetVal() );
                break;

        case  cmIF:
        case  cmELSE:
                m_nIfElseCounter += (opt.GetCode()==cmIF) ? 1 : -1;
                if (m_nIfElseCounter<0)
                  Error(ecMISPLACED_COLON, m_pTokenReader->GetPos());

                ApplyRemainingOprt(stOpt, stVal);
                m_vByteCode.AddIfElse(opt.GetCode());
                stOpt.push(opt);
                break;

        case cmARG_SEP:
                if (stArgCount.empty())
                  Error(ecUNEXPECTED_ARG_SEP, m_pTokenReader->GetPos());

                ++stArgCount.top();
                // fall through...

        case cmEND:
                ApplyRemainingOprt(stOpt, stVal);
                ApplyIfElse(stOpt, stVal);
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
                  ApplyIfElse(stOpt, stVal);

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

                    if (iArgCount>1 && ( stOpt.size()==0 || !stOpt.top().IsFunction() ) )
                      Error(ecUNEXPECTED_ARG, m_pTokenReader->GetPos());
                    
                    // The opening bracket was popped from the stack now check if there
                    // was a function before this bracket
                    if (stOpt.size() && 
                        stOpt.top().GetCode()!=cmOPRT_INFIX && 
                        stOpt.top().GetCode()!=cmOPRT_BIN && 
                        stOpt.top().IsFunction()!=0)
                    {
                      ApplyFunc(stOpt, stVal, iArgCount);
                    }
                  }
                } // if bracket content is evaluated
                break;




/*





                {
                  // The argument count for parameterless functions is zero
                  // by default an opening bracket sets parameter count to 1
                  // in preparation of arguments to come. If the last token
                  // was an opening bracket we know better...
                  if (opta.GetCode()==cmBO)
                    --stArgCount.top();

                  ApplyRemainingOprt(stOpt, stVal);
                  ApplyIfElse(stOpt, stVal);

                  if ( opt.GetCode()!=cmBC || stOpt.size()==0 || stOpt.top().GetCode()!=cmBO )
                    break;

                  // if opt is ")" and opta is "(" the bracket has been evaluated, now its time to check
			            // if there is either a function or a sign pending
		   	          // neither the opening nor the closing bracket will be pushed back to
			            // the operator stack
			            // Check if a function is standing in front of the opening bracket, 
                  // if yes evaluate it afterwards check for infix operators
			            assert(stArgCount.size());
			            int iArgCount = stArgCount.pop();
                  
                  stOpt.pop(); // Take opening bracket from stack

                  if (iArgCount>1 && ( stOpt.size()==0 || !stOpt.top().IsFunction() ) )
                    Error(ecUNEXPECTED_ARG, m_pTokenReader->GetPos());
                  
                  if (stOpt.size() &&
                      stOpt.top().GetCode()!=cmOPRT_BIN && 
                      stOpt.top().GetCode()!=cmOPRT_INFIX && 
                      stOpt.top().IsFunction() )
                  {
                    ApplyFunc(stOpt, stVal, iArgCount);
                  }
                } // if bracket content is evaluated





*/




        //
        // Next are the binary operator entries
        //
        case cmMIN:
        case cmMAX:
        case cmLT:
        case cmGT:
        case cmLE:
        case cmGE:
        case cmNEQ:
        case cmEQ:
        case cmAND:
        case cmOR:
        case cmADD:
        case cmSUB:
        case cmMUL:
        case cmDIV:
//        case cmMOD:
        case cmOPRT_BIN:
                // A binary operator (user defined or built in) has been found. 
                while (stOpt.size() && 
                       stOpt.top().GetCode() != cmBO &&
                       stOpt.top().GetCode() != cmELSE &&
                       stOpt.top().GetCode() != cmIF)
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

        case cmSIN:
        case cmCOS:
        case cmTAN:
        case cmABS:
        case cmSQRT:
        case cmFUNC:
        case cmOPRT_INFIX:
                stOpt.push(opt);
                break;

        case cmOPRT_POSTFIX:
                stOpt.push(opt);
                ApplyFunc(stOpt, stVal, 1);  // this is the postfix operator
                break;

        default:  Error(ecINTERNAL_ERROR, 3);
      } // end of switch operator-token

      opta = opt;

      if ( opt.GetCode() == cmEND )
      {
        m_vByteCode.Finalize();
        break;
      }

      //if (stArgCount.size())
      //  cout << "Arguments: " << stArgCount.top() << "\n";

  #if defined(MEC_DUMP_STACK)

      StackDump(stVal, stOpt);
      m_vByteCode.AsciiDump();
  #endif
    } // while (true)

    if (g_DbgDumpCmdCode)
      m_vByteCode.AsciiDump();

    if (m_nIfElseCounter>0)
      Error(ecMISSING_ELSE_CLAUSE);

    // get the last value (= final result) from the stack
    if (stVal.size()!=1)
      Error(ecEMPTY_EXPRESSION);

    // no error, so change the function pointer for the main parse routine
    value_type fVal = stVal.top().GetVal();   // Result from String parsing

    SwitchEngine();
    return fVal;
  }

  void ParserBase::AsciiDump()
  {
    m_vByteCode.AsciiDump();
  }

  //---------------------------------------------------------------------------
  /** \brief Switch the parser engine. */
  void ParserBase::SwitchEngine() 
  {
    m_vStackBuffer.resize(m_vByteCode.GetMaxStackSize());
    m_pStackZero = &m_vStackBuffer[0];

    switch (m_eEngine)
    {
    case  peBYTECODE:
          m_pParseFormula = &ParserBase::ParseCmdCode;
          break;

#if !defined(NO_MICROSOFT_STYLE_INLINE_ASSEMBLY)
    case  peBYTECODE_ASM:
          m_pParseFormula = &ParserBase::ParseCmdCodeASM;
          break;
#endif

    case peJIT:
         m_compiler.Bind(m_vByteCode.GetRPNBasePtr());
         m_pCompiledFun = m_compiler.Compile(5);
         m_pParseFormula = &ParserBase::ParseJIT;
         break;
    }
  }

  //---------------------------------------------------------------------------
  value_type (*ParserBase::Compile(int nHighestReg))()
  {
    // First create the bytecode by calling Eval
    Eval(); 

    // Next compile the expression 
    m_compiler.Bind(m_vByteCode.GetRPNBasePtr());
    return m_compiler.Compile(nHighestReg);
  }

  //---------------------------------------------------------------------------
  /** \brief Create an error containing the parse error position.
      \param a_iErrc [in] The error code of type #EErrorCodes.
      \param a_iPos [in] The position where the error was detected.
      \param a_strTok [in] The token string representation associated with the error.
      \throw ParserException always throws thats the only purpose of this function.

    This function will create an Parser Exception object containing the error text and
    its position.
  */
  void  ParserBase::Error(EErrorCodes a_iErrc, int a_iPos, const string_type &a_sTok) const
  {
    throw exception_type(a_iErrc, a_sTok, m_pTokenReader->GetFormula(), a_iPos);
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
  /** \brief Clear the formula. 
      \post Resets the parser to string parsing mode.
      \throw nothrow

      Clear the formula and existing bytecode.
  */
  void ParserBase::ClearFormula()
  {
    m_vByteCode.clear();
    m_pTokenReader->SetExpr(_T(""));
    ReInit();
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
    m_StrVarDef.clear();
    ReInit();
  }

  //------------------------------------------------------------------------------
  /** \brief Clear all user defined postfix operators.
      \post Resets the parser to string parsing mode.
      \throw nothrow
  */
  void ParserBase::ClearPostfixOprt()
  {
    m_PostOprtDef.clear();
    ReInit();
  }

  //------------------------------------------------------------------------------
  /** \brief Clear all user defined binary operators.
      \post Resets the parser to string parsing mode.
      \throw nothrow
  */
  void ParserBase::ClearOprt()
  {
    m_OprtDef.clear();
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

  //------------------------------------------------------------------------------
  /** \brief Set the parser engine.

    You can use this function in order to disable the bytecode. 
    \attention There is no reason to disable bytecode. It will 
               drastically decrease parsing speed.
  */
  void ParserBase::SetParserEngine(EParserEngine eEngine)
  {
    if (m_eEngine!=eEngine)
      ReInit();

    m_eEngine = eEngine;
  }

  //------------------------------------------------------------------------------
  /** \brief Get the argument separator character. 
  */
  char_type ParserBase::GetArgSep() const
  {
    return m_pTokenReader->GetArgSep();
  }

  //------------------------------------------------------------------------------
  /** \brief Set argument separator. 
      \param cArgSep the argument separator character.
  */
  void ParserBase::SetArgSep(char_type cArgSep)
  {
    m_pTokenReader->SetArgSep(cArgSep);
  }

  //------------------------------------------------------------------------------
  /** \brief Dump stack content. 

      This function is used for debugging only.
  */
  void ParserBase::StackDump(const Stack<Token> &a_stVal, 
                             const Stack<Token> &a_stOprt) const
  {
    Stack<Token> stOprt(a_stOprt), 
                 stVal(a_stVal);

    console() << _T("\nValue stack:\n");
    while ( !stVal.empty() ) 
    {
      Token val = stVal.pop();
      ECmdCode cmd = val.GetCode();
      switch (cmd)
      {
      case cmVAL: console() << _T(" ") << val.GetVal(); break;
      case cmVAR: console() << _T(" ") << val.GetAsString(); break;
      }

      if (val.IsFlagSet(flVOLATILE))
      {
        console() << _T("* ");
      }
      else
      {
        console() << _T(" ");
      }
    }

    console() << "\nOperator stack:\n";

    while ( !stOprt.empty() )
    {
      ECmdCode cmd = stOprt.top().GetCode();
      switch(cmd)
      {
      case cmVAR:   console() << _T("VAR\n");  break;
      case cmVAL:   console() << _T("VAL\n");  break;
      case cmSIN:   console() << _T("SIN\n");  break;
      case cmCOS:   console() << _T("COS\n");  break;
      case cmTAN:   console() << _T("TAN\n");  break;
      case cmABS:   console() << _T("ABS\n");  break;
      case cmSQRT:  console() << _T("SQRT\n"); break;

      case cmMIN:   console() << _T("<?\n"); break;
      case cmMAX:   console() << _T(">?\n"); break;
      case cmLE:    console() << _T("<=\n"); break;
      case cmGE:    console() << _T(">=\n"); break;
      case cmNEQ:   console() << _T("!=\n"); break;
      case cmEQ:    console() << _T("==\n"); break;
      case cmLT:    console() << _T("<\n");  break;
      case cmGT:    console() << _T(">\n");  break;
      case cmAND:   console() << _T("&&\n");  break;
      case cmOR:    console() << _T("||\n");  break;
      case cmADD:   console() << _T("+\n");  break;
      case cmSUB:   console() << _T("-\n");  break;
      case cmMUL:   console() << _T("*\n");  break;
      case cmDIV:   console() << _T("/\n");  break;
//      case cmMOD:   console() << _T("%\n");  break;

#ifdef MEC_INTRINSIC_POW
      case cmPOW:   console() << _T("^\n");  break;
#endif

      case cmFUNC:  console() << _T("FUNC_NUM \"") 
                              << stOprt.top().GetAsString() 
                              << _T("\"\n");   break;
  	  case cmOPRT_INFIX: console() << _T("OPRT_INFIX \"")
                                   << stOprt.top().GetAsString() 
                                   << _T("\"\n");      break;
      case cmOPRT_BIN:   console() << _T("OPRT_BIN \"") 
                                   << stOprt.top().GetAsString() 
                                   << _T("\"\n");           break;
      case cmEND:      console() << _T("END\n");            break;
      case cmUNKNOWN:  console() << _T("UNKNOWN\n");        break;
      case cmBO:       console() << _T("BRACKET \"(\"\n");  break;
      case cmBC:       console() << _T("BRACKET \")\"\n");  break;
      }
      stOprt.pop();
    }

    console() << dec << endl;
  }
} // namespace mec

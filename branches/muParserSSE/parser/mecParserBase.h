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
#ifndef MEC_PARSER_BASE_H
#define MEC_PARSER_BASE_H

//--- Standard includes ------------------------------------------------------------------------
#include <cmath>
#include <string>
#include <iostream>
#include <map>
#include <memory>

#undef free
#undef realloc

//--- ASMJit compiler---------------------------------------------------------------------
#include "AsmJit/Assembler.h"
#include "AsmJit/Compiler.h"
#include "AsmJit/MemoryManager.h"
#include "AsmJit/Logger.h"

//--- Parser includes --------------------------------------------------------------------------
#include "mecDef.h"
#include "mecStack.h"
#include "mecTokenReader.h"
#include "mecReversePolishNotation.h"
#include "mecError.h"
#include "mecExprCompiler.h"


namespace mec
{
/** \file
    \brief This file contains the class definition of the muparser engine.
*/

//--------------------------------------------------------------------------------------------------
/** \brief Mathematical expressions parser (JIT Version).
    \author (C) 2011 Ingo Berg

  This is the implementation of a bytecode based mathematical expressions parser. 
  The formula will be parsed from string and converted into a bytecode. 
  Future calculations will be done with the bytecode instead the formula string
  resulting in a significant performance increase. 
  Complementary to a set of internally implemented functions the parser is able to handle 
  user defined functions and variables. 

*/
class ParserBase 
{
friend class TokenReader;

private:

    /** \brief Typedef for the parse functions. 
    
      The parse function do the actual work. The parser exchanges
      the function pointer to the parser function depending on 
      which state it is in. (i.e. bytecode parser vs. string parser)
    */
    typedef value_type (ParserBase::*ParseFunction)();  

    /** \brief Type used for storing an array of values. */
    typedef std::vector<value_type> valbuf_type;

    /** \brief Typedef for the token reader. */
    typedef TokenReader token_reader_type;
    
 public:

    /** \brief Type of the error class. 
    
      Included for backwards compatibility.
    */
    typedef ParserError exception_type;

    ParserBase(); 
    ParserBase(const ParserBase &a_Parser);
    ParserBase& operator=(const ParserBase &a_Parser);

    virtual ~ParserBase();
    
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
	  inline value_type Eval() 
    {
      return (this->*m_pParseFormula)(); 
    }
	  
    value_type (*Compile(int nHighestReg))();

    void AsciiDump();
    void SetExpr(const string_type &a_sExpr);
    void SetVarFactory(facfun_type a_pFactory, void *pUserData = NULL);
    void SetParserEngine(EParserEngine eEngine);
    void AddValIdent(identfun_type a_pCallback);

/** \fn void mu::ParserBase::DefineFun(const string_type &a_strName, fun_type0 a_pFun) 
    \brief Define a parser function without arguments.
    \param a_strName Name of the function
    \param a_pFun Pointer to the callback function
*/

#define MEC_DEFINE_FUNC(TYPE)                                        \
    void DefineFun(const string_type &a_strName, TYPE a_pFun, int flags = 0) \
    {                                                                \
      AddCallback( a_strName,                                        \
                   Callback(a_pFun),                                 \
                   m_FunDef,                                         \
                   ValidNameChars() );                               \
    }

    MEC_DEFINE_FUNC(fun_type0)
    MEC_DEFINE_FUNC(fun_type1)
    MEC_DEFINE_FUNC(fun_type2)
    MEC_DEFINE_FUNC(fun_type3)
    MEC_DEFINE_FUNC(fun_type4)
    MEC_DEFINE_FUNC(fun_type5)
    MEC_DEFINE_FUNC(fun_type6)
    MEC_DEFINE_FUNC(fun_type7)
    MEC_DEFINE_FUNC(fun_type8)
    MEC_DEFINE_FUNC(fun_type9)
    MEC_DEFINE_FUNC(fun_type10)
#undef MEC_DEFINE_FUNC

    void DefineOprt(const string_type &a_strName, 
                    fun_type2 a_pFun, 
                    unsigned a_iPri=0, 
                    EOprtAssociativity eAsc = oaLEFT, 
                    int flags = 0);
    void DefineConst(const string_type &a_sName, value_type a_fVal);
    void DefineVar(const string_type &a_sName, value_type *a_fVar);
    void DefinePostfixOprt(const string_type &a_strFun, fun_type1 a_pOprt, int flags = 0);
    void DefineInfixOprt(const string_type &a_strName, fun_type1 a_pOprt, int a_iPrec=prINFIX);

    // Clear user defined variables, constants or functions
    void ClearVar();
    void ClearFun();
    void ClearConst();
    void ClearInfixOprt();
    void ClearPostfixOprt();
    void ClearOprt();
    
    void RemoveVar(const string_type &a_strVarName);
    const varmap_type& GetUsedVar();
    const varmap_type& GetVar() const;
    const valmap_type& GetConst() const;
    const string_type& GetExpr() const;
    const funmap_type& GetFunDef() const;
    string_type GetVersion() const;

    const char_type ** GetOprtDef() const;
    void DefineNameChars(const char_type *a_szCharset);
    void DefineOprtChars(const char_type *a_szCharset);
    void DefineInfixOprtChars(const char_type *a_szCharset);
    
    void EnableOptimizer(bool a_bIsOn);
    const char_type* ValidNameChars() const;
    const char_type* ValidOprtChars() const;
    const char_type* ValidInfixOprtChars() const;

    void SetArgSep(char_type cArgSep);
    char_type GetArgSep() const;
    
    void  Error(EErrorCodes a_iErrc, 
                int a_iPos = (int)string_type::npos, 
                const string_type &a_strTok = string_type() ) const;

 protected:
	  
    void Init();

    virtual void InitCharSets() = 0;
    virtual void InitFun() = 0;
    virtual void InitConst() = 0;
    virtual void InitOprt() = 0; 

    static const char_type *c_DefaultOprt[]; 

 private:

    void Assign(const ParserBase &a_Parser);
    void InitTokenReader();
    void ReInit();

    void AddCallback( const string_type &a_strName, 
                      const Callback &a_Callback, 
                      funmap_type &a_Storage,
                      const char_type *a_szCharSet );

    void ApplyRemainingOprt(Stack<Token> &a_stOpt,
                            Stack<Token> &a_stVal);

    void ApplyBinOprt(Stack<Token> &a_stOpt,
                      Stack<Token> &a_stVal);

    void ApplyIfElse(Stack<Token> &a_stOpt,
                     Stack<Token> &a_stVal);

    void ApplyFunc(Stack<Token> &a_stOpt,
                   Stack<Token> &a_stVal, 
                   int iArgCount); 

    Token ApplyNumFunc(const Token &a_FunTok,
                       const std::vector<Token> &a_vArg);

    int GetOprtPrecedence(const Token &a_Tok) const;
    EOprtAssociativity GetOprtAssociativity(const Token &a_Tok) const;

    value_type ParseString(); 
    value_type ParseCmdCode();
    value_type ParseJIT();

#if !defined(NO_MICROSOFT_STYLE_INLINE_ASSEMBLY)
    value_type ParseCmdCodeASM();
#endif

    void SwitchEngine();

    void  ClearFormula();
    void  CheckName(const string_type &a_strName, const string_type &a_CharSet) const;

    void StackDump(const Stack<Token> &a_stVal, 
                   const Stack<Token> &a_stOprt) const;

    /** \brief Pointer to the parser function. 
    
      Eval() calls the function whose address is stored there.
    */
    ParseFunction  m_pParseFormula;
    value_type (*m_pCompiledFun)();
    valbuf_type m_vStackBuffer; ///< This is merely a buffer used for the stack in the cmd parsing routine
    value_type *m_pStackZero;
    ReversePolishNotation m_vByteCode;   ///< The Bytecode class.

    std::auto_ptr<token_reader_type> m_pTokenReader; ///< Managed pointer to the token reader object.

    funmap_type  m_FunDef;         ///< Map of function names and pointers.
    funmap_type  m_PostOprtDef;    ///< Postfix operator callbacks
    funmap_type  m_InfixOprtDef;   ///< unary infix operator.
    funmap_type  m_OprtDef;        ///< Binary operator callbacks
    valmap_type  m_ConstDef;       ///< user constants.
    strmap_type  m_StrVarDef;      ///< user defined string constants
    varmap_type  m_VarDef;         ///< user defind variables.

    bool m_bOptimize;              ///< Flag that indicates if the optimizer is on or off.

    EParserEngine m_eEngine;       ///< Specifies the parser engine to be used for the parsing
    string_type m_sNameChars;      ///< Charset for names
    string_type m_sOprtChars;      ///< Charset for postfix/ binary operator tokens
    string_type m_sInfixOprtChars; ///< Charset for infix operator tokens
    mutable int m_nIfElseCounter;  ///< Internal counter for keeping track of nested if-then-else clauses
    ExprCompiler m_compiler;
};

} // namespace mec

#endif


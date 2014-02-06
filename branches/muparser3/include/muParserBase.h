#ifndef _MU_PARSER_BASE_H
#define _MU_PARSER_BASE_H

//--- Standard includes ------------------------------------------------------------------------
#include <cmath>
#include <string>
#include <iostream>
#include <map>
#include <memory>
#include <locale>

//--- Parser includes --------------------------------------------------------------------------
#include "muParserDef.h"
#include "muParserTokenReader.h"
#include "muParserBytecode.h"
#include "muParserError.h"
#include "muParserStack.h"


MUP_NAMESPACE_START

//--------------------------------------------------------------------------------------------------
/** \brief Implementation of the parser engine.
*/
template<typename TValue, typename TString>
class ParserBase 
{
  friend class ParserTokenReader<TValue, TString>;

  protected:

  typedef TValue (ParserBase::*ParseFunction)() const;

  typedef Token<TValue, TString> token_type;
  typedef ParserTokenReader<TValue, TString> token_reader_type;
  typedef TValue* (*facfun_type)(const typename TString::value_type*, void*);
  typedef int (*identfun_type)(const typename TString::value_type *sExpr, int *nPos, TValue *fVal);
  typedef void (*fun_type)(TValue*, int narg);
  typedef std::basic_stringstream<typename TString::value_type,
                                  std::char_traits<typename TString::value_type>,
                                  std::allocator<typename TString::value_type> > stringstream_type;

  public:

    static TValue g_NullValue; ///< A value representing 0

    //---------------------------------------------------------------------------
    static void EnableDebugDump(bool bDumpCmd, bool bDumpStack)
    {
      ParserBase::g_DbgDumpCmdCode = bDumpCmd;
      ParserBase::g_DbgDumpStack   = bDumpStack;
    }

    //---------------------------------------------------------------------------------------------
    /** \brief Constructs an empty parser instance. 
    
      The parser is'nt of much use after default construction since it does not contain any
      functions, operators or constants.
    */
    ParserBase()
      :m_pParseFormula(&ParserBase::ParseString)
      ,m_vRPN()
      ,m_pTokenReader()
      ,m_FunDef()
      ,m_PostOprtDef()
      ,m_InfixOprtDef()
      ,m_OprtDef()
      ,m_ConstDef()
      ,m_VarDef()
      ,m_nIfElseCounter(0)
      ,m_vStackBuffer()
      ,m_nFinalResultIdx(0)
    {
      InitTokenReader();
    }

    //---------------------------------------------------------------------------------------------
    /** \brief Copy constructor.
    */
    ParserBase(const ParserBase &a_Parser)
      :m_pParseFormula(&ParserBase::ParseString)
      ,m_vRPN()
      ,m_pTokenReader()
      ,m_FunDef()
      ,m_PostOprtDef()
      ,m_InfixOprtDef()
      ,m_OprtDef()
      ,m_ConstDef()
      ,m_VarDef()
      ,m_nIfElseCounter(0)
    {
      m_pTokenReader.reset(new token_reader_type(this));
      Assign(a_Parser);
    }

    //---------------------------------------------------------------------------------------------
    /** \brief Copy parser state from another instance.

      Defined Functions, Operators, constants and variables will be copied from the other parser
      instance.
    */
    ParserBase& operator=(const ParserBase &a_Parser)
    {
      Assign(a_Parser);
      return *this;
    }

    //---------------------------------------------------------------------------------------------
    /** \brief Destructor.
    */
    virtual ~ParserBase()
    {}

    //---------------------------------------------------------------------------------------------
	  /** \brief Evaluate the expression. 
    
      When evaluating an ecpression for the first time the bytecode will be created. Every 
      successive call to Eval for the same expression with the same set of variable will invoke a 
      highly optimized evaluation function and speed up evaluation dramatically.
    */
    MUP_INLINE TValue  Eval() const
    {
      return (this->*m_pParseFormula)(); 
    }

    //---------------------------------------------------------------------------------------------
    /** \brief Evaluate an expression with multiple return values.
        \param nStackSize [out] The number of return values 
        \return Pointer to an array with the return values.

      This function will evaluate an expression and return a pointer to an array of its return 
      values.
    */
    TValue* Eval(int &nStackSize) const
    {
      ParseCmdCode();
      nStackSize = m_nFinalResultIdx;
      return &m_vStackBuffer[1];
    }

    //---------------------------------------------------------------------------------------------
    /** \brief Sets a new expression.
        \param a_sExpr a string containing the expression.
    */
    void SetExpr(const TString &a_sExpr)
    {
      TString sBuf(a_sExpr + _SL(" ") );
      m_pTokenReader->SetFormula(sBuf);
      ReInit();
    }

    //---------------------------------------------------------------------------------------------
    void SetVarFactory(facfun_type a_pFactory, void *pUserData = nullptr)
    {
      m_pTokenReader->SetVarCreator(a_pFactory, pUserData);  
    }

    //---------------------------------------------------------------------------------------------
    void AddValIdent(identfun_type a_pCallback)
    {
      m_pTokenReader->AddValIdent(a_pCallback);
    }

    //---------------------------------------------------------------------------------------------
    void DefineVar(const TString &a_sName, TValue *a_pVar)
    {
      if (a_pVar==0)
        Error(ecINVALID_VAR_PTR);

      // Test if a constant with that names already exists
      if (m_ConstDef.find(a_sName)!=m_ConstDef.end())
        Error(ecNAME_CONFLICT);

      CheckName(a_sName, c_sNameChars);
      m_VarDef[a_sName] = a_pVar;
      ReInit();
    }

    //---------------------------------------------------------------------------------------------
    void DefineConst(const TString &a_sName, TValue a_fVal)
    {
      CheckName(a_sName, c_sNameChars);
      m_ConstDef[a_sName] = a_fVal;
      ReInit();
    }

    //---------------------------------------------------------------------------------------------
    void DefineOprt(const TString &a_sName, 
                    fun_type a_pFun, 
                    unsigned a_iPrec=0, 
                    EOprtAssociativity a_eAssociativity = oaLEFT)
    {
      token_type tok;
      tok.SetFun(cmOPRT_BIN, a_pFun, 2, a_eAssociativity, a_iPrec, a_sName);
      AddCallback(a_sName, tok, m_OprtDef, c_sOprtChars);
    }

    //---------------------------------------------------------------------------------------------
    void DefineFun(const TString &a_sName, fun_type a_pFun, int argc)
    {
      token_type tok;
      tok.SetFun(cmFUNC, a_pFun, argc, oaNONE, 0, a_sName);
      AddCallback(a_sName, tok, m_FunDef, c_sNameChars );
    }

    //---------------------------------------------------------------------------------------------
    void DefinePostfixOprt(const TString &a_sName, fun_type a_pFun)
    {
      token_type tok;
      tok.SetFun(cmOPRT_POSTFIX, a_pFun, 1, oaNONE, prPOSTFIX, a_sName);
      AddCallback(a_sName, tok, m_PostOprtDef, c_sOprtChars);
    }

    //---------------------------------------------------------------------------------------------
    void DefineInfixOprt(const TString &a_sName, fun_type a_pFun, int a_iPrec=prINFIX)
    {
      token_type tok;
      tok.SetFun(cmOPRT_INFIX, a_pFun, 1, oaNONE, a_iPrec, a_sName);
      AddCallback(a_sName, tok, m_InfixOprtDef, c_sInfixOprtChars);
    }

    //---------------------------------------------------------------------------------------------
    void ClearVar()
    {
      m_VarDef.clear();
      ReInit();
    }

    //---------------------------------------------------------------------------------------------
    void RemoveVar(const TString &a_strVarName)
    {
      auto item = m_VarDef.find(a_strVarName);
      if (item!=m_VarDef.end())
      {
        m_VarDef.erase(item);
        ReInit();
      }
    }

    //---------------------------------------------------------------------------------------------
    int GetNumResults() const
    {
      return m_nFinalResultIdx;
    }

    //---------------------------------------------------------------------------------------------
    const std::map<TString, TValue*>& GetUsedVar() const
    {
      try
      {
        m_pTokenReader->IgnoreUndefVar(true);
        CreateRPN(); // try to create bytecode, but don't use it for any further calculations since it
                     // may contain references to nonexisting variables.
        m_pParseFormula = &ParserBase::ParseString;
        m_pTokenReader->IgnoreUndefVar(false);
      }
      catch(ParserError<TString> &e)
      {
        // Make sure to stay in string parse mode, dont call ReInit()
        // because it deletes the array with the used variables
        m_pParseFormula = &ParserBase::ParseString;
        m_pTokenReader->IgnoreUndefVar(false);
        throw e;
      }
    
      return m_pTokenReader->GetUsedVar();
    }

    //---------------------------------------------------------------------------------------------
    const std::map<TString, TValue*>& GetVar() const
    {
      return m_VarDef;
    }

    //---------------------------------------------------------------------------------------------
    const std::map<TString, TValue>& GetConst() const
    {
      return m_ConstDef;
    }

    //---------------------------------------------------------------------------------------------
    const TString& GetExpr() const
    {
      return m_pTokenReader->GetExpr();
    }

    //---------------------------------------------------------------------------------------------
    const ParserByteCode<TValue, TString>& GetByteCode() const
    {
      return m_vRPN;
    }

    //---------------------------------------------------------------------------------------------
    TString GetVersion(EParserVersionInfo eInfo = pviFULL) const
    {
      TString sCompileTimeSettings;
    
      stringstream_type ss;

      ss << MUP_VERSION;

      if (eInfo==pviFULL)
      {
        ss << _SL(" (") << MUP_VERSION_DATE;
        ss << std::dec << _SL("; ") << sizeof(void*)*8 << _SL("BIT");

  #ifdef _DEBUG
        ss << _SL("; DEBUG");
  #else 
        ss << _SL("; RELEASE");
  #endif

  #if defined(MUP_MATH_EXCEPTIONS)
      ss << _SL("; MATHEXC");
  #endif

        ss << _SL(")");
      }

      return ss.str();
    }

    //---------------------------------------------------------------------------------------------
    void Error(EErrorCodes a_iErrc, 
               int a_iPos = (int)TString::npos, 
               const TString &a_sTok = TString() ) const
    {
      throw ParserError<TString>(a_iErrc, a_sTok, m_pTokenReader->GetExpr(), a_iPos);
    }

  protected:

    static const typename TString::value_type* c_DefaultOprt[];
    static const typename TString::value_type* c_sNameChars;
    static const typename TString::value_type* c_sOprtChars;
    static const typename TString::value_type* c_sInfixOprtChars;

    static bool g_DbgDumpCmdCode;
    static bool g_DbgDumpStack;

 private:

    //---------------------------------------------------------------------------
    void Assign(const ParserBase &a_Parser)
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
      m_nIfElseCounter  = a_Parser.m_nIfElseCounter;
      m_pTokenReader.reset(a_Parser.m_pTokenReader->Clone(this));

      // Copy function and operator callbacks
      m_FunDef = a_Parser.m_FunDef;             // Copy function definitions
      m_PostOprtDef = a_Parser.m_PostOprtDef;   // post value unary operators
      m_InfixOprtDef = a_Parser.m_InfixOprtDef; // unary operators for infix notation
      m_OprtDef = a_Parser.m_OprtDef;           // binary operators
    }

    //---------------------------------------------------------------------------------------------
    void InitTokenReader()
    {
      m_pTokenReader.reset(new token_reader_type(this));
    }

    //---------------------------------------------------------------------------------------------
    void ReInit() const
    {
      m_pParseFormula = &ParserBase::ParseString;
      m_vRPN.Clear();
      m_pTokenReader->ReInit();
      m_nIfElseCounter = 0;
    }

    //---------------------------------------------------------------------------
    void AddCallback(const TString &a_sName,
                     const token_type &tok, 
                     std::map<TString, token_type> &a_Storage,
                     const typename TString::value_type *a_szCharSet)
    {
      if (tok.Fun.ptr==nullptr)
          Error(ecINVALID_FUN_PTR);

      const std::map<TString, token_type> *pFunMap = &a_Storage;

      // Check for conflicting operator or function names
      if ( pFunMap!=&m_FunDef && m_FunDef.find(a_sName)!=m_FunDef.end() )
        Error(ecNAME_CONFLICT, -1, a_sName);

      if ( pFunMap!=&m_PostOprtDef && m_PostOprtDef.find(a_sName)!=m_PostOprtDef.end() )
        Error(ecNAME_CONFLICT, -1, a_sName);

      if ( pFunMap!=&m_InfixOprtDef && pFunMap!=&m_OprtDef && m_InfixOprtDef.find(a_sName)!=m_InfixOprtDef.end() )
        Error(ecNAME_CONFLICT, -1, a_sName);

      if ( pFunMap!=&m_InfixOprtDef && pFunMap!=&m_OprtDef && m_OprtDef.find(a_sName)!=m_OprtDef.end() )
        Error(ecNAME_CONFLICT, -1, a_sName);

      if ( !a_sName.length() ||
          (a_sName.find_first_not_of(a_szCharSet)!=TString::npos) ||
          (a_sName[0]>='0' && a_sName[0]<='9'))
      {
        switch(tok.Cmd)
        {
        case cmOPRT_POSTFIX: Error(ecINVALID_POSTFIX_IDENT, -1, a_sName);
        case cmOPRT_INFIX:   Error(ecINVALID_INFIX_IDENT, -1, a_sName);
        default:             Error(ecINVALID_NAME, -1, a_sName);
        }
      }

      a_Storage[a_sName] = tok;
      ReInit();
    }

    //-----------------------------------------------------------------------------------------------
    void ApplyFunc(ParserStack<token_type> &a_stOpt,
                   ParserStack<token_type> &a_stVal, 
                   int iArgCount) const
    { 
      assert(m_pTokenReader.get());

      // Operator stack empty or does not contain tokens with callback functions
      if (a_stOpt.empty() || a_stOpt.top().Fun.ptr==nullptr)
        return;

      token_type funTok = a_stOpt.pop();
      assert(funTok.Fun.ptr);

      // Check the numbe of function arguments against the number required by the function token
      if ( funTok.Fun.argc>=0 && iArgCount>funTok.Fun.argc) 
        Error(ecTOO_MANY_PARAMS, m_pTokenReader->GetPos()-1, funTok.Ident);

      if (funTok.Cmd!=cmOPRT_BIN && iArgCount<funTok.Fun.argc )
        Error(ecTOO_FEW_PARAMS, m_pTokenReader->GetPos()-1, funTok.Ident);

      // Collect the numeric function arguments from the value stack and store them
      // in a vector
      std::vector<token_type> stArg;  
      for (int i=0; i<iArgCount; ++i)
        stArg.push_back( a_stVal.pop() );

      funTok.Fun.argc = iArgCount;
      m_vRPN.AddFun(funTok);

      // Push dummy value representing the function result to the stack
      token_type token;
      token.Val.mul = 1;
      a_stVal.push(token);
    }

    //---------------------------------------------------------------------------
    void ApplyBinOprt(ParserStack<token_type> &a_stOpt,
                      ParserStack<token_type> &a_stVal) const
    {
      if (a_stOpt.top().Cmd==cmOPRT_BIN)
      {
        ApplyFunc(a_stOpt, a_stVal, 2);
      }
      else
      {
        MUP_ASSERT(a_stVal.size()>=2);
        token_type valTok1 = a_stVal.pop(),
               valTok2 = a_stVal.pop(),
               optTok  = a_stOpt.pop(),
               resTok; 

        if (optTok.Cmd==cmASSIGN)
        {
          if (valTok2.Cmd!=cmVAR)
            Error(ecUNEXPECTED_OPERATOR, -1, _SL("="));
                      
          optTok.Oprt.ptr = valTok2.Val.ptr;
          m_vRPN.AddAssignOp(optTok);
        }

        token_type tok;
        tok.SetVal(1);
        a_stVal.push(tok);
      }
    }

    //---------------------------------------------------------------------------
    void ApplyRemainingOprt(ParserStack<token_type> &stOpt,
                            ParserStack<token_type> &stVal) const
    {
      while (stOpt.size() && 
             stOpt.top().Cmd != cmBO)
      {
        token_type tok = stOpt.top();
        switch (tok.Cmd)
        {
        case cmOPRT_INFIX:
        case cmOPRT_BIN:
        case cmASSIGN:
            if (stOpt.top().Cmd==cmOPRT_INFIX)
              ApplyFunc(stOpt, stVal, 1);
            else
              ApplyBinOprt(stOpt, stVal);
            break;

        default:
            Error(ecINTERNAL_ERROR, 1);
        }
      }
    }


    //---------------------------------------------------------------------------
    int GetOprtPrecedence(const token_type &tok) const
    {
      switch (tok.Cmd)
      {
      // built in operators
      case cmEND:      return -5;
      case cmARG_SEP:  return -4;
      case cmASSIGN:   return -1;               

      // user defined binary operators
      case cmOPRT_INFIX: 
      case cmOPRT_BIN: return tok.Fun.prec;
      default:  Error(ecINTERNAL_ERROR, 5);
                return 999;
      }  
    }

    //---------------------------------------------------------------------------
    EOprtAssociativity GetOprtAssociativity(const token_type &tok) const
    {
      switch (tok.Cmd)
      {
      case cmASSIGN:
      case cmOPRT_BIN: return tok.Fun.asoc;
      default:         return oaNONE;
      }  
    }

    //---------------------------------------------------------------------------
    void CreateRPN() const
    {
      if (!m_pTokenReader->GetExpr().length())
        Error(ecUNEXPECTED_EOF, 0);

      ParserStack<token_type> stOpt, stVal;
      ParserStack<int> stArgCount;
      token_type opta, opt;  // for storing operators

      ReInit();
    
      // The outermost counter counts the number of seperated items
      // such as in "a=10,b=20,c=c+a"
      stArgCount.push(1);
    
      for(;;)
      {
        opt = m_pTokenReader->ReadNextToken();

        switch (opt.Cmd)
        {
          case cmVAL_EX:
          case cmVAR:
          case cmVAL:
                  stVal.push(opt);
                  opt.Cmd = cmVAL_EX;
                  m_vRPN.AddVal(opt);
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
                    if (opta.Cmd==cmBO)
                      --stArgCount.top();
                  
                    ApplyRemainingOprt(stOpt, stVal);

                    // Check if the bracket content has been evaluated completely
                    if (stOpt.size() && stOpt.top().Cmd==cmBO)
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

                      if (iArgCount>1 && ( stOpt.size()==0 || stOpt.top().Cmd!=cmFUNC ) )
                        Error(ecUNEXPECTED_ARG, m_pTokenReader->GetPos());
                    
                      // The opening bracket was popped from the stack now check if there
                      // was a function before this bracket
                      if (stOpt.size() && stOpt.top().Cmd==cmFUNC)
                        ApplyFunc(stOpt, stVal, iArgCount);
                    }
                  } // if bracket content is evaluated
                  break;

          //
          // Next are the binary operator entries
          //
          case cmASSIGN:
          case cmOPRT_BIN:

                  // A binary operator (user defined or built in) has been found. 
                  while ( stOpt.size() && 
                          stOpt.top().Cmd != cmBO)
                  {
                    int nPrec1 = GetOprtPrecedence(stOpt.top()),
                        nPrec2 = GetOprtPrecedence(opt);

                    const token_type &top = stOpt.top();
                    if (top.Cmd==opt.Cmd && top.Ident==opt.Ident)
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
                  
                    if (stOpt.top().Cmd==cmOPRT_INFIX)
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
                  stOpt.push(opt);
                  break;

          case cmOPRT_POSTFIX:
                  stOpt.push(opt);
                  ApplyFunc(stOpt, stVal, 1);  // this is the postfix operator
                  break;

          default:  Error(ecINTERNAL_ERROR, 3);
        } // end of switch operator-token

        opta = opt;

        if ( opt.Cmd == cmEND )
          break;

        if (ParserBase::g_DbgDumpStack)
        {
          StackDump(stVal, stOpt);
          m_vRPN.AsciiDump();
        }
      } // while (true)

      if (m_nIfElseCounter>0)
        Error(ecMISSING_ELSE_CLAUSE);

      // get the last value (= final result) from the stack
      MUP_ASSERT(stArgCount.size()==1);
      m_nFinalResultIdx = stArgCount.top();
      if (m_nFinalResultIdx==0)
        Error(ecINTERNAL_ERROR, 6);

      if (stVal.size()==0)
        Error(ecEMPTY_EXPRESSION);

      m_vStackBuffer.resize(m_vRPN.GetMaxStackSize());
      m_pRPN   = m_vRPN.GetBase();
      m_pStack = &m_vStackBuffer[0];
      m_vRPN.Finalize();

      if (ParserBase::g_DbgDumpCmdCode)
        m_vRPN.AsciiDump();
    }

    //---------------------------------------------------------------------------------------------
    TValue ParseString() const
    {
      CreateRPN();
    
      if (m_nFinalResultIdx!=1)
      {
        // Functions with multiple return values do not use short expression 
        // optimization
        m_pParseFormula = &ParserBase::ParseCmdCode; 
      }
      else
      {
        EEngineCode ec = m_vRPN.GetEngineCode();
        bool bNoMul = (ec & ecNO_MUL) != 0;
        switch(ec & ~ecNO_MUL)
        {   
        case ecV:    if (m_pRPN[0].Val.mul==0)
                       m_pParseFormula = &ParserBase::ParseCmdCode_V1;
                     else if (m_pRPN[0].Val.mul!=0 && m_pRPN[0].Val.fixed==0)
                       m_pParseFormula = &ParserBase::ParseCmdCode_V2;
                     else
                       m_pParseFormula = &ParserBase::ParseCmdCode_V3;
                     break;

        case ecVF:    m_pParseFormula = (bNoMul) ? &ParserBase::ParseCmdCode_VF   : &ParserBase::ParseCmdCode_XF;   break;
        case ecVFF:   m_pParseFormula = (bNoMul) ? &ParserBase::ParseCmdCode_VFF  : &ParserBase::ParseCmdCode_XFF;  break;
        case ecVVF:   m_pParseFormula = (bNoMul) ? &ParserBase::ParseCmdCode_VVF  : &ParserBase::ParseCmdCode_XXF;  break;
        case ecVFFF:  m_pParseFormula = (bNoMul) ? &ParserBase::ParseCmdCode_VFFF : &ParserBase::ParseCmdCode_XFFF; break;
        case ecVFVF:  m_pParseFormula = (bNoMul) ? &ParserBase::ParseCmdCode_VFVF : &ParserBase::ParseCmdCode_XFXF; break;
        case ecVVFF:  m_pParseFormula = (bNoMul) ? &ParserBase::ParseCmdCode_VVFF : &ParserBase::ParseCmdCode_XXFF; break;
        case ecVVVF:  m_pParseFormula = (bNoMul) ? &ParserBase::ParseCmdCode_VVVF : &ParserBase::ParseCmdCode_XXXF; break;

        case ecUNOPTIMIZABLE:
        default: 
              m_pParseFormula = &ParserBase::ParseCmdCode; 
              break;
        }
      }
      return (this->*m_pParseFormula)(); 
    }

    //---------------------------------------------------------------------------------------------
    // Parsing engines
    TValue ParseCmdCode() const
    {
      TValue *Stack = m_pStack;
      const typename token_type::SValDef *pVal = nullptr;
      const typename token_type::SFunDef *pFun = nullptr;

      register int sidx(0);

      for (const token_type *pTok = m_vRPN.GetBase(); pTok->Cmd!=cmEND; ++pTok)
      {
        switch (pTok->Cmd)
        {
        case  cmASSIGN: 
              --sidx; Stack[sidx] = *pTok->Oprt.ptr = Stack[sidx+1]; 
              continue;

        case  cmVAL_EX: 
              { 
                pVal = &(pTok->Val);
                Stack[++sidx] = *pVal->ptr * pVal->mul + pVal->fixed; 
              }
              continue;

        case  cmVAR:   ++sidx; Stack[sidx] = *pTok->Val.ptr;   continue;
        case  cmVAL:   ++sidx; Stack[sidx] =  pTok->Val.fixed; continue;

        case  cmFUNC:
              { 
                pFun = &(pTok->Fun);
                sidx -= pFun->argc - 1;
                (*pFun->ptr)(&Stack[sidx], pFun->argc);
              }
              continue;
      
        default:
              Error(ecINTERNAL_ERROR, 2);
              return 0;
        } // switch CmdCode
      } // for all bytecode tokens

      return Stack[m_nFinalResultIdx];  
    }

  #define SXO_INIT   \
          int sidx = 0;

  #define SXO_VAX(TOK,IDX) \
          m_pStack[++sidx] = (TOK[IDX])->Val.fixed + *(TOK[IDX])->Val.ptr * (TOK[IDX])->Val.mul;  \

  #define SXO_VAL(TOK,IDX) \
          m_pStack[++sidx] = (TOK[IDX])->Val.fixed + *(TOK[IDX])->Val.ptr;  \

  #define SXO_CON(TOK,IDX) \
          m_pStack[++sidx] = (TOK[IDX])->Val.fixed;  \

  #define SXO_FUN(TOK,IDX) \
          {                                                         \
            const typename token_type::SFunDef &fun = (TOK[IDX])->Fun;   \
            (*fun.ptr)(&m_pStack[sidx -= fun.argc - 1], fun.argc);  \
          }

  #define SXO_RET   \
        return m_pStack[1];

  #define ParseFunc2(What, OP1, OP2)           \
      TValue ParseCmdCode_##What() const       \
      {                                        \
        SXO_INIT                               \
        SXO_##OP1(&m_pRPN,0)                  \
        SXO_##OP2(&m_pRPN,1)                  \
        SXO_RET                                \
      }

  #define ParseFunc3(What, OP1, OP2, OP3)      \
      TValue ParseCmdCode_##What() const       \
      {                                        \
        SXO_INIT                               \
        SXO_##OP1(&m_pRPN,0)                  \
        SXO_##OP2(&m_pRPN,1)                  \
        SXO_##OP3(&m_pRPN,2)                  \
        SXO_RET                                \
      }

  #define ParseFunc4(What, OP1, OP2, OP3, OP4) \
      TValue ParseCmdCode_##What() const       \
      {                                        \
        SXO_INIT                               \
        SXO_##OP1(&m_pRPN,0)                  \
        SXO_##OP2(&m_pRPN,1)                  \
        SXO_##OP3(&m_pRPN,2)                  \
        SXO_##OP4(&m_pRPN,3)                  \
        SXO_RET                                \
      }

    //---------------------------------------------------------------------------------------------
    TValue ParseCmdCode_V1() const
    {    
      return m_pRPN->Val.fixed;
    }

    //---------------------------------------------------------------------------------------------
    TValue ParseCmdCode_V2() const
    {    
      return *m_pRPN->Val.ptr * m_pRPN->Val.mul;
    }

    //---------------------------------------------------------------------------------------------
    TValue ParseCmdCode_V3() const
    {    
      return *m_pRPN->Val.ptr * m_pRPN->Val.mul + m_pRPN->Val.fixed;
    }

    //---------------------------------------------------------------------------------------------
    ParseFunc2(VF,   VAL, FUN)
    ParseFunc3(VFF,  VAL, FUN, FUN)
    ParseFunc3(VVF,  VAL, VAL, FUN)
    ParseFunc4(VFFF, VAL, FUN, FUN, FUN)
    ParseFunc4(VFVF, VAL, FUN, VAL, FUN)
    ParseFunc4(VVFF, VAL, VAL, FUN, FUN)
    ParseFunc4(VVVF, VAL, VAL, VAL, FUN)

/*
#define ParseFunc4(What, OP1, OP2, OP3, OP4) \
    TValue ParseCmdCode_##What() const       \
    {                                        \
      SXO_INIT                               \
      SXO_##OP1(&m_pRPN[0])                  \
      SXO_##OP2(&m_pRPN[1])                  \
      SXO_##OP3(&m_pRPN[2])                  \
      SXO_##OP4(&m_pRPN[3])                  \
      SXO_RET                                \
    }

#define ParseFunc(ARG1, ARG2, ARG3, ARG4, ARG5) ParseFunc4(ARG1, 1, ARG2, 2, ARG3, 3, ARG4, 4, ARG5)

//  ParseFunc(VVVF, VAL, VAL, VAL, FUN)
//    ParseFunc4(VVVF, 1, VAL, 2, VAL, 3, VAL, 4, FUN)
*/

/*
    //---------------------------------------------------------------------------------------------
    TValue ParseCmdCode_VVVF() const
    {
      m_pStack[1] = m_pRPN[0]->Val.fixed + *(m_pRPN[0])->Val.ptr;
      m_pStack[2] = m_pRPN[1]->Val.fixed + *(m_pRPN[1])->Val.ptr;
      m_pStack[3] = m_pRPN[2]->Val.fixed + *(m_pRPN[2])->Val.ptr;
      {
        const typename token_type::SFunDef &fun = (m_pRPN[3])->Fun;
        (*fun.ptr)(&m_pStack[1], fun.argc);
      }
      return m_pStack[1];
    }

    //---------------------------------------------------------------------------------------------
    TValue ParseCmdCode_VVVF() const
    {
      *m_pStack_1 = m_pRPN_0->Val.fixed + *(m_pRPN_0->Val.ptr);
      *m_pStack_2 = m_pRPN_1->Val.fixed + *(m_pRPN_1->Val.ptr);
      *m_pStack_3 = m_pRPN_2->Val.fixed + *(m_pRPN_2->Val.ptr);
      (*m_pRPN_3->Fun.ptr)(&m_pStack_1, m_pRPN_3->Fun.argc);
      return *m_pStack_1;
    }

 */

    ParseFunc2(XF,   VAX, FUN)
    ParseFunc3(XFF,  VAX, FUN, FUN)
    ParseFunc3(XXF,  VAX, VAX, FUN)
    ParseFunc4(XFFF, VAX, FUN, FUN, FUN)
    ParseFunc4(XFXF, VAX, FUN, VAX, FUN)
    ParseFunc4(XXFF, VAX, VAX, FUN, FUN)
    ParseFunc4(XXXF, VAX, VAX, VAX, FUN)

    //---------------------------------------------------------------------------------------------
    void CheckName(const TString &a_sName,
                   const TString &a_szCharSet) const
    {
      if ( !a_sName.length() ||
          (a_sName.find_first_not_of(a_szCharSet)!=TString::npos) ||
          (a_sName[0]>='0' && a_sName[0]<='9'))
      {
        Error(ecINVALID_NAME);
      }
    }

    //------------------------------------------------------------------------------
    void StackDump(const ParserStack<token_type> &a_stVal, 
                   const ParserStack<token_type> &a_stOprt) const
    {
      ParserStack<token_type> stOprt(a_stOprt), 
                              stVal(a_stVal);

      _OUT << _SL("\nValue stack:\n");

      while ( !stVal.empty() ) 
      {
        token_type val = stVal.pop();
        _OUT << _SL(" ") << val.Val.fixed << _SL(" ");
      }
      _OUT << "\nOperator stack:\n";

      while ( !stOprt.empty() )
      {
        if (stOprt.top().Cmd<=cmASSIGN) 
        {
          _OUT << _SL("OPRT_INTRNL \"")
               << ParserBase::c_DefaultOprt[stOprt.top().Cmd] 
               << _SL("\" \n");
        }
        else
        {
          switch(stOprt.top().Cmd)
          {
          case cmVAR:        _OUT << _SL("VAR\n");  break;
          case cmVAL:        _OUT << _SL("VAL\n");  break;
          case cmFUNC:       _OUT << _SL("FUNC \"")
                                  << stOprt.top().Ident
                                  << _SL("\"\n");   break;
          case cmOPRT_INFIX: _OUT << _SL("OPRT_INFIX \"")
                                  << stOprt.top().Ident
                                  << _SL("\"\n");          break;
          case cmOPRT_BIN:   _OUT << _SL("OPRT_BIN \"")
                                  << stOprt.top().Ident
                                  << _SL("\"\n");          break;
          case cmEND:        _OUT << _SL("END\n");            break;
          case cmBO:         _OUT << _SL("BRACKET \"(\"\n");  break;
          case cmBC:         _OUT << _SL("BRACKET \")\"\n");  break;
          default:           _OUT << stOprt.top().Cmd << _SL(" ");  break;
          }
        }	
        stOprt.pop();
      }

      _OUT << std::dec << std::endl;
    }

    mutable ParseFunction  m_pParseFormula;
    mutable ParserByteCode<TValue, TString> m_vRPN;

    std::unique_ptr<token_reader_type> m_pTokenReader;

    std::map<TString, token_type>  m_FunDef;
    std::map<TString, token_type>  m_PostOprtDef;
    std::map<TString, token_type>  m_InfixOprtDef;
    std::map<TString, token_type>  m_OprtDef;
    std::map<TString, TValue>   m_ConstDef;
    std::map<TString, TValue*>  m_VarDef;

    mutable int m_nIfElseCounter;

    mutable const token_type *m_pRPN;
    mutable TValue *m_pStack;
    mutable std::vector<TValue> m_vStackBuffer;
    mutable int m_nFinalResultIdx;
};

  template<typename TValue, typename TString>
  const typename TString::value_type* ParserBase<TValue, TString>::c_DefaultOprt[] = { _SL("="), 
                                                                                       _SL("("), 
                                                                                       _SL(")"),
                                                                                       nullptr };

  template<typename TValue, typename TString>
  const typename TString::value_type* ParserBase<TValue, TString>::c_sNameChars = _SL("0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
  
  template<typename TValue, typename TString>
  const typename TString::value_type* ParserBase<TValue, TString>::c_sOprtChars = _SL("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*^/?<>=#!$%&|~'_{}");

  template<typename TValue, typename TString>
  const typename TString::value_type* ParserBase<TValue, TString>::c_sInfixOprtChars = _SL("/+-*^?<>=#!$%&|~'_");

  template<typename TValue, typename TString>
  bool ParserBase<TValue, TString>::g_DbgDumpCmdCode = false;

  template<typename TValue, typename TString>
  bool ParserBase<TValue, TString>::g_DbgDumpStack = false;

  template<typename TValue, typename TString>
  TValue ParserBase<TValue, TString>::g_NullValue = 0;
} // namespace mu

#endif


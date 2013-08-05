/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
  Copyright (C) 2004-2012 Ingo Berg

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
#ifndef MU_PARSER_ERROR_H
#define MU_PARSER_ERROR_H

#include <cassert>
#include <string>
#include <sstream>
#include <vector>


MUP_NAMESPACE_START

  //-----------------------------------------------------------------------------------------------
  /** \brief A class that handles the error messages.
  */
  template<typename TString>
  class ParserErrorMsg
  {
  public:

    //---------------------------------------------------------------------------------------------
    ParserErrorMsg()
      :m_vErrMsg(0)
    {
      m_vErrMsg.resize(ecCOUNT);

      m_vErrMsg[ecUNASSIGNABLE_TOKEN]     = _SL("Unexpected token \"$TOK$\" found at position $POS$.");
      m_vErrMsg[ecINTERNAL_ERROR]         = _SL("Internal error");
      m_vErrMsg[ecINVALID_NAME]           = _SL("Invalid function-, variable- or constant name: \"$TOK$\".");
      m_vErrMsg[ecINVALID_BINOP_IDENT]    = _SL("Invalid binary operator identifier: \"$TOK$\".");
      m_vErrMsg[ecINVALID_INFIX_IDENT]    = _SL("Invalid infix operator identifier: \"$TOK$\".");
      m_vErrMsg[ecINVALID_POSTFIX_IDENT]  = _SL("Invalid postfix operator identifier: \"$TOK$\".");
      m_vErrMsg[ecINVALID_FUN_PTR]        = _SL("Invalid pointer to callback function.");
      m_vErrMsg[ecEMPTY_EXPRESSION]       = _SL("Expression is empty.");
      m_vErrMsg[ecINVALID_VAR_PTR]        = _SL("Invalid pointer to variable.");
      m_vErrMsg[ecUNEXPECTED_OPERATOR]    = _SL("Unexpected operator \"$TOK$\" found at position $POS$");
      m_vErrMsg[ecUNEXPECTED_EOF]         = _SL("Unexpected end of expression at position $POS$");
      m_vErrMsg[ecUNEXPECTED_ARG_SEP]     = _SL("Unexpected argument separator at position $POS$");
      m_vErrMsg[ecUNEXPECTED_PARENS]      = _SL("Unexpected parenthesis \"$TOK$\" at position $POS$");
      m_vErrMsg[ecUNEXPECTED_FUN]         = _SL("Unexpected function \"$TOK$\" at position $POS$");
      m_vErrMsg[ecUNEXPECTED_VAL]         = _SL("Unexpected value \"$TOK$\" found at position $POS$");
      m_vErrMsg[ecUNEXPECTED_VAR]         = _SL("Unexpected variable \"$TOK$\" found at position $POS$");
      m_vErrMsg[ecUNEXPECTED_ARG]         = _SL("Function arguments used without a function (position: $POS$)");
      m_vErrMsg[ecMISSING_PARENS]         = _SL("Missing parenthesis");
      m_vErrMsg[ecTOO_MANY_PARAMS]        = _SL("Too many parameters for function \"$TOK$\" at expression position $POS$");
      m_vErrMsg[ecTOO_FEW_PARAMS]         = _SL("Too few parameters for function \"$TOK$\" at expression position $POS$");
      m_vErrMsg[ecDIV_BY_ZERO]            = _SL("Divide by zero");
      m_vErrMsg[ecDOMAIN_ERROR]           = _SL("Domain error");
      m_vErrMsg[ecNAME_CONFLICT]          = _SL("Name conflict");
      m_vErrMsg[ecOPT_PRI]                = _SL("Invalid value for operator priority (must be greater or equal to zero).");
      m_vErrMsg[ecBUILTIN_OVERLOAD]       = _SL("user defined binary operator \"$TOK$\" conflicts with a built in operator.");
      m_vErrMsg[ecVAL_EXPECTED]           = _SL("String value used where a numerical argument is expected.");
      m_vErrMsg[ecGENERIC]                = _SL("Parser error.");
      m_vErrMsg[ecLOCALE]                 = _SL("Decimal separator is identic to function argument separator.");
      m_vErrMsg[ecUNEXPECTED_CONDITIONAL] = _SL("The \"$TOK$\" operator must be preceeded by a closing bracket.");
      m_vErrMsg[ecMISSING_ELSE_CLAUSE]    = _SL("If-then-else operator is missing an else clause");
      m_vErrMsg[ecMISPLACED_COLON]        = _SL("Misplaced colon at position $POS$");

      #if defined(_DEBUG)
        for (int i=0; i<ecCOUNT; ++i)
          if (!m_vErrMsg[i].length())
            assert(false);
      #endif
    }

    //---------------------------------------------------------------------------------------------
    ~ParserErrorMsg()
    {}

    //---------------------------------------------------------------------------------------------
    static const ParserErrorMsg& Instance()
    {
      return m_Instance;
    }

    //---------------------------------------------------------------------------------------------
    TString operator[](unsigned a_iIdx) const
    {
      return (a_iIdx<m_vErrMsg.size()) ? m_vErrMsg[a_iIdx] : TString();
    }


  private:
    std::vector<TString>  m_vErrMsg;  ///< A vector with the predefined error messages
    static const ParserErrorMsg m_Instance;

    ParserErrorMsg(const ParserErrorMsg&);
    ParserErrorMsg& operator=(const ParserErrorMsg &);
  };

  template<typename TStr>
  const ParserErrorMsg<TStr> ParserErrorMsg<TStr>::m_Instance;

  //-----------------------------------------------------------------------------------------------
  /** \brief Error class of the parser. 

    muparser needs its own error class since std::exception does not provides unicode string 
    messages.
  */
  template<typename TString>
  class ParserError
  {
  private:

    typedef std::basic_stringstream<typename TString::value_type,
                                    std::char_traits<typename TString::value_type>,  
                                    std::allocator<typename TString::value_type> > stringstream_type;

    //---------------------------------------------------------------------------------------------
    /** \brief Replace all occurences of a substring with another string. */
    void ReplaceSubString(TString &strSource,
                          const TString &strFind,
                          const TString &strReplaceWith)
    {
      TString strResult;
      TString::size_type iPos(0), iNext(0);

      for(;;)
      {
        iNext = strSource.find(strFind, iPos);
        strResult.append(strSource, iPos, iNext-iPos);

        if( iNext==TString::npos )
          break;

        strResult.append(strReplaceWith);
        iPos = iNext + strFind.length();
      } 

      strSource.swap(strResult);
    }

    //---------------------------------------------------------------------------------------------
    void Reset()
    {
      m_sMsg  = _SL("");
      m_sExpr = _SL("");
      m_sTok  = _SL("");
      m_iPos  = -1;
      m_iErrc = ecUNDEFINED;
    }

  public:

    //---------------------------------------------------------------------------------------------
    ParserError()
      :m_strMsg()
      ,m_strFormula()
      ,m_strTok()
      ,m_iPos(-1)
      ,m_iErrc(ecUNDEFINED)
      ,m_ErrMsg(ParserErrorMsg<TString>::Instance())
    {}

    //---------------------------------------------------------------------------------------------
    explicit ParserError(EErrorCodes /*a_iErrc*/) 
      :m_ErrMsg(ParserErrorMsg<TString>::Instance())
    {
      Reset();
      m_sMsg = _SL("parser error");
    }

    //---------------------------------------------------------------------------------------------
    explicit ParserError(const TString &sMsg) 
      :m_ErrMsg(ParserErrorMsg<TString>::Instance())
    {
      Reset();
      m_sMsg = sMsg;
    }

    //---------------------------------------------------------------------------------------------
    ParserError(EErrorCodes iErrc,
                const TString &sTok,
                const TString &sExpr = TString(_SL("(mathematical expression is not available)")),
                int iPos = -1)
      :m_sMsg()
      ,m_sExpr(sExpr)
      ,m_sTok(sTok)
      ,m_iPos(iPos)
      ,m_iErrc(iErrc)
      ,m_ErrMsg(ParserErrorMsg<TString>::Instance())
    {
      m_sMsg = m_ErrMsg[m_iErrc];
      stringstream_type stream;
      stream << (int)m_iPos;
      ReplaceSubString(m_sMsg, _SL("$POS$"), stream.str());
      ReplaceSubString(m_sMsg, _SL("$TOK$"), m_sTok);
    }

    //---------------------------------------------------------------------------------------------
    ParserError(EErrorCodes a_iErrc, 
                int a_iPos, 
                const TString &sTok)
      :m_sMsg()
      ,m_sExpr()
      ,m_sTok(sTok)
      ,m_iPos(iPos)
      ,m_iErrc(iErrc)
      ,m_ErrMsg(ParserErrorMsg<TString>::Instance())
    {
      m_sMsg = m_ErrMsg[m_iErrc];
      stringstream_type stream;
      stream << (int)m_iPos;
      ReplaceSubString(m_sMsg, _SL("$POS$"), stream.str());
      ReplaceSubString(m_sMsg, _SL("$TOK$"), m_sTok);
    }

    //---------------------------------------------------------------------------------------------
    ParserError(const typename TString::value_type *szMsg, int iPos = -1, const TString &sTok = TString()) 
      :m_sMsg(szMsg)
      ,m_sExpr()
      ,m_sTok(sTok)
      ,m_iPos(iPos)
      ,m_iErrc(ecGENERIC)
      ,m_ErrMsg(ParserErrorMsg<TString>::Instance())
    {
      stringstream_type stream;
      stream << (int)m_iPos;
      ReplaceSubString(m_sMsg, _SL("$POS$"), stream.str());
      ReplaceSubString(m_sMsg, _SL("$TOK$"), m_sTok);
    }

    //---------------------------------------------------------------------------------------------
    ParserError(const ParserError &a_Obj)
      :m_sMsg(a_Obj.m_sMsg)
      ,m_sExpr(a_Obj.m_sExpr)
      ,m_sTok(a_Obj.m_sTok)
      ,m_iPos(a_Obj.m_iPos)
      ,m_iErrc(a_Obj.m_iErrc)
      ,m_ErrMsg(ParserErrorMsg<TString>::Instance())
    {}

    //---------------------------------------------------------------------------------------------
    ParserError& operator=(const ParserError &a_Obj)
    {
      if (this==&a_Obj)
        return *this;

      m_strMsg = a_Obj.m_strMsg;
      m_strFormula = a_Obj.m_strFormula;
      m_strTok = a_Obj.m_strTok;
      m_iPos = a_Obj.m_iPos;
      m_iErrc = a_Obj.m_iErrc;
      return *this;
    }

    //---------------------------------------------------------------------------------------------
   ~ParserError()
    {}

    //---------------------------------------------------------------------------------------------
    void SetFormula(const TString &a_strFormula)
    {
      m_strFormula = a_strFormula;
    }

    //---------------------------------------------------------------------------------------------
    const TString& GetExpr() const 
    {
      return m_sExpr;
    }

    //---------------------------------------------------------------------------------------------
    const TString& GetMsg() const
    {
      return m_sMsg;
    }

    //---------------------------------------------------------------------------------------------
    std::size_t GetPos() const
    {
      return m_iPos;
    }

    //---------------------------------------------------------------------------------------------
    const TString& GetToken() const
    {
      return m_sTok;
    }

    //---------------------------------------------------------------------------------------------
    EErrorCodes GetCode() const
    {
      return m_iErrc;
    }

  private:
    TString m_sMsg;       ///< The message string
    TString m_sExpr;      ///< Formula string
    TString m_sTok;       ///< Token related with the error
    int m_iPos;           ///< Formula position related to the error 
    EErrorCodes m_iErrc;  ///< Error code
    const ParserErrorMsg<TString> &m_ErrMsg;
  };		
} // namespace mu

#endif

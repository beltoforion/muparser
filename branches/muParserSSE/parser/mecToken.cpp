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
#include "mecToken.h"

namespace mec
{
  //---------------------------------------------------------------------------
  /** \brief Constructor (default).
      \throw nothrow
      \sa ECmdCode
    
    Sets token to an neutral state of type cmUNKNOWN.
  */
  Token::Token()
    :m_eCode(cmUNKNOWN)
    ,m_pVar(NULL)
    ,m_iFlags(0)
    ,m_sTok()
    ,m_pCallback()
  {}

  //------------------------------------------------------------------------------
  /** \brief Create token from another one.

      Implemented by calling Assign(...)
      \throw nothrow
      \sa #Assign
  */
  Token::Token(const Token &a_Tok)
  {
    Assign(a_Tok);
  }
      
  //------------------------------------------------------------------------------
  /** \brief Assignement operator. 

      Copy token state from another token and return this.
      Implemented by calling Assign(...).
      \throw nothrow
  */
  Token& Token::operator=(const Token &a_Tok)
  {
    Assign(a_Tok);
    return *this;
  }

  //------------------------------------------------------------------------------
  /** \brief Copy token information from argument.
      \throw nothrow
  */
  void Token::Assign(const Token &a_Tok)
  {
    m_eCode  = a_Tok.m_eCode;
    m_pVar   = a_Tok.m_pVar;
    m_iFlags = a_Tok.m_iFlags;
    m_sTok = a_Tok.m_sTok;
    m_fVal   = a_Tok.m_fVal;
    // create new callback object if a_Tok has one 
    m_pCallback.reset(a_Tok.m_pCallback.get() ? new Callback(*(a_Tok.m_pCallback.get())) : 0);
  }

  //------------------------------------------------------------------------------
  /** \brief Add additional flags to the token. 
      \sa m_iFlags, ETokFlags    

      Flags are currently used to mark volatile (non optimizeable) functions.
  */
  void Token::AddFlags(int a_iFlags)
  {
    m_iFlags |= a_iFlags;
  }

  //------------------------------------------------------------------------------
  /** \brief Check if a certain flag ist set. 

      \throw nothrow
  */
  bool Token::IsFlagSet(int a_iFlags) const
  {
    return (m_iFlags & a_iFlags)!=0;
  }

  //------------------------------------------------------------------------------
  /** \brief Assign a token type. 
      \pre [assert] a_iType!=cmVAR
      \pre [assert] a_iType!=cmVAL
      \pre [assert] a_iType!=cmFUNC
      \post m_fVal = 0
      \post m_pTok = 0

    Token may not be of type value, variable or function. Those have seperate set functions. 
  */
  Token& Token::Set(ECmdCode a_iType, const string_type &a_strTok)
  {
    // The following types cant be set this way, they have special Set functions
    assert(a_iType!=cmVAR);
    assert(a_iType!=cmVAL);
    assert(a_iType!=cmFUNC);

    m_eCode  = a_iType;
    m_pVar   = NULL;
    m_iFlags = 0;
    m_sTok = a_strTok;

    return *this;
  }

  //------------------------------------------------------------------------------
  /** \brief Set Callback type. */
  Token& Token::Set(ECmdCode eCode, const Callback &a_pCallback, const string_type &a_sTok)
  {
    assert(a_pCallback.m_pFun);

    m_eCode = eCode;
    m_sTok = a_sTok;
    m_pCallback.reset(new Callback(a_pCallback));
    m_pVar   = NULL;
    m_iFlags = 0;
    
    return *this;
  }

  //------------------------------------------------------------------------------
  /** \brief Make this token a value token. 
      \throw nothrow

      Member variables not necessary for value tokens will be invalidated.
  */
  Token& Token::SetVal(value_type a_fVal, const string_type &a_strTok)
  {
    m_eCode = cmVAL;
    m_fVal = a_fVal;
    m_iFlags = 0;
    m_sTok = a_strTok;
    
    m_pVar = NULL;
    m_pCallback.reset(0);

    return *this;
  }

  //------------------------------------------------------------------------------
  /** \brief make this token a variable token. 
      \throw nothrow

      Member variables not necessary for variable tokens will be invalidated.
  */
  Token& Token::SetVar(value_type *a_pVar, const string_type &a_strTok)
  {
    m_eCode  = cmVAR;
    m_iFlags = 0;
    m_sTok = a_strTok;
    m_pVar   = a_pVar;
    m_pCallback.reset(0);

    AddFlags(flVOLATILE);
    return *this;
  }

  //------------------------------------------------------------------------------
  /** \brief Return the token type.
      \throw nothrow
  */
  ECmdCode Token::GetCode() const
  {
    return m_eCode;
  }

  //------------------------------------------------------------------------------
  /** \biref Get value of the token.
    
      Only applicable to variable and value tokens.
      \throw exception_type if token is no value/variable token.
  */
  value_type Token::GetVal() const
  {
    switch (m_eCode)
    {
    case cmVAL:  return  m_fVal;
    case cmVAR:  return *m_pVar;
    default:     throw ParserError(ecVAL_EXPECTED);
    }
  }

  //------------------------------------------------------------------------------
  /** \brief Get address of a variable token.

    \throw exception_type if token is no variable token.
  */
  value_type* Token::GetVar() const
  {
    if (m_eCode!=cmVAR)
      throw ParserError(ecINTERNAL_ERROR);

    return m_pVar;
  }

  //------------------------------------------------------------------------------
  bool Token::IsFunction() const
  {
    return m_eCode==cmFUNC || (m_eCode>=cmSIN && m_eCode<=cmSQRT);
  }

  //------------------------------------------------------------------------------
  /** \brief Return the number of function arguments. 
  */
  int Token::GetArgCount() const
  {
    if (m_eCode>=cmSIN && m_eCode<=cmSQRT)
      return 1;
    else
      return m_pCallback->m_nArgc;
  }

  //------------------------------------------------------------------------------
  /** \brief Return the token identifier. 
      \return #m_sTok
      \throw nothrow
      \sa m_sTok
  */
  const string_type& Token::GetAsString() const
  {
    return m_sTok;
  }

  //------------------------------------------------------------------------------
  Callback* Token::GetCallback() const
  {
    return m_pCallback.get();
  }
} // namespace mec


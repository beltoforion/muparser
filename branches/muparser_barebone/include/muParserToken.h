/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
  Copyright (C) 2004-2013 Ingo Berg

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

#ifndef MU_PARSER_TOKEN_H
#define MU_PARSER_TOKEN_H

#include <cassert>
#include <string>
#include <stack>
#include <vector>
#include <memory>

#include "muParserError.h"
#include "muParserCallback.h"

/** \file
    \brief This file contains the parser token definition.
*/

namespace mu
{
  /** \brief Encapsulation of the data for a single formula token. 

    Formula token implementation. Part of the Math Parser Package.
    Formula tokens can be either one of the following:
    <ul>
      <li>value</li>
      <li>variable</li>
      <li>function with numerical arguments</li>
      <li>functions with a string as argument</li>
      <li>prefix operators</li>
      <li>infix operators</li>
	    <li>binary operator</li>
    </ul>

   \author (C) 2004-2013 Ingo Berg 
  */
  template<typename TBase, typename TString>
  class ParserToken
  {
  private:

      ECmdCode  m_iCode;  ///< Type of the token; The token type is a constant of type #ECmdCode.
      void  *m_pTok;      ///< Stores Token pointer; not applicable for all tokens
      TString m_strTok;   ///< Token string
      value_type m_fVal;  ///< the value 
      std::auto_ptr<ParserCallback> m_pCallback;

  public:

      //---------------------------------------------------------------------------
      /** \brief Constructor (default).
        
          Sets token to an neutral state of type cmUNKNOWN.
          \throw nothrow
          \sa ECmdCode
      */
      ParserToken()
        :m_iCode(cmUNKNOWN)
        ,m_pTok(0)
        ,m_strTok()
        ,m_pCallback()
      {}

      //------------------------------------------------------------------------------
      /** \brief Create token from another one.
      
          Implemented by calling Assign(...)
          \throw nothrow
          \post m_iType==cmUNKNOWN
          \sa #Assign
      */
      ParserToken(const ParserToken &a_Tok)
      {
        Assign(a_Tok);
      }
      
      //------------------------------------------------------------------------------
      /** \brief Assignement operator. 
      
          Copy token state from another token and return this.
          Implemented by calling Assign(...).
          \throw nothrow
      */
      ParserToken& operator=(const ParserToken &a_Tok)
      {
        Assign(a_Tok);
        return *this;
      }

      //------------------------------------------------------------------------------
      /** \brief Copy token information from argument.
      
          \throw nothrow
      */
      void Assign(const ParserToken &a_Tok)
      {
        m_iCode = a_Tok.m_iCode;
        m_pTok = a_Tok.m_pTok;
        m_strTok = a_Tok.m_strTok;
        m_fVal = a_Tok.m_fVal;
        // create new callback object if a_Tok has one 
        m_pCallback.reset(a_Tok.m_pCallback.get() ? a_Tok.m_pCallback->Clone() : 0);
      }

      //------------------------------------------------------------------------------
      /** \brief Assign a token type. 

        Token may not be of type value, variable or function. Those have seperate set functions. 

        \pre [assert] a_iType!=cmVAR
        \pre [assert] a_iType!=cmVAL
        \pre [assert] a_iType!=cmFUNC
        \post m_fVal = 0
        \post m_pTok = 0
      */
      ParserToken& Set(ECmdCode a_iType, const TString &a_strTok=TString())
      {
        // The following types cant be set this way, they have special Set functions
        assert(a_iType!=cmVAR);
        assert(a_iType!=cmVAL);
        assert(a_iType!=cmFUNC);

        m_iCode = a_iType;
        m_pTok = 0;
        m_strTok = a_strTok;

        return *this;
      }

      //------------------------------------------------------------------------------
      /** \brief Set Callback type. */
      ParserToken& Set(const ParserCallback &a_pCallback, const TString &a_sTok)
      {
        assert(a_pCallback.GetAddr());

        m_iCode = a_pCallback.GetCode();
        m_strTok = a_sTok;
        m_pCallback.reset(new ParserCallback(a_pCallback));
        m_pTok = 0;
        
        return *this;
      }

      //------------------------------------------------------------------------------
      /** \brief Make this token a value token. 
      
          Member variables not necessary for value tokens will be invalidated.
          \throw nothrow
      */
      ParserToken& SetVal(TBase a_fVal, const TString &a_strTok=TString())
      {
        m_iCode = cmVAL;
        m_fVal = a_fVal;
        m_strTok = a_strTok;
        m_pTok = 0;
        m_pCallback.reset(0);
        return *this;
      }

      //------------------------------------------------------------------------------
      /** \brief make this token a variable token. 
      
          Member variables not necessary for variable tokens will be invalidated.
          \throw nothrow
      */
      ParserToken& SetVar(TBase *a_pVar, const TString &a_strTok)
      {
        m_iCode = cmVAR;
        m_strTok = a_strTok;
        m_pTok = (void*)a_pVar;
        m_pCallback.reset(0);
        return *this;
      }

      //------------------------------------------------------------------------------
      /** \brief Return the token type.
      
          \return #m_iType
          \throw nothrow
      */
      ECmdCode GetCode() const
      {
        if (m_pCallback.get())
        {
          return m_pCallback->GetCode();
        }
        else
        {
          return m_iCode;
        }
      }

      //------------------------------------------------------------------------------
      int GetPri() const
      {
        if ( !m_pCallback.get())
	        throw ParserError(ecINTERNAL_ERROR);
            
        if (m_pCallback->GetCode()!=cmOPRT_INFIX)
	        throw ParserError(ecINTERNAL_ERROR);

        return m_pCallback->GetPri();
      }

      //------------------------------------------------------------------------------
      EOprtAssociativity GetAssociativity() const
      {
        if (m_pCallback.get()==nullptr)
	        throw ParserError(ecINTERNAL_ERROR);

        return m_pCallback->GetAssociativity();
      }

      //------------------------------------------------------------------------------
      /** \brief Return the address of the callback function assoziated with
                 function and operator tokens.

          \return The pointer stored in #m_pTok.
          \throw exception_type if token type is non of:
                 <ul>
                   <li>cmFUNC</li>
                   <li>cmINFIXOP</li>
                   <li>cmOPRT_BIN</li>
                 </ul>
          \sa ECmdCode
      */
      generic_fun_type GetFuncAddr() const
      {
        return (m_pCallback.get()) ? (generic_fun_type)m_pCallback->GetAddr() : 0;
      }

      //------------------------------------------------------------------------------
      /** \biref Get value of the token.
        
          Only applicable to variable and value tokens.
          \throw exception_type if token is no value/variable token.
      */
      TBase GetVal() const
      {
        switch (m_iCode)
        {
          case cmVAL:  return m_fVal;
          case cmVAR:  return *((TBase*)m_pTok);
          default:     throw ParserError(ecINTERNAL_ERROR);
        }
      }

      //------------------------------------------------------------------------------
      /** \brief Get address of a variable token.

        Valid only if m_iType==CmdVar.
        \throw exception_type if token is no variable token.
      */
      TBase* GetVar() const
      {
        if (m_iCode!=cmVAR)
	        throw ParserError(ecINTERNAL_ERROR);

        return (TBase*)m_pTok;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Return the number of function arguments. 

        Valid only if m_iType==CmdFUNC.
      */
      int GetArgCount() const
      {
        assert(m_pCallback.get());

        if (!m_pCallback->GetAddr())
	        throw ParserError(ecINTERNAL_ERROR);

        return m_pCallback->GetArgc();
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Return the token identifier. 
          \return #m_strTok
          \throw nothrow
          \sa m_strTok
      */
      const TString& GetAsString() const
      {
        return m_strTok;
      }
  };
} // namespace mu

#endif

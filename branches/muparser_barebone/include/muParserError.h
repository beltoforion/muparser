/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
  Copyright (C) 2004-2011 Ingo Berg

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
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

#include "muParserDef.h"

/** \file 
    \brief This file defines the error class used by the parser.
*/

namespace mu
{

/** \brief Error codes. */
enum EErrorCodes
{
  // Formula syntax errors
  ecUNEXPECTED_OPERATOR    = 0,  ///< Unexpected binary operator found
  ecUNASSIGNABLE_TOKEN     = 1,  ///< Token cant be identified.
  ecUNEXPECTED_EOF         = 2,  ///< Unexpected end of formula. (Example: "2+sin(")
  ecUNEXPECTED_ARG_SEP     = 3,  ///< An unexpected comma has been found. (Example: "1,23")
  ecUNEXPECTED_ARG         = 4,  ///< An unexpected argument has been found
  ecUNEXPECTED_VAL         = 5,  ///< An unexpected value token has been found
  ecUNEXPECTED_VAR         = 6,  ///< An unexpected variable token has been found
  ecUNEXPECTED_PARENS      = 7,  ///< Unexpected Parenthesis, opening or closing
  ecMISSING_PARENS         = 8, ///< Missing parens. (Example: "3*sin(3")
  ecUNEXPECTED_FUN         = 9, ///< Unexpected function found. (Example: "sin(8)cos(9)")
  ecTOO_MANY_PARAMS        = 10, ///< Too many function parameters
  ecTOO_FEW_PARAMS         = 11, ///< Too few function parameters. (Example: "ite(1<2,2)")

  // Invalid Parser input Parameters
  ecINVALID_NAME           = 12, ///< Invalid function, variable or constant name.
  ecINVALID_INFIX_IDENT    = 13, ///< Invalid function, variable or constant name.

  ecINVALID_FUN_PTR        = 14, ///< Invalid callback function pointer 
  ecINVALID_VAR_PTR        = 15, ///< Invalid variable pointer 
  ecEMPTY_EXPRESSION       = 16, ///< The Expression is empty
  ecNAME_CONFLICT          = 17, ///< Name conflict
  ecOPT_PRI                = 18, ///< Invalid operator priority
  // 
  ecDOMAIN_ERROR           = 19, ///< catch division by zero, sqrt(-1), log(0) (currently unused)
  ecDIV_BY_ZERO            = 20, ///< Division by zero (currently unused)
  ecGENERIC                = 21, ///< Generic error
  ecLOCALE                 = 22, ///< Conflict with current locale

  // internal errors
  ecINTERNAL_ERROR         = 23, ///< Internal error of any kind.
  
  // The last two are special entries 
  ecCOUNT,                      ///< This is no error code, It just stores just the total number of error codes
  ecUNDEFINED              = -1  ///< Undefined message, placeholder to detect unassigned error messages
};

//---------------------------------------------------------------------------
/** \brief A class that handles the error messages.
*/
class ParserErrorMsg
{
public:
    typedef ParserErrorMsg self_type;

    ParserErrorMsg& operator=(const ParserErrorMsg &);
    ParserErrorMsg(const ParserErrorMsg&);
    ParserErrorMsg();

   ~ParserErrorMsg();

    static const ParserErrorMsg& Instance();
    string_type operator[](unsigned a_iIdx) const;

private:
    std::vector<string_type>  m_vErrMsg;  ///< A vector with the predefined error messages
    static const self_type m_Instance;    ///< The instance pointer
};

//---------------------------------------------------------------------------
/** \brief Error class of the parser. 
    \author Ingo Berg

  Part of the math parser package.
*/
class ParserError
{
private:

    /** \brief Replace all ocuurences of a substring with another string. */
    void ReplaceSubString( string_type &strSource, 
                           const string_type &strFind,
                           const string_type &strReplaceWith);
    void Reset();

public:

    ParserError();
    explicit ParserError(EErrorCodes a_iErrc);
    explicit ParserError(const string_type &sMsg);
    ParserError( EErrorCodes a_iErrc,
                 const string_type &sTok,
                 const string_type &sFormula = string_type(),
                 int a_iPos = -1);
    ParserError( EErrorCodes a_iErrc, 
                 int a_iPos, 
                 const string_type &sTok);
    ParserError( const char_type *a_szMsg, 
                 int a_iPos = -1, 
                 const string_type &sTok = string_type());
    ParserError(const ParserError &a_Obj);
    ParserError& operator=(const ParserError &a_Obj);
   ~ParserError();

    void SetFormula(const string_type &a_strFormula);
    const string_type& GetExpr() const;
    const string_type& GetMsg() const;
    std::size_t GetPos() const;
    const string_type& GetToken() const;
    EErrorCodes GetCode() const;

private:
    string_type m_strMsg;     ///< The message string
    string_type m_sExpr; ///< Formula string
    string_type m_strTok;     ///< Token related with the error
    int m_iPos;               ///< Formula position related to the error 
    EErrorCodes m_iErrc;      ///< Error code
    const ParserErrorMsg &m_ErrMsg;
};		

} // namespace mu

#endif


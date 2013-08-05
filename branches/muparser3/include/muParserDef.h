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
#ifndef MUP_DEF_H
#define MUP_DEF_H

//--- Standard includes ---------------------------------------------------------------------------
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <map>

//-------------------------------------------------------------------------------------------------
#if defined min
  #undef min
#endif

#if defined max
  #undef max
#endif

//-------------------------------------------------------------------------------------------------
#define MUP_VERSION      _SL("0.3.4")
#define MUP_VERSION_DATE _SL("20120714; SF-SVN/BRANCHES")

#define MUP_NAMESPACE_START namespace mp {
#define MUP_NAMESPACE_END }

#if defined(_MSC_VER)
  #define MUP_FASTCALL __fastcall
  #define MUP_INLINE __forceinline
#else
  #define MUP_FASTCALL
  #define MUP_INLINE 
#endif

#if defined(_DEBUG)
  #define MUP_FAIL(MSG)     \
          {                 \
            bool MSG=false; \
            assert(MSG);    \
          }
/*
  #define MUP_ASSERT(COND)                          \
          if (!(COND))                              \
          {                                         \
            stringstream_type ss;                   \
            ss << _SL("Assertion \"")               \
               << _SL(#COND)                        \
               << _SL("\" failed: ")                \
               << __FILE__ << _SL(" line ")         \
               << __LINE__ << _SL(".");             \
            throw ParserError<TString>( ss.str() ); \
          }
*/
#else
  #define MUP_FAIL(MSG)
//  #define MUP_ASSERT(COND)
#endif

#define MUP_ASSERT(COND)                          \
        if (!(COND))                              \
        {                                         \
          stringstream_type ss;                   \
          ss << _SL("Assertion \"")               \
             << _SL(#COND)                        \
             << _SL("\" failed: ")                \
             << __FILE__ << _SL(" line ")         \
             << __LINE__ << _SL(".");             \
          throw ParserError<TString>( ss.str() ); \
        }

#define _SL(x) details::string_traits<typename TString::value_type>::select(x, L##x)
#define _OUT   details::string_traits<typename TString::value_type>::out()


MUP_NAMESPACE_START

  //------------------------------------------------------------------------------
  /** \brief Code used to distinguish different hardcoded parsing engines.
  */
  enum EEngineCode
  {
    // V - Value entry   : 1 
    // F - Function call : 0
    //     Integer     Binary 
    ecV     =  1,      //       1
    ecVF    =  2,      //      10

    ecVFF   =  4,      //     100
    ecVVF   =  6,      //     110

    ecVFFF  =  8,      //    1000
    ecVFVF  = 10,      //    1010  
    ecVVFF  = 12,      //    1100
    ecVVVF  = 14,      //    1110

    ecVFFFF = 16,      //   10000
    ecVFFVF = 18,      //   10010
    ecVFVFF = 20,      //   10100   
    ecVFVVF = 22,      //   10110   
    ecVVFFF = 24,      //   11000
    ecVVFVF = 26,      //   11010
    ecVVVFF = 28,      //   11100
    ecVVVVF = 30,      //   11110

    ecUNOPTIMIZABLE = 32,
    ecNO_MUL = 64      // If this flag is set the expression does not have any variable multiplier in VAL_EX tokens.

/*
    ecVFFFFF = 32,     //  100000
    ecVFFFVF = 34,     //  100010
    ecVFFVFF = 36,     //  100100   
    ecVFFVVF = 38,     //  100110
    ecVFVFFF = 40,     //  101000
    ecVFVFVF = 42,     //  101010
    ecVFVVFF = 44,     //  101100

    ecVVFFFF = 48,     //  110000
    ecVVFFVF = 50,     //  100010
    ecVVFVFF = 52,     //  100100   
    ecVVFVVF = 54,     //  100110
    ecVVVFFF = 56,     //  101000
    ecVVVFVF = 58,     //  101010
    ecVVVVFF = 60,     //  101100
    ecVVVVVF = 62,     //  101100

    ecUNOPTIMIZABLE = 64
*/
  };

  //------------------------------------------------------------------------------
  /** \brief Code for expression tokens.
  */
  enum ECmdCode
  {
    cmASSIGN        = 0,
    cmBO            = 1,
    cmBC            = 2,
    cmIF            = 3,
    cmELSE          = 4,
    cmENDIF         = 5,
    cmARG_SEP       = 6,
    cmVAL_EX        = 7,
    cmVAR           = 8,
    cmVAL           = 9,
    cmFUNC          = 10,
    cmOPRT_BIN,
    cmOPRT_POSTFIX,
    cmOPRT_INFIX,
    cmEND
  };

  //------------------------------------------------------------------------------
  enum EParserVersionInfo
  {
    pviBRIEF,
    pviFULL
  };

  //------------------------------------------------------------------------------
  enum EOprtAssociativity
  {
    oaLEFT  = 0,
    oaRIGHT = 1,
    oaNONE  = 2
  };

  //------------------------------------------------------------------------------
  enum EOprtPrecedence
  {
    prLOR     = 1,
    prLAND    = 2,
    prLOGIC   = 3,
    prCMP     = 4,
    prADD_SUB = 5,
    prMUL_DIV = 6,
    prPOW     = 7,
    prINFIX   = 6,
    prPOSTFIX = 6 
  };

  //------------------------------------------------------------------------------
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
    ecVAL_EXPECTED           = 8,  ///< A numerical function has been called with a non value type of argument
    ecMISSING_PARENS         = 9,  ///< Missing parens. (Example: "3*sin(3")
    ecUNEXPECTED_FUN         = 10, ///< Unexpected function found. (Example: "sin(8)cos(9)")
    ecTOO_MANY_PARAMS        = 11, ///< Too many function parameters
    ecTOO_FEW_PARAMS         = 12, ///< Too few function parameters. (Example: "ite(1<2,2)")

    // Invalid Parser input Parameters
    ecINVALID_NAME           = 13, ///< Invalid function, variable or constant name.
    ecINVALID_BINOP_IDENT    = 14, ///< Invalid binary operator identifier
    ecINVALID_INFIX_IDENT    = 15, ///< Invalid function, variable or constant name.
    ecINVALID_POSTFIX_IDENT  = 16, ///< Invalid function, variable or constant name.

    ecBUILTIN_OVERLOAD       = 17, ///< Trying to overload builtin operator
    ecINVALID_FUN_PTR        = 18, ///< Invalid callback function pointer 
    ecINVALID_VAR_PTR        = 19, ///< Invalid variable pointer 
    ecEMPTY_EXPRESSION       = 20, ///< The Expression is empty
    ecNAME_CONFLICT          = 21, ///< Name conflict
    ecOPT_PRI                = 22, ///< Invalid operator priority
    // 
    ecDOMAIN_ERROR           = 23, ///< catch division by zero, sqrt(-1), log(0) (currently unused)
    ecDIV_BY_ZERO            = 24, ///< Division by zero (currently unused)
    ecGENERIC                = 25, ///< Generic error
    ecLOCALE                 = 26, ///< Conflict with current locale

    ecUNEXPECTED_CONDITIONAL = 27,
    ecMISSING_ELSE_CLAUSE    = 28, 
    ecMISPLACED_COLON        = 29,

    // internal errors
    ecINTERNAL_ERROR         = 30, ///< Internal error of any kind.
  
    // The last two are special entries 
    ecCOUNT,                       ///< This is no error code, It just stores just the total number of error codes
    ecUNDEFINED              = -1  ///< Undefined message, placeholder to detect unassigned error messages
  };

  //------------------------------------------------------------------------------
  // Forward declarations
  template<typename TVal, typename TStr> 
  class Token;

  template<typename TValue, typename TString>
  class ParserBase;

  //------------------------------------------------------------------------------
  // basic types
  template<typename TVal, typename TString>
  struct parser_types
  {
    typedef TVal value_type;
    typedef void (MUP_FASTCALL *fun_type)(TVal*, int narg);
    typedef int (*identfun_type)(const typename TString::value_type *sExpr, int *nPos, TVal *fVal);
    typedef TVal* (*facfun_type)(const typename TString::value_type*, void*);
    typedef Token<TVal, TString> token_type;
  };


  namespace details
  {
    //---------------------------------------------------------------------------------------------
    template<typename T>
    struct string_traits
    {
      static const char* select(const char* sel, const wchar_t*)  { return sel;  }
      static char select(const char sel, const wchar_t) { return sel;  }
      static std::ostream& out() { return std::cout; }
      static std::istream& in()  { return std::cin; }
    };

    //---------------------------------------------------------------------------------------------
    template<>
    struct string_traits<wchar_t>
    {
      static const wchar_t* select(const char*, const wchar_t *sel) { return sel; }
      static wchar_t select(const char, const wchar_t sel) { return sel; }
      static std::wostream& out() { return std::wcout; }
      static std::wistream& in()  { return std::wcin; }
    };

    /** \brief A class singling out integer types at compile time using 
               template meta programming.
    */
    template<typename T>
    struct value_traits
    {
      static bool IsInteger() { return false; }
    };

    template<>
    struct value_traits<char>
    {
      static bool IsInteger() { return true;  }
    };

    template<>
    struct value_traits<short>
    {
      static bool IsInteger() { return true;  }
    };

    template<>
    struct value_traits<int>
    {
      static bool IsInteger() { return true;  }
    };

    template<>
    struct value_traits<long>
    {
      static bool IsInteger() { return true;  }
    };

    template<>
    struct value_traits<unsigned char>
    {
      static bool IsInteger() { return true;  }
    };

    template<>
    struct value_traits<unsigned short>
    {
      static bool IsInteger() { return true;  }
    };

    template<>
    struct value_traits<unsigned int>
    {
      static bool IsInteger() { return true;  }
    };

    template<>
    struct value_traits<unsigned long>
    {
      static bool IsInteger() { return true;  }
    };
  } // namespace mp::details
} // namespace mp

#endif


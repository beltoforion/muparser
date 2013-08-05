/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
  Copyright (C) 2013 Ingo Berg

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

#include <iostream>
#include <string>
#include <sstream>
#include <map>

/** \file
    \brief This file contains standard definitions used by the parser.
*/

#define MUP_VERSION _T("0.0.0")
#define MUP_VERSION_DATE _T("20130402; SF")

#define MUP_CHARS _T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")

/** \brief Define the base datatype for values.

  This datatype must be a built in value type. You can not use custom classes.
  It should be working with all types except "int"!
*/
#define MUP_BASETYPE double

/** \brief Activate this option in order to compile with OpenMP support. 

  OpenMP is used only in the bulk mode it may increase the performance a bit. 
*/
//#define MUP_USE_OPENMP

#if defined(_UNICODE)
  /** \brief Definition of the basic parser string type. */
  #define MUP_STRING_TYPE std::wstring

  #if !defined(_T)
    #define _T(x) L##x
  #endif // not defined _T
#else
  #ifndef _T
  #define _T(x) x
  #endif
  
  /** \brief Definition of the basic parser string type. */
  #define MUP_STRING_TYPE std::string
#endif

#if defined(_DEBUG)
    /** \brief An assertion that does not kill the program.

        This macro is neutralised in UNICODE builds. It's
        too difficult to translate.
    */
    #define MUP_ASSERT(COND)                         \
            if (!(COND))                             \
            {                                        \
              stringstream_type ss;                  \
              ss << _T("Assertion \"") _T(#COND) _T("\" failed: ") \
                 << __FILE__ << _T(" line ")         \
                 << __LINE__ << _T(".");             \
              throw ParserError( ss.str() );         \
            }
#else
  #define MUP_ASSERT(COND)
#endif


namespace mu
{
#if defined(_UNICODE)

  //------------------------------------------------------------------------------
  /** \brief Encapsulate wcout. */
  inline std::wostream& console()
  {
    return std::wcout;
  }

  /** \brief Encapsulate cin. */
  inline std::wistream& console_in()
  {
    return std::wcin;
  }

#else

  /** \brief Encapsulate cout. 
  
    Used for supporting UNICODE more easily.
  */
  inline std::ostream& console()
  {
    return std::cout;
  }

  /** \brief Encapsulate cin. 

    Used for supporting UNICODE more easily.
  */
  inline std::istream& console_in()
  {
    return std::cin;
  }

#endif

  //------------------------------------------------------------------------------
  /** \brief Bytecode values.

      \attention The order of the operator entries must match the order in ParserBase::c_DefaultOprt!
  */
  enum ECmdCode
  {
    // The following are codes for built in binary operators
    // apart from built in operators the user has the opportunity to
    // add user defined operators.
    cmLE            = 0,   ///< Operator item:  less or equal
    cmGE            = 1,   ///< Operator item:  greater or equal
    cmNEQ           = 2,   ///< Operator item:  not equal
    cmEQ            = 3,   ///< Operator item:  equals
    cmLT            = 4,   ///< Operator item:  less than
    cmGT            = 5,   ///< Operator item:  greater than
    cmADD           = 6,   ///< Operator item:  add
    cmSUB           = 7,   ///< Operator item:  subtract
    cmMUL           = 8,   ///< Operator item:  multiply
    cmDIV           = 9,   ///< Operator item:  division
    cmPOW           = 10,  ///< Operator item:  y to the power of ...
    cmLAND          = 11,
    cmLOR           = 12,
    cmBO            = 13,  ///< Operator item:  opening bracket
    cmBC            = 14,  ///< Operator item:  closing bracket
    cmARG_SEP       = 15,  ///< function argument separator
    cmVAR           = 16,  ///< variable item
    cmVAL           = 17,  ///< value item

    // operators and functions
    cmFUNC,                ///< Code for a generic function item
    cmFUNC_BULK,           ///< Special callbacks for Bulk mode with an additional parameter for the bulk index 
    cmOPRT_INFIX,          ///< code for infix operators
    cmEND,                 ///< end of formula
    cmUNKNOWN              ///< uninitialized item
  };

  //------------------------------------------------------------------------------
  enum EParserVersionInfo
  {
    pviBRIEF,
    pviFULL
  };

  //------------------------------------------------------------------------------
  /** \brief Parser operator precedence values. */
  enum EOprtAssociativity
  {
    oaLEFT  = 0,
    oaRIGHT = 1,
    oaNONE  = 2
  };

  //------------------------------------------------------------------------------
  /** \brief Parser operator precedence values. */
  enum EOprtPrecedence
  {
    // binary operators
    prLOR     = 1,
    prLAND    = 2,
    prLOGIC   = 3,  ///< logic operators
    prCMP     = 4,  ///< comparsion operators
    prADD_SUB = 5,  ///< addition
    prMUL_DIV = 6,  ///< multiplication/division
    prPOW     = 7,  ///< power operator priority (highest)

    // infix operators
    prINFIX   = 6, ///< Signs have a higher priority than ADD_SUB, but lower than power operator
    prPOSTFIX = 6  ///< Postfix operator priority (currently unused)
  };

  //------------------------------------------------------------------------------
  // basic types

  /** \brief The numeric datatype used by the parser. 
  
    Normally this is a floating point type either single or double precision.
  */
  typedef MUP_BASETYPE value_type;

  /** \brief The stringtype used by the parser. 

    Depends on wether UNICODE is used or not.
  */
  typedef MUP_STRING_TYPE string_type;

  /** \brief The character type used by the parser. 
  
    Depends on wether UNICODE is used or not.
  */
  typedef string_type::value_type char_type;

  /** \brief Typedef for easily using stringstream that respect the parser stringtype. */
  typedef std::basic_stringstream<char_type,
                                  std::char_traits<char_type>,
                                  std::allocator<char_type> > stringstream_type;

  // Data container types

  /** \brief Type used for storing variables. */
  typedef std::map<string_type, value_type*> varmap_type;
  
  /** \brief Type used for storing constants. */
  typedef std::map<string_type, value_type> valmap_type;
  
  // Parser callbacks
  
  /** \brief Callback type used for functions without arguments. */
  typedef value_type (*generic_fun_type)();

  /** \brief Callback type used for functions without arguments. */
  typedef value_type (*fun_type0)();

  /** \brief Callback type used for functions with a single arguments. */
  typedef value_type (*fun_type1)(value_type);

  /** \brief Callback type used for functions with two arguments. */
  typedef value_type (*fun_type2)(value_type, value_type);

  /** \brief Callback type used for functions with three arguments. */
  typedef value_type (*fun_type3)(value_type, value_type, value_type);

  /** \brief Callback type used for functions without arguments. */
  typedef value_type (*bulkfun_type0)(int, int);

  /** \brief Callback type used for functions with a single arguments. */
  typedef value_type (*bulkfun_type1)(int, int, value_type);

  /** \brief Callback type used for functions with two arguments. */
  typedef value_type (*bulkfun_type2)(int, int, value_type, value_type);

  /** \brief Callback type used for functions with three arguments. */
  typedef value_type (*bulkfun_type3)(int, int, value_type, value_type, value_type);

  /** \brief Callback used for functions that identify values in a string. */
  typedef int (*identfun_type)(const char_type *sExpr, int *nPos, value_type *fVal);
} // end of namespace

#endif


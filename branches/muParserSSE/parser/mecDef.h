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
#ifndef MEC_DEF_H
#define MEC_DEF_H

#include <iostream>
#include <string>
#include <sstream>
#include <map>

/** \file
    \brief This file contains standard definitions used by the parser.
*/
#define MEC_VERSION _T("1.0.5 (20130714)")

/** \brief Characters for use in unary and binary operators. */
#define MEC_OPRT_CHARS _T("+-*^/?<>=#!$%&|~'_")

#define MEC_CHARS _T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")

/** \brief Define the base datatype for values.

  This datatype must be a built in value type. You can not use custom classes.
  It has been tested with float, double and long double types, int should
  work as well.
*/
#define MEC_BASETYPE float

/** \brief Definition of the basic bytecode datatype.

  This defines the smalles entity used in the bytecode.
*/
#define MEC_BYTECODE_TYPE long

/** \brief Definition to ignore the inline assembly engine.

  This engine has no purpose except to help me in 
  creating the assembly code for asmjit. Since it will not
  compile on GCC there has to be a way to ignore it 
  entirely.
*/
//#define NO_MICROSOFT_STYLE_INLINE_ASSEMBLY

#if defined(_UNICODE)
  /** \brief Definition of the basic parser string type. */
  #define MEC_STRING_TYPE std::wstring

  #if !defined(_T)
    #define _T(x) L##x
  #endif // not defined _T
#else
  #ifndef _T
  #define _T(x) x
  #endif
  
  /** \brief Definition of the basic parser string type. */
  #define MEC_STRING_TYPE std::string
#endif

#if defined(_DEBUG)
    /** \brief An assertion that does not kill the program.

        This macro is neutralised in UNICODE builds. It's
        too difficult to translate.
    */
    #define MEC_ASSERT(COND)                      \
            if (!(COND))                             \
            {                                        \
              stringstream_type ss;                  \
              ss << _T("Assertion \"") _T(#COND) _T("\" failed: ") \
                 << __FILE__ << _T(" line ")         \
                 << __LINE__ << _T(".");             \
              throw ParserError( ss.str() );         \
            }
#else
  #define MEC_ASSERT(COND)
#endif

//------------------------------------------------------------------------------
//
// do not change anything beyond this point...
//
// !!! This section is devoted to macros that are used for debugging
// !!! or for features that are not fully implemented yet.
//
//#define MEC_SAVE_ASM
//#define MEC_DUMP_STACK
//#define MEC_DUMP_CMDCODE
//#define MEC_INTRINSIC_POW

namespace mec
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

  extern bool g_DbgDumpCmdCode;
  extern bool g_DbgDumpStack;

  //------------------------------------------------------------------------------
  /** \brief Bytecode values.

      \attention The order of the operator entries must match the order in ParserBase::c_DefaultOprt!
  */
  enum ECmdCode
  {
    // The following are codes for built in binary operators
    // apart from built in operators the user has the opportunity to
    // add user defined operators.

    // intrinsic binary operators
    cmMIN     = 0,           ///< Operator item:  Minimum of two values
    cmMAX     = 1,           ///< Operator item:  Maximum of two values
    cmLE      = 2,           ///< Operator item:  less or equal
    cmGE      = 3,           ///< Operator item:  greater or equal
    cmNEQ     = 4,           ///< Operator item:  not equal
    cmEQ      = 5,           ///< Operator item:  equals
    cmLT      = 6,           ///< Operator item:  less than
    cmGT      = 7,           ///< Operator item:  greater than
    cmAND     = 8,           ///< Operator item:  logical and
    cmOR      = 9,           ///< Operator item:  logical or
    cmADD     = 10,          ///< Operator item:  add
    cmSUB     = 11,          ///< Operator item:  subtract
    cmMUL     = 12,          ///< Operator item:  multiply
    cmDIV     = 13,          ///< Operator item:  division
//    cmMOD     = 14,

    // intrinsic functions implemented by using the FPU
    cmSIN     = 14,          ///< function: sine must be the first fucntion!
    cmCOS     = 15,
    cmTAN     = 16,

    // intrinsic functions implemented by using the SSE
    cmABS     = 17,
    cmSQRT    = 18,          ///< function: sqrt MUST be the last function
 
    cmBO      = 19,          ///< Operator item:  opening bracket
    cmBC      = 20,          ///< Operator item:  closing bracket
    cmIF      = 21,          ///< For use in the ternary if-then-else operator
    cmELSE    = 22,          ///< For use in the ternary if-then-else operator
    cmENDIF   = 23,          ///< For use in the ternary if-then-else operator
    cmARG_SEP = 24,          ///< function argument separator
    cmVAR     = 25,          ///< variable item
    cmVAL     = 26,          ///< value item
    cmFUNC,                  ///< Code for a function item
    cmOPRT_BIN,              ///< user defined binary operator
    cmOPRT_POSTFIX,          ///< code for postfix operators
    cmOPRT_INFIX,            ///< code for infix operators
    cmEND,                   ///< end of expression
    cmUNKNOWN,               ///< uninitialized item
  };

  // SSE instruction set
  //Arithmetic:
  //addps - Adds 4 single-precision (32bit) floating-point values to 4 other single-precision floating-point values.
  //addss - Adds the lowest single-precision values, top 3 remain unchanged.
  //subps - Subtracts 4 single-precision floating-point values from 4 other single-precision floating-point values.
  //subss - Subtracts the lowest single-precision values, top 3 remain unchanged.
  //mulps - Multiplies 4 single-precision floating-point values with 4 other single-precision values.
  //mulss - Multiplies the lowest single-precision values, top 3 remain unchanged.
  //divps - Divides 4 single-precision floating-point values by 4 other single-precision floating-point values.
  //divss - Divides the lowest single-precision values, top 3 remain unchanged.
  //rcpps - Reciprocates (1/x) 4 single-precision floating-point values.
  //rcpss - Reciprocates the lowest single-precision values, top 3 remain unchanged.
  //sqrtps - Square root of 4 single-precision values.
  //sqrtss - Square root of lowest value, top 3 remain unchanged.
  //rsqrtps - Reciprocal square root of 4 single-precision floating-point values.
  //rsqrtss - Reciprocal square root of lowest single-precision value, top 3 remain unchanged.
  //maxps - Returns maximum of 2 values in each of 4 single-precision values.
  //maxss - Returns maximum of 2 values in the lowest single-precision value. Top 3 remain unchanged.
  //minps - Returns minimum of 2 values in each of 4 single-precision values.
  //minss - Returns minimum of 2 values in the lowest single-precision value, top 3 remain unchanged.
  //pavgb - Returns average of 2 values in each of 8 bytes.
  //pavgw - Returns average of 2 values in each of 4 words.
  //psadbw - Returns sum of absolute differences of 8 8bit values. Result in bottom 16 bits.
  //pextrw - Extracts 1 of 4 words.
  //pinsrw - Inserts 1 of 4 words.
  //pmaxsw - Returns maximum of 2 values in each of 4 signed word values.
  //pmaxub - Returns maximum of 2 values in each of 8 unsigned byte values.
  //pminsw - Returns minimum of 2 values in each of 4 signed word values.
  //pminub - Returns minimum of 2 values in each of 8 unsigned byte values.
  //pmovmskb - builds mask byte from top bit of 8 byte values.
  //pmulhuw - Multiplies 4 unsigned word values and stores the high 16bit result.
  //pshufw - Shuffles 4 word values. Takes 2 128bit values (source and dest) and an 8-bit immediate value, and then fills in each Dest 32-bit value from a Source 32-bit value specified by the immediate. The immediate byte is broken into 4 2-bit values.
  //
  //Logic:
  //andnps - Logically ANDs 4 single-precision values with the logical inverse (NOT) of 4 other single-precision values.
  //andps - Logically ANDs 4 single-precision values with 4 other single-precision values.
  //orps - Logically ORs 4 single-precision values with 4 other single-precision values.
  //xorps - Logically XORs 4 single-precision values with 4 other single-precision values.

  //------------------------------------------------------------------------------
  /** \brief Parser operator precedence values. */
  enum EOprtAssociativity
  {
    oaLEFT,
    oaRIGHT,
    oaNONE
  };

  //------------------------------------------------------------------------------
  /** \brief Parser operator precedence values. */
  enum EOprtPrecedence
  {
    // binary operators
    prLOGIC   = 1,  ///< logic operators
    prCMP     = 2,  ///< comparsion operators
    prADD_SUB = 3,  ///< addition
    prMUL_DIV = 4,  ///< multiplication/division
    prPOW     = 5,  ///< power operator priority (highest)

    // infix operators
    prINFIX    = 4, ///< Signs have a higher priority than ADD_SUB, but lower than power operator
    prPOSTFIX  = 4  ///< Postfix operator priority (currently unused)
  };

  //------------------------------------------------------------------------------
  /** \brief An enumeration to distinguish different implementations 
             of the parser engine. 
  */
  enum EParserEngine
  {
    peSTRING,         ///< Parse only from string
    peBYTECODE,       ///< Parse expression from bytecode when doing successive evaluations
    peJIT,
#if !defined(NO_MICROSOFT_STYLE_INLINE_ASSEMBLY)
    peBYTECODE_ASM,    ///< Parse expression from bytecode using an engine written in inline assembly
#endif
  };

  //------------------------------------------------------------------------------
  /** \brief Additional token flags. */
  enum ETokenFlags
  {
    flVOLATILE           = 1 << 0, ///< Mark a token that depends on a variable or a function that is not conservative
  };

  //------------------------------------------------------------------------------
  // basic types

  /** \brief The numeric datatype used by the parser. 
  
    Normally this is a floating point type either single or double precision.
  */
  typedef MEC_BASETYPE value_type;

  /** \brief The stringtype used by the parser. 

    Depends on wether UNICODE is used or not.
  */
  typedef MEC_STRING_TYPE string_type;

  /** \brief The bytecode type used by the parser. 
  
    The bytecode type depends on the value_type.
  */
  typedef MEC_BYTECODE_TYPE bytecode_type;

  typedef int index_type;

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
  
  /** \brief Type for assigning a string name to an index in the internal string table. */
  typedef std::map<string_type, std::size_t> strmap_type;

  // Parser callbacks
  
  /** \brief Callback type used for functions without arguments. */
  typedef value_type (*fun_type0)();

  /** \brief Callback type used for functions with a single arguments. */
  typedef value_type (*fun_type1)(value_type);

  /** \brief Callback type used for functions with two arguments. */
  typedef value_type (*fun_type2)(value_type, 
                                  value_type);

  /** \brief Callback type used for functions with three arguments. */
  typedef value_type (*fun_type3)(value_type, 
                                  value_type, 
                                  value_type);

  /** \brief Callback type used for functions with four arguments. */
  typedef value_type (*fun_type4)(value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type);

  /** \brief Callback type used for functions with five arguments. */
  typedef value_type (*fun_type5)(value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type);

  /** \brief Callback type used for functions with five arguments. */
  typedef value_type (*fun_type6)(value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type);

  /** \brief Callback type used for functions with five arguments. */
  typedef value_type (*fun_type7)(value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type);

  /** \brief Callback type used for functions with five arguments. */
  typedef value_type (*fun_type8)(value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type);

  /** \brief Callback type used for functions with five arguments. */
  typedef value_type (*fun_type9)(value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type, 
                                  value_type);

  /** \brief Callback type used for functions with five arguments. */
  typedef value_type (*fun_type10)(value_type, 
                                   value_type, 
                                   value_type, 
                                   value_type, 
                                   value_type, 
                                   value_type, 
                                   value_type, 
                                   value_type, 
                                   value_type, 
                                   value_type);

  /** \brief Callback used for functions that identify values in a string. */
  typedef int (*identfun_type)(const char_type *sExpr, int *nPos, value_type *fVal);

  /** \brief Callback used for variable creation factory functions. */
  typedef value_type* (*facfun_type)(const char_type*, void*);

  typedef value_type (*exprfun_type)();
} // end fo namespace

#endif
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
#include "mecParser.h"

//--- Standard includes ------------------------------------------------------------------------
#include <cmath>
#include <algorithm>
#include <numeric>

/** \brief Pi (what else?). */
#define PARSER_CONST_PI  3.141592653589793238462643

/** \brief The eulerian number. */
#define PARSER_CONST_E   2.718281828459045235360287

using namespace std;

/** \file
    \brief Implementation of the standard floating point parser.
*/


/** \brief Namespace for mathematical applications. */
namespace mec
{
  std::locale Parser::s_locale;

  //---------------------------------------------------------------------------
  // Binary operators
  value_type Parser::Pow(value_type v1, value_type v2) 
  { 
    int v2i = (int)v2;
    if (v2==v2i)
    {
      switch(v2i)
      {
      case 0:  return 1;
      case 1:  return v1;
      case 2:  return v1*v1;
      case 3:  return v1*v1*v1;
      case 4:  return v1*v1*v1*v1;
      case 5:  return v1*v1*v1*v1*v1;
      default: return std::pow(v1, v2i); 
      }
    }
    else
      return std::pow(v1, v2); 
  }

  //---------------------------------------------------------------------------
  // Trigonometric function
  value_type Parser::ASin(value_type v)  { return asin(v); }
  value_type Parser::ACos(value_type v)  { return acos(v); }
  value_type Parser::ATan(value_type v)  { return atan(v); }
  value_type Parser::Sinh(value_type v)  { return sinh(v); }
  value_type Parser::Cosh(value_type v)  { return cosh(v); }
  value_type Parser::Tanh(value_type v)  { return tanh(v); }
  value_type Parser::ASinh(value_type v) { return log(v + sqrt(v * v + 1)); }
  value_type Parser::ACosh(value_type v) { return log(v + sqrt(v * v - 1)); }
  value_type Parser::ATanh(value_type v) { return ((value_type)0.5 * log((1 + v) / (1 - v))); }

  //---------------------------------------------------------------------------
  // Logarithm functions
  value_type Parser::Log2(value_type v)  { return log(v)/log((value_type)2); } // Logarithm base 2
  value_type Parser::Log10(value_type v) { return log10(v); } // Logarithm base 10
  value_type Parser::Ln(value_type v)    { return log(v);   } // Logarithm base e (natural logarithm)

  //---------------------------------------------------------------------------
  //  misc
  value_type Parser::Exp(value_type v)  { return exp(v);   }
  value_type Parser::Rint(value_type v) { return floor(v + (value_type)0.5); }
  value_type Parser::Fmod(value_type v1, value_type v2) { return fmod(v1, v2); }
  value_type Parser::Sign(value_type v) { return (value_type)((v<0) ? -1 : (v>0) ? 1 : 0); }
  value_type Parser::Min(value_type v1, value_type v2) { return (v1<v2) ? v1 : v2; }
  value_type Parser::Max(value_type v1, value_type v2) { return (v1>v2) ? v1 : v2; }

  //---------------------------------------------------------------------------
  /** \brief Callback for the unary minus operator.
      \param v The value to negate
      \return -v
  */
  value_type Parser::UnaryMinus(value_type v) 
  { 
    return -v; 
  }

  //---------------------------------------------------------------------------
  value_type Parser::UnaryPlus(value_type v) 
  { 
    return v; 
  }

  //---------------------------------------------------------------------------
  /** \brief Default value recognition callback. 
      \param [in] a_szExpr Pointer to the expression
      \param [in, out] a_iPos Pointer to an index storing the current position within the expression
      \param [out] a_fVal Pointer where the value should be stored in case one is found.
      \return 1 if a value was found 0 otherwise.
  */
  int Parser::IsVal(const char_type* a_szExpr, int *a_iPos, value_type *a_fVal)
  {
    value_type fVal(0);

    stringstream_type stream(a_szExpr);
    stream.seekg(0);        // todo:  check if this really is necessary
    stream.imbue(Parser::s_locale);
    stream >> fVal;
    stringstream_type::pos_type iEnd = stream.tellg(); // Position after reading

    if (iEnd==(stringstream_type::pos_type)-1)
      return 0;

    *a_iPos += (int)iEnd;
    *a_fVal = fVal;
    return 1;
  }


  //---------------------------------------------------------------------------
  /** \brief Constructor. 

    Call ParserBase class constructor and trigger Function, Operator and Constant initialization.
  */
  Parser::Parser()
    :ParserBase()
  {
    // For some reason locale initialization in the dll fails when done in the static
    // constructor. I have to do it here
    static bool bInitLocale = true;
    if (bInitLocale)
    {
      Parser::s_locale = std::locale(std::locale::classic(), new change_dec_sep<char_type>('.'));
      bInitLocale = false;
    }

    AddValIdent(IsVal);

    InitCharSets();
    InitFun();
    InitConst();
    InitOprt();
  }

  //---------------------------------------------------------------------------
  /** \brief Define the character sets. 
      \sa DefineNameChars, DefineOprtChars, DefineInfixOprtChars
    
    This function is used for initializing the default character sets that define
    the characters to be useable in function and variable names and operators.
  */
  void Parser::InitCharSets()
  {
    DefineNameChars( _T("0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") );
    DefineOprtChars( _T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*^/?<>=#!$%&|~'_{µ}") );
    DefineInfixOprtChars( _T("/+-*^?<>=#!$%&|~'_") );
  }

  //---------------------------------------------------------------------------
  /** \brief Initialize the default functions. */
  void Parser::InitFun()
  {
    // arcus functions
    DefineFun(_T("asin"), ASin);
    DefineFun(_T("acos"), ACos);
    DefineFun(_T("atan"), ATan);
    // hyperbolic functions
    DefineFun(_T("sinh"), Sinh);
    DefineFun(_T("cosh"), Cosh);
    DefineFun(_T("tanh"), Tanh);
    // arcus hyperbolic functions
    DefineFun(_T("asinh"), ASinh);
    DefineFun(_T("acosh"), ACosh);
    DefineFun(_T("atanh"), ATanh);
    // Logarithm functions
    DefineFun(_T("log2"), Log2);
    DefineFun(_T("log10"), Log10);
    DefineFun(_T("log"), Ln);
    DefineFun(_T("ln"), Ln);
    // misc
    DefineFun(_T("exp"), Exp);
    DefineFun(_T("sign"), Sign);
    DefineFun(_T("rint"), Rint);

    DefineFun(_T("max"), Max);
    DefineFun(_T("min"), Min);
  }

  //---------------------------------------------------------------------------
  /** \brief Initialize constants.
  
    By default the parser recognizes two constants. Pi ("pi") and the eulerian
    number ("_e").
  */
  void Parser::InitConst()
  {
    DefineConst(_T("_pi"), (value_type)PARSER_CONST_PI);
    DefineConst(_T("_e"), (value_type)PARSER_CONST_E);
  }

  //---------------------------------------------------------------------------
  /** \brief Set the decimal separator.
      \param cDecSep Decimal separator as a character value.
      \sa SetThousandsSep

      By default muparser uses the "C" locale. The decimal separator of this
      locale is overwritten by the one provided here.
  */
  void Parser::SetDecSep(char_type cDecSep)
  {
    char_type cThousandsSep = std::use_facet< change_dec_sep<char_type> >(s_locale).thousands_sep();
    s_locale = std::locale(std::locale("C"), new change_dec_sep<char_type>(cDecSep, cThousandsSep));
  }
  
  //---------------------------------------------------------------------------
  /** \brief Sets the thousands operator. 
      \param cThousandsSep The thousands separator as a character
      \sa SetDecSep

      By default muparser uses the "C" locale. The thousands separator of this
      locale is overwritten by the one provided here.
  */
  void Parser::SetThousandsSep(char_type cThousandsSep)
  {
    char_type cDecSep = std::use_facet< change_dec_sep<char_type> >(s_locale).decimal_point();
    s_locale = std::locale(std::locale("C"), new change_dec_sep<char_type>(cDecSep, cThousandsSep));
  }

  //---------------------------------------------------------------------------
  /** \brief Resets the locale. 

    The default locale used "." as decimal separator, no thousands separator and
    "," as function argument separator.
  */
  void Parser::ResetLocale()
  {
    s_locale = std::locale(std::locale("C"), new change_dec_sep<char_type>('.'));
    SetArgSep(',');
  }

  //---------------------------------------------------------------------------
  /** \brief Initialize operators. 
  
    By default only the unary minus operator is added.
  */
  void Parser::InitOprt()
  {
    // infix operator definitions
    DefineInfixOprt(_T("-"), UnaryMinus);
    DefineInfixOprt(_T("+"), UnaryPlus);
    
    // unit postfix operators
    DefinePostfixOprt( _T("{G}"), Giga);
    DefinePostfixOprt( _T("{M}"), Mega);
    DefinePostfixOprt( _T("{k}"), Kilo);
    DefinePostfixOprt( _T("{m}"), Milli);
    DefinePostfixOprt( _T("{µ}"), Micro);
    DefinePostfixOprt( _T("{n}"), Nano);

    // binary operator definitions
    DefineOprt("^", Pow, prPOW, oaRIGHT);
    DefineOprt("%", Fmod, prMUL_DIV, oaLEFT);
  }
} // namespace mec

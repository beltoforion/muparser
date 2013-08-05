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
#ifndef MEC_PARSER_H
#define MEC_PARSER_H

//--- Standard includes ------------------------------------------------------------------------
#include <vector>
#include <locale>

//--- Parser includes --------------------------------------------------------------------------
#include "mecParserBase.h"

/** \file
    \brief Definition of the standard floating point parser.
*/

namespace mec
{
  /** \brief Mathematical expressions parser.
    
    Standard implementation of the mathematical expressions parser. 
    Can be used as a reference implementation for subclassing the parser.

    <small>
    (C) 2011 Ingo Berg<br>
    muparser(at)gmx.de
    </small>
  */
  class Parser : public ParserBase
  {
  public:

    Parser();

    virtual void InitCharSets();
    virtual void InitFun();
    virtual void InitConst();
    virtual void InitOprt();

    void SetDecSep(char_type cDecSep);
    void SetThousandsSep(char_type cThousandsSep = 0);
    void ResetLocale();

  private:

    /** \brief A facet class used to change decimal and thousands separator. */
    template<class TChar>
    class change_dec_sep : public std::numpunct<TChar>
    {
    public:
      
      explicit change_dec_sep(char_type cDecSep, char_type cThousandsSep = 0, int nGroup = 3)
        :std::numpunct<TChar>()
        ,m_cDecPoint(cDecSep)
        ,m_cThousandsSep(cThousandsSep)
        ,m_nGroup(nGroup)
      {}
      
    protected:
      
      virtual char_type do_decimal_point() const
      {
        return m_cDecPoint;
      }

      virtual char_type do_thousands_sep() const
      {
        return m_cThousandsSep;
      }

      virtual std::string do_grouping() const 
      { 
        return std::string(1, m_nGroup); 
      }

    private:

      int m_nGroup;
      char_type m_cDecPoint;  
      char_type m_cThousandsSep;
    };
     
    // unit postfixes
    static value_type Giga(value_type a_fVal)  { return a_fVal * (value_type)1e9;  }
    static value_type Mega(value_type a_fVal)  { return a_fVal * (value_type)1e6;  }
    static value_type Kilo(value_type a_fVal)  { return a_fVal * (value_type)1e3;  }
    static value_type Milli(value_type a_fVal) { return a_fVal * (value_type)1e-3; }
    static value_type Micro(value_type a_fVal) { return a_fVal * (value_type)1e-6; }
    static value_type Nano(value_type a_fVal)  { return a_fVal * (value_type)1e-9; }

    // binary operators
    static value_type Pow(value_type v1, value_type v2);

    // arcus functions
    static value_type  ASin(value_type);
    static value_type  ACos(value_type);
    static value_type  ATan(value_type);
    // hyperbolic functions
    static value_type  Sinh(value_type);
    static value_type  Cosh(value_type);
    static value_type  Tanh(value_type);
    // arcus hyperbolic functions
    static value_type  ASinh(value_type);
    static value_type  ACosh(value_type);
    static value_type  ATanh(value_type);
    // Logarithm functions
    static value_type  Log2(value_type);  // Logarithm Base 2
    static value_type  Log10(value_type); // Logarithm Base 10
    static value_type  Ln(value_type);    // Logarithm Base e (natural logarithm)
    // misc
    static value_type  Exp(value_type);
    static value_type  Rint(value_type);
    static value_type  Sign(value_type);
    static value_type  Fmod(value_type, value_type);
    static value_type  Min(value_type v1, value_type v2);
    static value_type  Max(value_type v1, value_type v2);

    // Prefix operators
    // !!! Unary Minus is a MUST if you want to use negative signs !!!
    static value_type  UnaryMinus(value_type);
    static value_type  UnaryPlus(value_type);

    static int IsVal(const char_type* a_szExpr, int *a_iPos, value_type *a_fVal);

    static std::locale s_locale;  ///< The locale used by the parser
  };
} // namespace mec

#endif

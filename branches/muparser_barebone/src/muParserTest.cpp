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

#include "muParserTest.h"

#include <cstdio>
#include <cmath>
#include <iostream>
#include <limits>

#define PARSER_CONST_PI  3.141592653589793238462643
#define PARSER_CONST_E   2.718281828459045235360287

using namespace std;

/** \file
    \brief This file contains the implementation of parser test cases.
*/

namespace mu
{
  namespace Test
  {
    int ParserTester::c_iCount = 0;

    //---------------------------------------------------------------------------------------------
    ParserTester::ParserTester()
      :m_vTestFun()
    {
      AddTest(&ParserTester::TestNames);
      AddTest(&ParserTester::TestSyntax);
      AddTest(&ParserTester::TestInfixOprt);
      AddTest(&ParserTester::TestVarConst);
      AddTest(&ParserTester::TestMultiArg);
      AddTest(&ParserTester::TestExpression);
      AddTest(&ParserTester::TestInterface);
      AddTest(&ParserTester::TestBinOprt);
      AddTest(&ParserTester::TestException);

      ParserTester::c_iCount = 0;
    }

    //---------------------------------------------------------------------------------------------
    int ParserTester::TestInterface()
    {
      int iStat = 0;
      mu::console() << _T("testing member functions...");
   
      // Test RemoveVar
      value_type afVal[3] = {1,2,3};
      Parser p;
  
      try
      {
        p.DefineVar( _T("a"), &afVal[0]);
        p.DefineVar( _T("b"), &afVal[1]);
        p.DefineVar( _T("c"), &afVal[2]);
        p.SetExpr( _T("a+b+c") );
        p.Eval();
      }
      catch(...)
      {
        iStat += 1;  // this is not supposed to happen 
      }

      try
      {
        p.RemoveVar( _T("c") );
        p.Eval();
        iStat += 1;  // not supposed to reach this, nonexisting variable "c" deleted...
      }
      catch(...)
      {
        // failure is expected...
      }

      if (iStat==0) 
        mu::console() << _T("passed") << endl;
      else 
        mu::console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------------------------
    int ParserTester::TestBinOprt()
    {
      int iStat = 0;
      mu::console() << _T("testing binary operators...");
   
      // built in operators
      iStat += EqnTest(_T("a<b"), 1, true);
      iStat += EqnTest(_T("b>a"), 1, true);
      iStat += EqnTest(_T("a>a"), 0, true);
      iStat += EqnTest(_T("a<a"), 0, true);
      iStat += EqnTest(_T("a>a"), 0, true);
      iStat += EqnTest(_T("a<=a"), 1, true);
      iStat += EqnTest(_T("a<=b"), 1, true);
      iStat += EqnTest(_T("b<=a"), 0, true);
      iStat += EqnTest(_T("a>=a"), 1, true);
      iStat += EqnTest(_T("b>=a"), 1, true);
      iStat += EqnTest(_T("a>=b"), 0, true);

      // Test logical operators, expecially if user defined "&" and the internal "&&" collide
      iStat += EqnTest(_T("1 && 1"), 1, true); 
      iStat += EqnTest(_T("1 && 0"), 0, true); 
      iStat += EqnTest(_T("(a<b) && (b>a)"), 1, true); 
      iStat += EqnTest(_T("(a<b) && (a>b)"), 0, true); 

      iStat += EqnTest(_T("2^2^3"), 256, true); 
      iStat += EqnTest(_T("1/2/3"), 1.0/6.0, true); 

      // reference: http://www.wolframalpha.com/input/?i=3%2B4*2%2F%281-5%29^2^3
      iStat += EqnTest(_T("3+4*2/(1-5)^2^3"), 3.0001220703125, true); 


      if (iStat==0)
        mu::console() << _T("passed") << endl;
      else 
        mu::console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------------------------
    /** \brief Check muParser name restriction enforcement. */
    int ParserTester::TestNames()
    {
      int  iStat= 0,
           iErr = 0;

      mu::console() << "testing name restriction enforcement...";
    
      Parser p;

  #define PARSER_THROWCHECK(DOMAIN, FAIL, EXPR, ARG) \
      iErr = 0;                                      \
      ParserTester::c_iCount++;                      \
      try                                            \
      {                                              \
        p.Define##DOMAIN(EXPR, ARG);                 \
      }                                              \
      catch(Parser::exception_type&)                 \
      {                                              \
        iErr = (FAIL==false) ? 0 : 1;                \
      }                                              \
      iStat += iErr;      
      
      // constant names
      PARSER_THROWCHECK(Const, false, _T("0a"), 1)
      PARSER_THROWCHECK(Const, false, _T("9a"), 1)
      PARSER_THROWCHECK(Const, false, _T("+a"), 1)
      PARSER_THROWCHECK(Const, false, _T("-a"), 1)
      PARSER_THROWCHECK(Const, false, _T("a-"), 1)
      PARSER_THROWCHECK(Const, false, _T("a*"), 1)
      PARSER_THROWCHECK(Const, false, _T("a?"), 1)
      PARSER_THROWCHECK(Const, true, _T("a"), 1)
      PARSER_THROWCHECK(Const, true, _T("a_min"), 1)
      PARSER_THROWCHECK(Const, true, _T("a_min0"), 1)
      PARSER_THROWCHECK(Const, true, _T("a_min9"), 1)
      // variable names
      value_type a;
      p.ClearConst();
      PARSER_THROWCHECK(Var, false, _T("123abc"), &a)
      PARSER_THROWCHECK(Var, false, _T("9a"), &a)
      PARSER_THROWCHECK(Var, false, _T("0a"), &a)
      PARSER_THROWCHECK(Var, false, _T("+a"), &a)
      PARSER_THROWCHECK(Var, false, _T("-a"), &a)
      PARSER_THROWCHECK(Var, false, _T("?a"), &a)
      PARSER_THROWCHECK(Var, false, _T("!a"), &a)
      PARSER_THROWCHECK(Var, false, _T("a+"), &a)
      PARSER_THROWCHECK(Var, false, _T("a-"), &a)
      PARSER_THROWCHECK(Var, false, _T("a*"), &a)
      PARSER_THROWCHECK(Var, false, _T("a?"), &a)
      PARSER_THROWCHECK(Var, true, _T("a"), &a)
      PARSER_THROWCHECK(Var, true, _T("a_min"), &a)
      PARSER_THROWCHECK(Var, true, _T("a_min0"), &a)
      PARSER_THROWCHECK(Var, true, _T("a_min9"), &a)
      PARSER_THROWCHECK(Var, false, _T("a_min9"), 0)
  #undef PARSER_THROWCHECK

      if (iStat==0) 
        mu::console() << _T("passed") << endl;
      else 
        mu::console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestSyntax()
    {
      int iStat = 0;
      mu::console() << _T("testing syntax engine...");

      iStat += ThrowTest(_T("1,"), ecUNEXPECTED_EOF);  // incomplete hex definition
      iStat += ThrowTest(_T("a,"), ecUNEXPECTED_EOF);  // incomplete hex definition
      iStat += ThrowTest(_T("sin(8),"), ecUNEXPECTED_EOF);  // incomplete hex definition
      iStat += ThrowTest(_T("(sin(8)),"), ecUNEXPECTED_EOF);  // incomplete hex definition

      iStat += EqnTest(_T("(1+ 2*a)"), 3, true);   // Spaces within formula
      iStat += EqnTest(_T("sqrt((4))"), 2, true);  // Multiple brackets
      iStat += EqnTest(_T("sqrt((2)+2)"), 2, true);// Multiple brackets
      iStat += EqnTest(_T("sqrt(2+(2))"), 2, true);// Multiple brackets
      iStat += EqnTest(_T("sqrt(a+(3))"), 2, true);// Multiple brackets
      iStat += EqnTest(_T("sqrt((3)+a)"), 2, true);// Multiple brackets
      iStat += EqnTest(_T("(2+"), 0, false);       // missing closing bracket 
      iStat += EqnTest(_T("2++4"), 0, false);      // unexpected operator
      iStat += EqnTest(_T("2+-4"), 0, false);      // unexpected operator
      iStat += EqnTest(_T("(2+)"), 0, false);      // unexpected closing bracket
      iStat += EqnTest(_T("--2"), 0, false);       // double sign
      iStat += EqnTest(_T("ksdfj"), 0, false);     // unknown token
      iStat += EqnTest(_T("()"), 0, false);        // empty bracket without a function
      iStat += EqnTest(_T("5+()"), 0, false);      // empty bracket without a function
      iStat += EqnTest(_T("sin(cos)"), 0, false);  // unexpected function
      iStat += EqnTest(_T("5t6"), 0, false);       // unknown token
      iStat += EqnTest(_T("5 t 6"), 0, false);     // unknown token
      iStat += EqnTest(_T("8*"), 0, false);        // unexpected end of formula
      iStat += EqnTest(_T(",3"), 0, false);        // unexpected comma
      iStat += EqnTest(_T("3,5"), 0, false);       // unexpected comma
      iStat += EqnTest(_T("sin(8,8)"), 0, false);  // too many function args
      iStat += EqnTest(_T("(7,8)"), 0, false);     // too many function args
      iStat += EqnTest(_T("sin)"), 0, false);      // unexpected closing bracket
      iStat += EqnTest(_T("a)"), 0, false);        // unexpected closing bracket
      iStat += EqnTest(_T("pi)"), 0, false);       // unexpected closing bracket
      iStat += EqnTest(_T("sin(())"), 0, false);   // unexpected closing bracket
      iStat += EqnTest(_T("sin()"), 0, false);     // unexpected closing bracket

      if (iStat==0)
        mu::console() << _T("passed") << endl;
      else 
        mu::console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestVarConst()
    {
      int iStat = 0;
      mu::console() << _T("testing variable/constant detection...");

      // Test if the result changes when a variable changes
      iStat += EqnTestWithVarChange( _T("a"), 1, 1, 2, 2 );
      iStat += EqnTestWithVarChange( _T("2*a"), 2, 4, 3, 6 );

      // distinguish constants with same basename
      iStat += EqnTest( _T("const"), 1, true);
      iStat += EqnTest( _T("const1"), 2, true);
      iStat += EqnTest( _T("const2"), 3, true);
      iStat += EqnTest( _T("2*const"), 2, true);
      iStat += EqnTest( _T("2*const1"), 4, true);
      iStat += EqnTest( _T("2*const2"), 6, true);
      iStat += EqnTest( _T("2*const+1"), 3, true);
      iStat += EqnTest( _T("2*const1+1"), 5, true);
      iStat += EqnTest( _T("2*const2+1"), 7, true);
      iStat += EqnTest( _T("const"), 0, false);
      iStat += EqnTest( _T("const1"), 0, false);
      iStat += EqnTest( _T("const2"), 0, false);

      // distinguish variables with same basename
      iStat += EqnTest( _T("a"), 1, true);
      iStat += EqnTest( _T("aa"), 2, true);
      iStat += EqnTest( _T("2*a"), 2, true);
      iStat += EqnTest( _T("2*aa"), 4, true);
      iStat += EqnTest( _T("2*a-1"), 1, true);
      iStat += EqnTest( _T("2*aa-1"), 3, true);

      if (iStat==0)  
        mu::console() << _T("passed") << endl;
      else
        mu::console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestMultiArg()
    {
      int iStat = 0;
      mu::console() << _T("testing multiarg functions...");
    
      // Compound expressions
      iStat += EqnTest( _T("1,2,3"), 3, true);
      iStat += EqnTest( _T("a,b,c"), 3, true);
      iStat += EqnTest( _T("1,\n2,\n3"), 3, true);
      iStat += EqnTest( _T("a,\nb,\nc"), 3, true);
      iStat += EqnTest( _T("1,\r\n2,\r\n3"), 3, true);
      iStat += EqnTest( _T("a,\r\nb,\r\nc"), 3, true);

      // picking the right argument
      iStat += EqnTest( _T("f1of1(1)"), 1, true);
      iStat += EqnTest( _T("f1of2(1, 2)"), 1, true);
      iStat += EqnTest( _T("f2of2(1, 2)"), 2, true);
      iStat += EqnTest( _T("f1of3(1, 2, 3)"), 1, true);
      iStat += EqnTest( _T("f2of3(1, 2, 3)"), 2, true);
      iStat += EqnTest( _T("f3of3(1, 2, 3)"), 3, true);
      // Too few arguments / Too many arguments
      iStat += EqnTest( _T("1+ping()"), 11, true);
      iStat += EqnTest( _T("ping()+1"), 11, true);
      iStat += EqnTest( _T("2*ping()"), 20, true);
      iStat += EqnTest( _T("ping()*2"), 20, true);
      iStat += EqnTest( _T("ping(1,2)"), 0, false);
      iStat += EqnTest( _T("1+ping(1,2)"), 0, false);
      iStat += EqnTest( _T("f1of1(1,2)"), 0, false);
      iStat += EqnTest( _T("f1of1()"), 0, false);
      iStat += EqnTest( _T("f1of2(1, 2, 3)"), 0, false);
      iStat += EqnTest( _T("f1of2(1)"), 0, false);
      iStat += EqnTest( _T("f1of3(1, 2, 3, 4)"), 0, false);
      iStat += EqnTest( _T("f1of3(1)"), 0, false);
      iStat += EqnTest( _T("(1,2,3)"), 0, false);
      iStat += EqnTest( _T("1,2,3"), 0, false);
      iStat += EqnTest( _T("(1*a,2,3)"), 0, false);
      iStat += EqnTest( _T("1,2*a,3"), 0, false);
     
      if (iStat==0) 
        mu::console() << _T("passed") << endl;
      else
        mu::console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;
  
      return iStat;
    }


    //---------------------------------------------------------------------------
    int ParserTester::TestInfixOprt()
    {
      int iStat(0);
      mu::console() << "testing infix operators...";

      iStat += EqnTest( _T("-1"),    -1, true);
      iStat += EqnTest( _T("-(-1)"),  1, true);
      iStat += EqnTest( _T("-(-1)*2"),  2, true);
      iStat += EqnTest( _T("-(-2)*sqrt(4)"),  4, true);
      iStat += EqnTest( _T("-_pi"), -PARSER_CONST_PI, true);
      iStat += EqnTest( _T("-a"),  -1, true);
      iStat += EqnTest( _T("-(a)"),  -1, true);
      iStat += EqnTest( _T("-(-a)"),  1, true);
      iStat += EqnTest( _T("-(-a)*2"),  2, true);
      iStat += EqnTest( _T("-(8)"), -8, true);
      iStat += EqnTest( _T("-8"), -8, true);
      iStat += EqnTest( _T("-(2+1)"), -3, true);
      iStat += EqnTest( _T("-(f1of1(1+2*3)+1*2)"), -9, true);
      iStat += EqnTest( _T("-(-f1of1(1+2*3)+1*2)"), 5, true);
      iStat += EqnTest( _T("-sin(8)"), -0.989358, true);
      iStat += EqnTest( _T("3-(-a)"), 4, true);
      iStat += EqnTest( _T("3--a"), 4, true);
      iStat += EqnTest( _T("-1*3"),  -3, true);

      iStat += EqnTest( _T("-2^2"),-4, true);
      iStat += EqnTest( _T("-(a+b)^2"),-9, true);
      iStat += EqnTest( _T("(-3)^2"),9, true);
      iStat += EqnTest( _T("-(-2^2)"),4, true);
      iStat += EqnTest( _T("3+-3^2"),-6, true);
      // This is the classic behaviour of the infix sign operator (here: "$") which is
      // now deprecated:
      iStat += EqnTest( _T("$2^2"),4, true);
      iStat += EqnTest( _T("$(a+b)^2"),9, true);
      iStat += EqnTest( _T("($3)^2"),9, true);
      iStat += EqnTest( _T("$($2^2)"),-4, true);
      iStat += EqnTest( _T("3+$3^2"),12, true);

      // infix operators sharing the first few characters
      iStat += EqnTest( _T("~ 123"),  123+2, true);
      iStat += EqnTest( _T("~~ 123"),  123+2, true);

      if (iStat==0)
        mu::console() << _T("passed") << endl;
      else
        mu::console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestExpression()
    {
      int iStat = 0;
      mu::console() << _T("testing expression samples...");

      value_type b = 2;

      // Optimization
      iStat += EqnTest( _T("2*b*5"), 20, true);
      iStat += EqnTest( _T("2*b*5 + 4*b"), 28, true);
      iStat += EqnTest( _T("2*a/3"), 2.0/3.0, true);

      iStat += EqnTest( _T("3+b"), b+3, true);
      iStat += EqnTest( _T("b+3"), b+3, true);
      iStat += EqnTest( _T("b*3+2"), b*3+2, true);
      iStat += EqnTest( _T("3*b+2"), b*3+2, true);
      iStat += EqnTest( _T("2+b*3"), b*3+2, true);
      iStat += EqnTest( _T("2+3*b"), b*3+2, true);
      iStat += EqnTest( _T("b+3*b"), b+3*b, true);
      iStat += EqnTest( _T("3*b+b"), b+3*b, true);

      iStat += EqnTest( _T("2+b*3+b"), 2+b*3+b, true);
      iStat += EqnTest( _T("b+2+b*3"), b+2+b*3, true);

      iStat += EqnTest( _T("(2*b+1)*4"), (2*b+1)*4, true);
      iStat += EqnTest( _T("4*(2*b+1)"), (2*b+1)*4, true);

      // operator precedencs
      iStat += EqnTest( _T("1+2-3*4/5^6"), 2.99923, true);
      iStat += EqnTest( _T("1^2/3*4-5+6"), 2.33333333, true);
      iStat += EqnTest( _T("1+2*3"), 7, true);
      iStat += EqnTest( _T("1+2*3"), 7, true);
      iStat += EqnTest( _T("(1+2)*3"), 9, true);
      iStat += EqnTest( _T("(1+2)*(-3)"), -9, true);
      iStat += EqnTest( _T("2/4"), 0.5, true);

      iStat += EqnTest( _T("exp(ln(7))"), 7, true);
      iStat += EqnTest( _T("e^ln(7)"), 7, true);
      iStat += EqnTest( _T("e^(ln(7))"), 7, true);
      iStat += EqnTest( _T("(e^(ln(7)))"), 7, true);
      iStat += EqnTest( _T("1-(e^(ln(7)))"), -6, true);
      iStat += EqnTest( _T("2*(e^(ln(7)))"), 14, true);
      iStat += EqnTest( _T("10^log(5)"), 5, true);
      iStat += EqnTest( _T("10^log10(5)"), 5, true);
      iStat += EqnTest( _T("2^log2(4)"), 4, true);
      iStat += EqnTest( _T("-(sin(0)+1)"), -1, true);
      iStat += EqnTest( _T("-(2^1.1)"), -2.14354692, true);

      iStat += EqnTest( _T("(cos(2.41)/b)"), -0.372056, true);
      iStat += EqnTest( _T("(1*(2*(3*(4*(5*(6*(a+b)))))))"), 2160, true);
      iStat += EqnTest( _T("(1*(2*(3*(4*(5*(6*(7*(a+b))))))))"), 15120, true);
      iStat += EqnTest( _T("(a/((((b+(((e*(((((pi*((((3.45*((pi+a)+pi))+b)+b)*a))+0.68)+e)+a)/a))+a)+b))+b)*a)-pi))"), 0.00377999, true);

      // long formula (Reference: Matlab)
      iStat += EqnTest(
        _T("(((-9))-e/(((((((pi-(((-7)+(-3)/4/e))))/(((-5))-2)-((pi+(-0))*(sqrt((e+e))*(-8))*(((-pi)+(-pi)-(-9)*(6*5))")
        _T("/(-e)-e))/2)/((((sqrt(2/(-e)+6)-(4-2))+((5/(-2))/(1*(-pi)+3))/8)*pi*((pi/((-2)/(-6)*1*(-1))*(-6)+(-e)))))/")
        _T("((e+(-2)+(-e)*((((-3)*9+(-e)))+(-9)))))))-((((e-7+(((5/pi-(3/1+pi)))))/e)/(-5))/(sqrt((((((1+(-7))))+((((-")
        _T("e)*(-e)))-8))*(-5)/((-e)))*(-6)-((((((-2)-(-9)-(-e)-1)/3))))/(sqrt((8+(e-((-6))+(9*(-9))))*(((3+2-8))*(7+6")
        _T("+(-5))+((0/(-e)*(-pi))+7)))+(((((-e)/e/e)+((-6)*5)*e+(3+(-5)/pi))))+pi))/sqrt((((9))+((((pi))-8+2))+pi))/e")
        _T("*4)*((-5)/(((-pi))*(sqrt(e)))))-(((((((-e)*(e)-pi))/4+(pi)*(-9)))))))+(-pi)"), -12.23016549, true);

      // long formula (Reference: Matlab)
      iStat += EqnTest(
          _T("(atan(sin((((((((((((((((pi/cos((a/((((0.53-b)-pi)*e)/b))))+2.51)+a)-0.54)/0.98)+b)*b)+e)/a)+b)+a)+b)+pi)/e")
          _T(")+a)))*2.77)"), -2.16995656, true);

      // long formula (Reference: Matlab)
      iStat += EqnTest( _T("1+2-3*4/5^6*(2*(1-5+(3*7^9)*(4+6*7-3)))+12"), -7995810.09926, true);
	  
      if (iStat==0) 
        mu::console() << _T("passed") << endl;  
      else 
        mu::console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestException()
    {
      int  iStat = 0;
      mu::console() << _T("testing error codes...");

      iStat += ThrowTest(_T("3+"),           ecUNEXPECTED_EOF);
      iStat += ThrowTest(_T("3+)"),          ecUNEXPECTED_PARENS);
      iStat += ThrowTest(_T("()"),           ecUNEXPECTED_PARENS);
      iStat += ThrowTest(_T("3+()"),         ecUNEXPECTED_PARENS);
      iStat += ThrowTest(_T("sin(3,4)"),     ecTOO_MANY_PARAMS);
      iStat += ThrowTest(_T("sin()"),        ecTOO_FEW_PARAMS);
      iStat += ThrowTest(_T("(1+2"),         ecMISSING_PARENS);
      iStat += ThrowTest(_T("sin(3)3"),      ecUNEXPECTED_VAL);
      iStat += ThrowTest(_T("sin(3)xyz"),    ecUNASSIGNABLE_TOKEN);
      iStat += ThrowTest(_T("sin(3)cos(3)"), ecUNEXPECTED_FUN);
      
      // No positive sign operator
      iStat += ThrowTest(_T("a^+2"),         ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest(_T("a+(+a)"),       ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest(_T("a++a"),         ecUNEXPECTED_OPERATOR);

      // functions without parameter
      iStat += ThrowTest( _T("3+ping(2)"),    ecTOO_MANY_PARAMS);
      iStat += ThrowTest( _T("3+ping(a+2)"),  ecTOO_MANY_PARAMS);
      iStat += ThrowTest( _T("3+ping(sin(a)+2)"),  ecTOO_MANY_PARAMS);
      iStat += ThrowTest( _T("3+ping(1+sin(a))"),  ecTOO_MANY_PARAMS);

      // <ibg 20090529>
      // this is now legal, for reference see:
      // https://sourceforge.net/forum/message.php?msg_id=7411373
      //      iStat += ThrowTest( _T("sin=9"), ecUNEXPECTED_OPERATOR);    
      // </ibg>

      if (iStat==0) 
        mu::console() << _T("passed") << endl;
      else 
        mu::console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }


    //---------------------------------------------------------------------------
    void ParserTester::AddTest(testfun_type a_pFun)
    {
      m_vTestFun.push_back(a_pFun);
    }

    //---------------------------------------------------------------------------
    void ParserTester::Run()
    {
      int iStat = 0;
      try
      {
        for (int i=0; i<(int)m_vTestFun.size(); ++i)
          iStat += (this->*m_vTestFun[i])();
      }
      catch(Parser::exception_type &e)
      {
        mu::console() << "\n" << e.GetMsg() << endl;
        mu::console() << e.GetToken() << endl;
        Abort();
      }
      catch(std::exception &e)
      {
        mu::console() << e.what() << endl;
        Abort();
      }
      catch(...)
      {
        mu::console() << "Internal error";
        Abort();
      }

      if (iStat==0) 
      {
        mu::console() << "Test passed (" <<  ParserTester::c_iCount << " expressions)" << endl;
      }
      else 
      {
        mu::console() << "Test failed with " << iStat 
                  << " errors (" <<  ParserTester::c_iCount 
                  << " expressions)" << endl;
      }
      ParserTester::c_iCount = 0;
    }


    //---------------------------------------------------------------------------
    int ParserTester::ThrowTest(const string_type &a_str, int a_iErrc, bool a_bFail)
    {
      ParserTester::c_iCount++;

      try
      {
        value_type fVal[] = {1,1,1};
        Parser p;

        p.DefineVar( _T("a"), &fVal[0]);
        p.DefineVar( _T("b"), &fVal[1]);
        p.DefineVar( _T("c"), &fVal[2]);
        p.DefineFun( _T("ping"), Ping);
        p.SetExpr(a_str);
        p.Eval();
      }
      catch(ParserError &e)
      {
        // output the formula in case of an failed test
        if (a_bFail==false || (a_bFail==true && a_iErrc!=e.GetCode()) )
        {
          mu::console() << _T("\n  ") 
                        << _T("Expression: ") << a_str 
                        << _T("  Code:") << e.GetCode() << _T("(") << e.GetMsg() << _T(")")
                        << _T("  Expected:") << a_iErrc;
        }

        return (a_iErrc==e.GetCode()) ? 0 : 1;
      }

      // if a_bFail==false no exception is expected
      bool bRet((a_bFail==false) ? 0 : 1);
      if (bRet==1)
      {
        mu::console() << _T("\n  ") 
                      << _T("Expression: ") << a_str 
                      << _T("  did evaluate; Expected error:") << a_iErrc;
      }

      return bRet; 
    }

    //---------------------------------------------------------------------------
    /** \brief Evaluate a tet expression. 

        \return 1 in case of a failure, 0 otherwise.
    */
    int ParserTester::EqnTestWithVarChange(const string_type &a_str, 
                                           double a_fVar1, 
                                           double a_fRes1, 
                                           double a_fVar2, 
                                           double a_fRes2)
    {
      ParserTester::c_iCount++;
      value_type fVal[2] = {-999, -999 }; // should be equalinitially

      try
      {
        Parser  p;

        // variable
        value_type var = 0;
        p.DefineVar( _T("a"), &var);
        p.SetExpr(a_str);

        var = a_fVar1;
        fVal[0] = p.Eval();

        var = a_fVar2;
        fVal[1] = p.Eval();
        
        if ( fabs(a_fRes1-fVal[0]) > 0.0000000001)
          throw std::runtime_error("incorrect result (first pass)");

        if ( fabs(a_fRes2-fVal[1]) > 0.0000000001)
          throw std::runtime_error("incorrect result (second pass)");
      }
      catch(Parser::exception_type &e)
      {
        mu::console() << _T("\n  fail: ") << a_str.c_str() << _T(" (") << e.GetMsg() << _T(")");
        return 1;
      }
      catch(std::exception &e)
      {
        mu::console() << _T("\n  fail: ") << a_str.c_str() << _T(" (") << e.what() << _T(")");
        return 1;  // always return a failure since this exception is not expected
      }
      catch(...)
      {
        mu::console() << _T("\n  fail: ") << a_str.c_str() <<  _T(" (unexpected exception)");
        return 1;  // exceptions other than ParserException are not allowed
      }

      return 0;
    }

    //---------------------------------------------------------------------------
    /** \brief Evaluate a tet expression. 

        \return 1 in case of a failure, 0 otherwise.
    */
    int ParserTester::EqnTest(const string_type &a_str, double a_fRes, bool a_fPass)
    {
      ParserTester::c_iCount++;
      int iRet(0);
      value_type fVal[4] = {-999, -998, -997, -996}; // initially should be different

      try
      {
        std::auto_ptr<Parser> p1;
        Parser  p2, p3;   // three parser objects
                          // they will be used for testing copy and assihnment operators
        // p1 is a pointer since i'm going to delete it in order to test if
        // parsers after copy construction still refer to members of it.
        // !! If this is the case this function will crash !!
      
        p1.reset(new mu::Parser()); 
        // Add constants
        p1->DefineConst( _T("pi"), (value_type)PARSER_CONST_PI);
        p1->DefineConst( _T("e"), (value_type)PARSER_CONST_E);
        p1->DefineConst( _T("const"), 1);
        p1->DefineConst( _T("const1"), 2);
        p1->DefineConst( _T("const2"), 3);
        // variables
        value_type vVarVal[] = { 1, 2, 3, -2};
        p1->DefineVar( _T("a"), &vVarVal[0]);
        p1->DefineVar( _T("aa"), &vVarVal[1]);
        p1->DefineVar( _T("b"), &vVarVal[1]);
        p1->DefineVar( _T("c"), &vVarVal[2]);
        p1->DefineVar( _T("d"), &vVarVal[3]);
        
        // functions
        p1->DefineFun( _T("ping"), Ping);
        p1->DefineFun( _T("f1of1"), f1of1);  // one parameter
        p1->DefineFun( _T("f1of2"), f1of2);  // two parameter
        p1->DefineFun( _T("f2of2"), f2of2);
        p1->DefineFun( _T("f1of3"), f1of3);  // three parameter
        p1->DefineFun( _T("f2of3"), f2of3);
        p1->DefineFun( _T("f3of3"), f3of3);

        // sample functions
        p1->DefineFun( _T("min"), Min);
        p1->DefineFun( _T("max"), Max);

        // infix / postfix operator
        // Note: Identifiers used here do not have any meaning 
        //       they are mere placeholders to test certain features.
        p1->DefineInfixOprt( _T("$"), sign, prPOW+1);  // sign with high priority
        p1->DefineInfixOprt( _T("~"), plus2);          // high priority
        p1->DefineInfixOprt( _T("~~"), plus2);
        p1->SetExpr(a_str);

        // Test bytecode integrity
        // String parsing and bytecode parsing must yield the same result
        fVal[0] = p1->Eval(); // result from stringparsing
        fVal[1] = p1->Eval(); // result from bytecode
        if (fVal[0]!=fVal[1])
          throw Parser::exception_type( _T("Bytecode / string parsing mismatch.") );

        // Test copy and assignement operators
        try
        {
          // Test copy constructor
          std::vector<mu::Parser> vParser;
          vParser.push_back(*(p1.get()));
          mu::Parser p2 = vParser[0];   // take parser from vector
        
          // destroy the originals from p2
          vParser.clear();              // delete the vector
          p1.reset(0);

          fVal[2] = p2.Eval();

          // Test Eval function for multiple return values
          // use p2 since it has the optimizer enabled!
          int nNum;
          value_type *v = p2.Eval(nNum);
          fVal[3] = v[nNum-1];
        }
        catch(std::exception &e)
        {
          mu::console() << _T("\n  ") << e.what() << _T("\n");
        }

        // limited floating point accuracy requires the following test
        bool bCloseEnough(true);
        for (unsigned i=0; i<sizeof(fVal)/sizeof(value_type); ++i)
        {
          bCloseEnough &= (fabs(a_fRes-fVal[i]) <= fabs(fVal[i]*0.00001));

          // The tests equations never result in infinity, if they do thats a bug.
          // reference:
          // http://sourceforge.net/projects/muparser/forums/forum/462843/topic/5037825
          if (numeric_limits<value_type>::has_infinity)
            bCloseEnough &= (fabs(fVal[i]) != numeric_limits<value_type>::infinity());
        }

        iRet = ((bCloseEnough && a_fPass) || (!bCloseEnough && !a_fPass)) ? 0 : 1;
        
        
        if (iRet==1)
        {
          mu::console() << _T("\n  fail: ") << a_str.c_str() 
                        << _T(" (incorrect result; expected: ") << a_fRes
                        << _T(" ;calculated: ") << fVal[0] << _T(",") 
                                                << fVal[1] << _T(",")
                                                << fVal[2] << _T(",")
                                                << fVal[3] << _T(",")
                                                << fVal[4] << _T(").");
        }
      }
      catch(Parser::exception_type &e)
      {
        if (a_fPass)
        {
          if (fVal[0]!=fVal[2] && fVal[0]!=-999 && fVal[1]!=-998)
            mu::console() << _T("\n  fail: ") << a_str.c_str() << _T(" (copy construction)");
          else
            mu::console() << _T("\n  fail: ") << a_str.c_str() << _T(" (") << e.GetMsg() << _T(")");
          return 1;
        }
      }
      catch(std::exception &e)
      {
        mu::console() << _T("\n  fail: ") << a_str.c_str() << _T(" (") << e.what() << _T(")");
        return 1;  // always return a failure since this exception is not expected
      }
      catch(...)
      {
        mu::console() << _T("\n  fail: ") << a_str.c_str() <<  _T(" (unexpected exception)");
        return 1;  // exceptions other than ParserException are not allowed
      }

      return iRet;
    }

    //---------------------------------------------------------------------------
    /** \brief Internal error in test class Test is going to be aborted. */
    void ParserTester::Abort() const
    {
      mu::console() << _T("Test failed (internal error in test class)") << endl;
      while (!getchar());
      exit(-1);
    }
  } // namespace test
} // namespace mu

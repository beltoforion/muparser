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

#include "mecUnitTest.h"

#include <cstdio>
#include <cmath>
#include <iostream>

#ifndef PARSER_CONST_PI
#define PARSER_CONST_PI  3.141592653589793238462643
#endif

#ifndef PARSER_CONST_E
#define PARSER_CONST_E   2.718281828459045235360287
#endif

using namespace std;

/** \file
    \brief This file contains the implementation of parser test cases.
*/


namespace mec
{
  namespace Test
  {
    int UnitTest::c_iCount = 0;

    //---------------------------------------------------------------------------
    UnitTest::UnitTest()
      :m_vTestFun()
    {
      AddTest(&UnitTest::TestException);
      AddTest(&UnitTest::TestNames);
      AddTest(&UnitTest::TestInterface);
      AddTest(&UnitTest::TestSyntax);
      AddTest(&UnitTest::TestVarConst);
      AddTest(&UnitTest::TestPostFix);
      AddTest(&UnitTest::TestLogic);
      AddTest(&UnitTest::TestInfixOprt);
      AddTest(&UnitTest::TestBinOprt);
      AddTest(&UnitTest::TestExpression);
      AddTest(&UnitTest::TestMultiArg);
      AddTest(&UnitTest::TestIfThenElse);

      UnitTest::c_iCount = 0;
    }

    //---------------------------------------------------------------------------
    int UnitTest::TestInterface()
    {
      int iStat = 0;
      console() << _T("testing member functions...");
   
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
        console() << _T("passed") << endl;
      else 
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int UnitTest::TestBinOprt()
    {
      int iStat = 0;
      console() << _T("testing binary operators...");
   
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
      iStat += EqnTest(_T("2^2^3"), 256, true);

      iStat += EqnTest(_T("a++b"), 3, true);
      iStat += EqnTest(_T("a ++ b"), 3, true);
      iStat += EqnTest(_T("1++2"), 3, true);
      iStat += EqnTest(_T("1 ++ 2"), 3, true);
      iStat += EqnTest(_T("a add b"), 3, true);
      iStat += EqnTest(_T("1 add 2"), 3, true);

      iStat += EqnTest(_T("2^2^3"), 256, true); 
      iStat += EqnTest(_T("1/2/3"), 1.0f/6.0f, true); 
      iStat += EqnTest(_T("3+4*2/(1-5)^2^3"), 3.0001220703125f, true); 

      // modulo operator
      iStat += EqnTest(_T("7 % 2"), 1, true);
      iStat += EqnTest(_T("6 % 2"), 0, true);
      iStat += EqnTest(_T("7 % b"), 1, true);
      iStat += EqnTest(_T("6 % b"), 0, true);
      iStat += EqnTest(_T("(6.2+a) % -(b+.1)"), 0.9f, true);

      if (iStat==0)
        console() << _T("passed") << endl;
      else 
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    /** \brief Check muParser name restriction enforcement. */
    int UnitTest::TestNames()
    {
      int  iStat= 0,
           iErr = 0;

      console() << "testing name restriction enforcement...";
    
      Parser p;

  #define PARSER_THROWCHECK(DOMAIN, FAIL, EXPR, ARG) \
      iErr = 0;                                      \
      UnitTest::c_iCount++;                      \
      try                                            \
      {                                              \
        p.Define##DOMAIN(EXPR, ARG);                 \
      }                                              \
      catch(ParserError&)                            \
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
      // Postfix operators
      // fail
      PARSER_THROWCHECK(PostfixOprt, false, _T("(k"), f1of1)
      PARSER_THROWCHECK(PostfixOprt, false, _T("9+"), f1of1)
      PARSER_THROWCHECK(PostfixOprt, false, _T("+"), 0)
      // pass
      PARSER_THROWCHECK(PostfixOprt, true, _T("-a"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("?a"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("_"),   f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("#"),   f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("&&"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("||"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("&"),   f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("|"),   f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("++"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("--"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("?>"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("?<"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("**"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("xor"), f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("and"), f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("or"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("not"), f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, _T("!"),   f1of1)
      // Binary operator
      // The following must fail due to name collisions with 
      // built in operators
      PARSER_THROWCHECK(Oprt, false, _T("+"),  f1of2)
      PARSER_THROWCHECK(Oprt, false, _T("-"),  f1of2)
      PARSER_THROWCHECK(Oprt, false, _T("*"),  f1of2)
      PARSER_THROWCHECK(Oprt, false, _T("/"),  f1of2)
  #undef PARSER_THROWCHECK

      if (iStat==0) 
        console() << _T("passed") << endl;
      else 
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int UnitTest::TestSyntax()
    {
      int iStat = 0;
      console() << _T("testing syntax engine...");

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
        console() << _T("passed") << endl;
      else 
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int UnitTest::TestVarConst()
    {
      int iStat = 0;
      console() << _T("testing variable/constant name recognition...");

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

      // Finally test querying of used variables
      try
      {
        int idx;
        Parser p;
        value_type vVarVal[] = { 1, 2, 3, 4, 5};
        p.DefineVar( _T("a"), &vVarVal[0]);
        p.DefineVar( _T("b"), &vVarVal[1]);
        p.DefineVar( _T("c"), &vVarVal[2]);
        p.DefineVar( _T("d"), &vVarVal[3]);
        p.DefineVar( _T("e"), &vVarVal[4]);

        // Test lookup of defined variables
        // 4 used variables
        p.SetExpr( _T("a+b+c+d") );
        varmap_type UsedVar = p.GetUsedVar();
        int iCount = (int)UsedVar.size();
        if (iCount!=4) throw false;

        varmap_type::const_iterator item = UsedVar.begin();
        for (idx=0; item!=UsedVar.end(); ++item)
        {
          if (&vVarVal[idx++]!=item->second) 
            throw false;
        }

        // Test lookup of undefined variables
        p.SetExpr( _T("undef1+undef2+undef3") );
        UsedVar = p.GetUsedVar();
        iCount = (int)UsedVar.size();
        if (iCount!=3) throw false;

        for (item = UsedVar.begin(); item!=UsedVar.end(); ++item)
        {
          if (item->second!=0) 
            throw false; // all pointers to undefined variables must be null
        }

        // 1 used variables
        p.SetExpr( _T("a+b") );
        UsedVar = p.GetUsedVar();
        iCount = (int)UsedVar.size();
        if (iCount!=2) throw false;
        item = UsedVar.begin();
        for (idx=0; item!=UsedVar.end(); ++item)
          if (&vVarVal[idx++]!=item->second) throw false;

      }
      catch(...)
      {
        iStat += 1;
      }

      if (iStat==0)  
        console() << _T("passed") << endl;
      else
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int UnitTest::TestMultiArg()
    {
      int iStat = 0;
      console() << _T("testing multiarg functions...");
    
      // picking the right argument
      iStat += EqnTest( _T("f1of1(1)"), 1, true);
      iStat += EqnTest( _T("f1of2(1, 2)"), 1, true);
      iStat += EqnTest( _T("f2of2(1, 2)"), 2, true);
      iStat += EqnTest( _T("f1of3(1, 2, 3)"), 1, true);
      iStat += EqnTest( _T("f2of3(1, 2, 3)"), 2, true);
      iStat += EqnTest( _T("f3of3(1, 2, 3)"), 3, true);
      iStat += EqnTest( _T("f1of4(1, 2, 3, 4)"), 1, true);
      iStat += EqnTest( _T("f2of4(1, 2, 3, 4)"), 2, true);
      iStat += EqnTest( _T("f3of4(1, 2, 3, 4)"), 3, true);
      iStat += EqnTest( _T("f4of4(1, 2, 3, 4)"), 4, true);
      iStat += EqnTest( _T("f1of5(1, 2, 3, 4, 5)"), 1, true);
      iStat += EqnTest( _T("f2of5(1, 2, 3, 4, 5)"), 2, true);
      iStat += EqnTest( _T("f3of5(1, 2, 3, 4, 5)"), 3, true);
      iStat += EqnTest( _T("f4of5(1, 2, 3, 4, 5)"), 4, true);
      iStat += EqnTest( _T("f5of5(1, 2, 3, 4, 5)"), 5, true);
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
      iStat += EqnTest( _T("f1of4(1, 2, 3, 4, 5)"), 0, false);
      iStat += EqnTest( _T("f1of4(1)"), 0, false);
      iStat += EqnTest( _T("(1,2,3)"), 0, false);
      iStat += EqnTest( _T("1,2,3"), 0, false);
      iStat += EqnTest( _T("(1*a,2,3)"), 0, false);
      iStat += EqnTest( _T("1,2*a,3"), 0, false);
     
      // correct calculation of arguments
      iStat += EqnTest( _T("min(a, 1)"),  1, true);
      iStat += EqnTest( _T("min(3*2, 1)"),  1, true);
      iStat += EqnTest( _T("min(3*2, 1)"),  6, false);
      iStat += EqnTest( _T("min(3*a+1, 1)"),  1, true);
      iStat += EqnTest( _T("max(3*a+1, 1)"),  4, true);
      iStat += EqnTest( _T("max(3*a+1, 1)*2"),  8, true);
      iStat += EqnTest( _T("2*max(3*a+1, 1)+2"),  10, true);

      if (iStat==0) 
        console() << _T("passed") << endl;
      else
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;
  
      return iStat;
    }


    //---------------------------------------------------------------------------
    int UnitTest::TestInfixOprt()
    {
      int iStat(0);
      console() << "testing infix operators...";

      iStat += EqnTest( _T("-1"),    -1, true);
      iStat += EqnTest( _T("-(-1)"),  1, true);
      iStat += EqnTest( _T("-(-1)*2"),  2, true);
      iStat += EqnTest( _T("-(-2)*sqrt(4)"),  4, true);
      iStat += EqnTest( _T("-a"),  -1, true);
      iStat += EqnTest( _T("-(a)"),  -1, true);
      iStat += EqnTest( _T("-(-a)"),  1, true);
      iStat += EqnTest( _T("-(-a)*2"),  2, true);
      iStat += EqnTest( _T("-(8)"), -8, true);
      iStat += EqnTest( _T("-8"), -8, true);
      iStat += EqnTest( _T("-(2+1)"), -3, true);
      iStat += EqnTest( _T("-(f1of1(1+2*3)+1*2)"), -9, true);
      iStat += EqnTest( _T("-(-f1of1(1+2*3)+1*2)"), 5, true);
      iStat += EqnTest( _T("-sin(8)"), -0.989358f, true);
      iStat += EqnTest( _T("3-(-a)"), 4, true);
      iStat += EqnTest( _T("3--a"), 4, true);
  
      // Postfix / infix priorities
      iStat += EqnTest( _T("~2#"), 8, true);
      iStat += EqnTest( _T("~f1of1(2)#"), 8, true);
      iStat += EqnTest( _T("~(b)#"), 8, true);
      iStat += EqnTest( _T("(~b)#"), 12, true);
      iStat += EqnTest( _T("~(2#)"), 8, true);
      iStat += EqnTest( _T("~(f1of1(2)#)"), 8, true);
      //
      iStat += EqnTest( _T("-2^2"),-4, true);
      iStat += EqnTest( _T("-(a+b)^2"),-9, true);
      iStat += EqnTest( _T("(-3)^2"),9, true);
      iStat += EqnTest( _T("-(-2^2)"),4, true);
      iStat += EqnTest( _T("3+-3^2"),-6, true);
      // The following assumes use of sqr as postfix operator ("?") together
      // withn a sign operator of low priority:
      iStat += EqnTest( _T("-2'"), -4, true);
      iStat += EqnTest( _T("-(1+1)'"),-4, true);
      iStat += EqnTest( _T("2+-(1+1)'"),-2, true);
      iStat += EqnTest( _T("2+-2'"), -2, true);
      // This is the classic behaviour of the infix sign operator (here: "$") which is
      // now deprecated:
      iStat += EqnTest( _T("$2^2"),4, true);
      iStat += EqnTest( _T("$(a+b)^2"),9, true);
      iStat += EqnTest( _T("($3)^2"),9, true);
      iStat += EqnTest( _T("$($2^2)"),-4, true);
      iStat += EqnTest( _T("3+$3^2"),12, true);

      if (iStat==0)
        console() << _T("passed") << endl;
      else
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int UnitTest::TestPostFix()
    {
      int iStat = 0;
      console() << _T("testing postfix operators...");

      // application
      iStat += EqnTest( _T("3{m}+5"), 5.003f, true);
      iStat += EqnTest( _T("1000{m}"), 1, true);
      iStat += EqnTest( _T("1000 {m}"), 1, true);
      iStat += EqnTest( _T("(a){m}"), 1e-3f, true);
      iStat += EqnTest( _T("a{m}"), 1e-3f, true);
      iStat += EqnTest( _T("a {m}"), 1e-3f, true);
      iStat += EqnTest( _T("-(a){m}"), -1e-3f, true);
      iStat += EqnTest( _T("-2{m}"), -2e-3f, true);
      iStat += EqnTest( _T("-2 {m}"), -2e-3f, true);
      iStat += EqnTest( _T("f1of1(1000){m}"), 1, true);
      iStat += EqnTest( _T("-f1of1(1000){m}"), -1, true);
      iStat += EqnTest( _T("-f1of1(-1000){m}"), 1, true);
      iStat += EqnTest( _T("f2of2(0,1000){m}"), 1, true);
      iStat += EqnTest( _T("f3of3(0, 0,1000){m}"), 1, true);
      iStat += EqnTest( _T("f4of4(0,0,0,1000){m}"), 1, true);
      iStat += EqnTest( _T("2+(a*1000){m}"), 3, true);

      // some incorrect results
      iStat += EqnTest( _T("1000{m}"), 0.1f, false);
      iStat += EqnTest( _T("(a){m}"), 2, false);
      // failure due to syntax checking
      iStat += ThrowTest(_T("0x"), ecUNASSIGNABLE_TOKEN);  // incomplete hex definition
      iStat += ThrowTest(_T("3+"), ecUNEXPECTED_EOF);
      iStat += ThrowTest( _T("4 + {m}"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( _T("{m}4"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( _T("sin({m})"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( _T("{m} {m}"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( _T("{m}(8)"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( _T("4,{m}"), ecUNEXPECTED_ARG_SEP);
      iStat += ThrowTest( _T("-{m}"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( _T("2(-{m})"), ecUNEXPECTED_PARENS);
      iStat += ThrowTest( _T("2({m})"), ecUNEXPECTED_PARENS);

      if (iStat==0)
        console() << _T("passed") << endl;
      else
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int UnitTest::TestLogic()
    {
      int iStat = 0;
      console() << _T("testing locic operators...");

      // Test logic operators
      iStat += EqnTest( _T("1 || 2"), 1, true);
      iStat += EqnTest( _T("a || b"), 1, true);
      iStat += EqnTest( _T("1 && 1"), 1, true);
      iStat += EqnTest( _T("a && b"), 1, true);
      iStat += EqnTest( _T("0 && b"), 0, true);
      iStat += EqnTest( _T("a && 0"), 0, true);
      iStat += EqnTest( _T("(a<b) && 1"), 1, true);
      iStat += EqnTest( _T("(a<b) && (b>a)"), 1, true);
      iStat += EqnTest( _T("(a<b) || (b>a)"), 1, true);
      iStat += EqnTest( _T("(a>b) && (b>a)"), 0, true);
      iStat += EqnTest( _T("(a<b) && (b<a)"), 0, true);
      iStat += EqnTest( _T("(sin(8)<b)"), 1, true);

      if (iStat==0)
        console() << _T("passed") << endl;
      else
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int UnitTest::TestIfThenElse()
    {
      int iStat = 0;
      mec::console() << _T("testing if-then-else operator...");

      // Test error detection
      iStat += ThrowTest(_T("(a<b) ? (b<c) ? 1 : 2"), ecMISSING_ELSE_CLAUSE); 
      iStat += ThrowTest(_T("(a<b) ? 1"), ecMISSING_ELSE_CLAUSE); 
      iStat += ThrowTest(_T("(a<b) ? a"), ecMISSING_ELSE_CLAUSE); 
      iStat += ThrowTest(_T("(a<b) ? a+b"), ecMISSING_ELSE_CLAUSE); 
      iStat += ThrowTest(_T("a : b"), ecMISPLACED_COLON); 
      iStat += ThrowTest(_T("1 : 2"), ecMISPLACED_COLON); 
      iStat += ThrowTest(_T("(1) ? 1 : 2 : 3"), ecMISPLACED_COLON); 
      iStat += ThrowTest(_T("(true) ? 1 : 2 : 3"), ecUNASSIGNABLE_TOKEN); 

      iStat += EqnTest(_T("1 ? 128 : 255"), 128, true);
      iStat += EqnTest(_T("1<2 ? 128 : 255"), 128, true);
      iStat += EqnTest(_T("a<b ? 128 : 255"), 128, true);
      iStat += EqnTest(_T("(a<b) ? 128 : 255"), 128, true);
      iStat += EqnTest(_T("(1) ? 10 : 11"), 10, true);
      iStat += EqnTest(_T("(0) ? 10 : 11"), 11, true);
      iStat += EqnTest(_T("(1) ? a+b : c+d"), 3, true);
      iStat += EqnTest(_T("(0) ? a+b : c+d"), 1, true);
      iStat += EqnTest(_T("(1) ? 0 : 1"), 0, true);
      iStat += EqnTest(_T("(0) ? 0 : 1"), 1, true);
      iStat += EqnTest(_T("(a<b) ? 10 : 11"), 10, true);
      iStat += EqnTest(_T("(a>b) ? 10 : 11"), 11, true);
      iStat += EqnTest(_T("(a<b) ? c : d"), 3, true);
      iStat += EqnTest(_T("(a>b) ? c : d"), -2, true);

      iStat += EqnTest(_T("(a>b) ? 1 : 0"), 0, true);
      iStat += EqnTest(_T("((a>b) ? 1 : 0) ? 1 : 2"), 2, true);

      // todo: auch für muParserX hinzufügen!
      iStat += EqnTest(_T("(a<b)&&(a<b) ? 128 : 255"), 128, true);
      iStat += EqnTest(_T("(a>b)&&(a<b) ? 128 : 255"), 255, true);
      iStat += EqnTest(_T("(1<2)&&(1<2) ? 128 : 255"), 128, true);
      iStat += EqnTest(_T("(1>2)&&(1<2) ? 128 : 255"), 255, true);
      iStat += EqnTest(_T("((1<2)&&(1<2)) ? 128 : 255"), 128, true);
      iStat += EqnTest(_T("((1>2)&&(1<2)) ? 128 : 255"), 255, true);
      iStat += EqnTest(_T("((a<b)&&(a<b)) ? 128 : 255"), 128, true);
      iStat += EqnTest(_T("((a>b)&&(a<b)) ? 128 : 255"), 255, true);

      iStat += EqnTest(_T("1>0 ? 1>2 ? 128 : 255 : 1>0 ? 32 : 64"), 255, true);
      iStat += EqnTest(_T("1>0 ? 1>2 ? 128 : 255 :(1>0 ? 32 : 64)"), 255, true);
      iStat += EqnTest(_T("1>0 ? 50 :  1>0 ? 128 : 255"), 50, true);
      iStat += EqnTest(_T("1>0 ? 50 : (1>0 ? 128 : 255)"), 50, true);
      iStat += EqnTest(_T("1>0 ? 1>0 ? 128 : 255 : 1>2 ? 32 : 64"), 128, true);
      iStat += EqnTest(_T("1>0 ? 1>0 ? 128 : 255 :(1>2 ? 32 : 64)"), 128, true);
      iStat += EqnTest(_T("1>2 ? 1>2 ? 128 : 255 : 1>0 ? 32 : 64"), 32, true);
      iStat += EqnTest(_T("1>2 ? 1>0 ? 128 : 255 : 1>2 ? 32 : 64"), 64, true);
      iStat += EqnTest(_T("1>0 ? 1>0 ? 128 : 255 : 50"), 128, true);
      iStat += EqnTest(_T("1>2 ? 1>2 ? 128 : 255 : 1>0 ? 32 : 1>2 ? 64 : 16"), 32, true);
      iStat += EqnTest(_T("1>2 ? 1>2 ? 128 : 255 : 1>0 ? 32 :(1>2 ? 64 : 16)"), 32, true);
      iStat += EqnTest(_T("1>0 ? 1>2 ? 128 : 255 :  1>0 ? 32 :1>2 ? 64 : 16"), 255, true);
      iStat += EqnTest(_T("1>0 ? 1>2 ? 128 : 255 : (1>0 ? 32 :1>2 ? 64 : 16)"), 255, true);
      iStat += EqnTest(_T("1 ? 0 ? 128 : 255 : 1 ? 32 : 64"), 255, true);
      if (iStat==0) 
        mec::console() << _T("passed") << endl;  
      else 
        mec::console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int UnitTest::TestExpression()
    {
      int iStat = 0;
      console() << _T("testing expression samples...");

      // operator precedencs
      iStat += EqnTest( _T("1+2-3*4/5^6"), 2.99923f, true);
      iStat += EqnTest( _T("1^2/3*4-5+6"), 2.3333f, true);
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
      iStat += EqnTest( _T("10^log10(5)"), 5, true);
      iStat += EqnTest( _T("10^log10(5)"), 5, true);
      iStat += EqnTest( _T("2^log2(4)"), 4, true);
      iStat += EqnTest( _T("-(sin(0)+1)"), -1, true);
      iStat += EqnTest( _T("-(2^1.1)"), -2.14354692f, true);

      iStat += EqnTest( _T("(cos(2.41)/b)"), -0.372056f, true);

      // testing register overflow due to high equation complexity
      iStat += EqnTest( _T("(1*(2*(3*(4*(5*(6*(a+b)))))))"), 2160, true);
      iStat += EqnTest( _T("(1*(2*(3*(4*(5*(6*(7*(a+b))))))))"), 15120, true);
      iStat += EqnTest( _T("1+(2+(3+(4+(5+(6+(sqrt(5)))))))"), 23.23607f, true);
      iStat += EqnTest( _T("1+(2+(3+(4+(5+(6+(sin(5)))))))"), 20.04108f, true);
      iStat += EqnTest( _T("1+(2+(3+(4+(5+(6+(f1of1(8)))))))"), 29, true);

      iStat += EqnTest( _T("(a/((((b+(((e*(((((pi*((((3.45*((pi+a)+pi))+b)+b)*a))+0.68)+e)+a)/a))+a)+b))+b)*a)-pi))"), 0.00377999f, true);

      // long formula (Reference: Matlab)
      iStat += EqnTest(
        _T("(((-9))-e/(((((((pi-(((-7)+(-3)/4/e))))/(((-5))-2)-((pi+(-0))*(sqrt((e+e))*(-8))*(((-pi)+(-pi)-(-9)*(6*5))")
        _T("/(-e)-e))/2)/((((sqrt(2/(-e)+6)-(4-2))+((5/(-2))/(1*(-pi)+3))/8)*pi*((pi/((-2)/(-6)*1*(-1))*(-6)+(-e)))))/")
        _T("((e+(-2)+(-e)*((((-3)*9+(-e)))+(-9)))))))-((((e-7+(((5/pi-(3/1+pi)))))/e)/(-5))/(sqrt((((((1+(-7))))+((((-")
        _T("e)*(-e)))-8))*(-5)/((-e)))*(-6)-((((((-2)-(-9)-(-e)-1)/3))))/(sqrt((8+(e-((-6))+(9*(-9))))*(((3+2-8))*(7+6")
        _T("+(-5))+((0/(-e)*(-pi))+7)))+(((((-e)/e/e)+((-6)*5)*e+(3+(-5)/pi))))+pi))/sqrt((((9))+((((pi))-8+2))+pi))/e")
        _T("*4)*((-5)/(((-pi))*(sqrt(e)))))-(((((((-e)*(e)-pi))/4+(pi)*(-9)))))))+(-pi)"), -12.23016549f, true);

      // long formula (Reference: Matlab)
      iStat += EqnTest(
          _T("(atan(sin((((((((((((((((pi/cos((a/((((0.53-b)-pi)*e)/b))))+2.51)+a)-0.54)/0.98)+b)*b)+e)/a)+b)+a)+b)+pi)/e")
          _T(")+a)))*2.77)"), -2.16995656f, true);

      // long formula (Reference: Matlab)
      iStat += EqnTest( _T("1+2-3*4/5^6*(2*(1-5+(3*7^9)*(4+6*7-3)))+12"), -7995810.09926f, true);
      iStat += EqnTest( _T("2-3/5^6*(2*(5+3*7^9))"), -46485.4f, true);
      iStat += EqnTest( _T("2-3/5^6*(2*(5+3*7^9))"), -46485.4f, true);
	  
      if (iStat==0) 
        console() << _T("passed") << endl;  
      else 
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }


    //---------------------------------------------------------------------------
    int UnitTest::TestException()
    {
      int  iStat = 0;
      console() << _T("testing error codes...");

      iStat += ThrowTest(_T("3+"),           ecUNEXPECTED_EOF);
      iStat += ThrowTest(_T("3+)"),          ecUNEXPECTED_PARENS);
      iStat += ThrowTest(_T("()"),           ecUNEXPECTED_PARENS);
      iStat += ThrowTest(_T("3+()"),         ecUNEXPECTED_PARENS);
      iStat += ThrowTest(_T("sin(3,4)"),     ecTOO_MANY_PARAMS);
      iStat += ThrowTest(_T("3,4"),          ecUNEXPECTED_ARG_SEP);
      iStat += ThrowTest(_T("(1+2"),         ecMISSING_PARENS);
      iStat += ThrowTest(_T("sin(3)3"),      ecUNEXPECTED_VAL);
      iStat += ThrowTest(_T("sin(3)xyz"),    ecUNASSIGNABLE_TOKEN);
      iStat += ThrowTest(_T("sin(3)cos(3)"), ecUNEXPECTED_FUN);

      // functions without parameter
      iStat += ThrowTest( _T("3+ping(2)"),    ecTOO_MANY_PARAMS);
      iStat += ThrowTest( _T("3+ping(a+2)"),  ecTOO_MANY_PARAMS);
      iStat += ThrowTest( _T("3+ping(sin(a)+2)"),  ecTOO_MANY_PARAMS);
      iStat += ThrowTest( _T("3+ping(1+sin(a))"),  ecTOO_MANY_PARAMS);

      if (iStat==0) 
        console() << _T("passed") << endl;
      else 
        console() << _T("\n  failed with ") << iStat << _T(" errors") << endl;

      return iStat;
    }


    //---------------------------------------------------------------------------
    void UnitTest::AddTest(testfun_type a_pFun)
    {
      m_vTestFun.push_back(a_pFun);
    }

    //---------------------------------------------------------------------------
    void UnitTest::Run()
    {
      int iStat = 0;
      try
      {
        for (int i=0; i<(int)m_vTestFun.size(); ++i)
          iStat += (this->*m_vTestFun[i])();
      }
      catch(ParserError &e)
      {
        console() << "\n" << e.GetMsg() << endl;
        console() << e.GetToken() << endl;
        Abort();
      }
      catch(std::exception &e)
      {
        console() << e.what() << endl;
        Abort();
      }
      catch(...)
      {
        console() << "Internal error";
        Abort();
      }

      if (iStat==0) 
      {
        console() << "Test passed (" <<  std::dec << UnitTest::c_iCount << " expressions)" << endl;
      }
      else 
      {
        console() << "Test failed with " << iStat 
                  << " errors (" <<  UnitTest::c_iCount 
                  << " expressions)" << endl;
      }
      UnitTest::c_iCount = 0;
    }


    //---------------------------------------------------------------------------
    int UnitTest::ThrowTest(const string_type &a_str, int a_iErrc, bool a_bFail)
    {
      UnitTest::c_iCount++;

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
          console() << _T("\n  ") 
                        << _T("Expression: ") << a_str 
                        << _T("  Code:") << e.GetCode() 
                        << _T("  Expected:") << a_iErrc;
        }

        return (a_iErrc==e.GetCode()) ? 0 : 1;
      }

      // if a_bFail==false no exception is expected
      bool bRet((a_bFail==false) ? 0 : 1);
      if (bRet==1)
      {
        console() << _T("\n  ") 
                      << _T("Expression: ") << a_str 
                      << _T("  did evaluate; Expected error:") << a_iErrc;
      }

      return bRet; 
    }

    //---------------------------------------------------------------------------
    /** \brief Evaluate a tet expression. 

        \return 1 in case of a failure, 0 otherwise.
    */
    int UnitTest::EqnTest(const string_type &a_str, value_type a_fRes, bool a_fPass)
    {
      UnitTest::c_iCount++;
      int iRet(0);

//#define NO_JIT
#if defined(NO_JIT)
      value_type fVal[3] = {-999, -998, -997};
#else
    value_type fVal[10] = {-999, -998, -997, -996, -995, -994, -993, -992, -991, -990 }; // initially should be different
#endif


      try
      {
        std::auto_ptr<Parser> p1;
        Parser  p2, p3;   // three parser objects
                          // they will be used for testing copy and assihnment operators
        // p1 is a pointer since i'm going to delete it in order to test if
        // parsers after copy construction still refer to members of it.
        // !! If this is the case this function will crash !!
      
        p1.reset(new Parser()); 

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
        p1->DefineFun( _T("f1of4"), f1of4);  // four parameter
        p1->DefineFun( _T("f2of4"), f2of4);
        p1->DefineFun( _T("f3of4"), f3of4);
        p1->DefineFun( _T("f4of4"), f4of4);
        p1->DefineFun( _T("f1of5"), f1of5);  // five parameter
        p1->DefineFun( _T("f2of5"), f2of5);
        p1->DefineFun( _T("f3of5"), f3of5);
        p1->DefineFun( _T("f4of5"), f4of5);
        p1->DefineFun( _T("f5of5"), f5of5);
        p1->DefineFun( _T("min"), Min);
        p1->DefineFun( _T("max"), Max);

        // binary operators
        p1->DefineOprt( _T("add"), add, 0);
        p1->DefineOprt( _T("++"), add, 0);
        p1->DefineOprt( _T("&"), and, 0);

        // infix / postfix operator
        // (identifiers used here do not have any meaning or make any sense at all)
        p1->DefineInfixOprt( _T("$"), sign, prPOW+1);  // sign with high priority
        p1->DefineInfixOprt( _T("~"), plus2);          // high priority
        p1->DefinePostfixOprt( _T("#"), times3);
        p1->DefinePostfixOprt( _T("'"), sqr);  //
        p1->SetExpr(a_str);
        p1->SetParserEngine(peBYTECODE_ASM);
//        p1->SetParserEngine(peBYTECODE);

        // Test bytecode integrity
        // String parsing and bytecode parsing must yield the same result
        fVal[0] = p1->Eval(); // result from stringparsing
        fVal[1] = p1->Eval(); // result from bytecode
//        if (fVal[0]!=fVal[1])
        if (fabs(fVal[0]-fVal[1]) > fabs(fVal[1]*0.0001))
          throw ParserError( _T("Bytecode / string parsing mismatch.") );

        // Test copy and assignement operators
        try
        {
          // Test copy constructor
          std::vector<Parser> vParser;
          vParser.push_back(*(p1.get()));
          Parser p2 = vParser[0];   // take parser from vector
        
          // destroy the originals from p2
          vParser.clear();              // delete the vector
          p1.reset(0);

          p2.SetParserEngine(peBYTECODE);
          p2.Eval(); // Reinit parser and create bytecode
          fVal[2] = p2.Eval();

#if !defined(NO_JIT)
          // Test assignement operator
          Parser p3;
          p3 = p2;

          // finally test the jit compiled parser with differnt values
          // for the number of sse registers to use
          exprfun_type ptfun;
          ptfun = p3.Compile(-1); // don't use sse registers as calculation stack
          fVal[3] = ptfun();

          ptfun = p3.Compile(0); // xmm0 (doesnt make any sense but will expose bugs in the engine...)
          fVal[4] = ptfun();

          ptfun = p3.Compile(1); // xmm0, xmm1 (doesnt make any sense but will expose bugs in the engine...)
          fVal[5] = ptfun();

          ptfun = p3.Compile(2); // xmm0, xmm1, xmm2
          fVal[6] = ptfun();

          ptfun = p3.Compile(3); // xmm0, xmm1, xmm2, xmm3
          fVal[7] = ptfun();

          ptfun = p3.Compile(4); // xmm0, xmm1, xmm2, xmm3, xmm4
          fVal[8] = ptfun();

          ptfun = p3.Compile(5); // xmm0, xmm1, xmm2, xmm3, xmm4, xmm5
          fVal[9] = ptfun();
#endif
        }
        catch(std::exception &e)
        {
          console() << _T("\n  ") << e.what() << _T("\n");
        }

        // limited floating point accuracy requires the following test
        bool bCloseEnough(true);
        for (int i=0; i<sizeof(fVal)/sizeof(value_type); ++i)
        {
          bCloseEnough &= (fabs(a_fRes-fVal[i]) <= fabs(fVal[i]*0.0001));
        }

        iRet = ((bCloseEnough && a_fPass) || (!bCloseEnough && !a_fPass)) ? 0 : 1;
        if (iRet==1)
        {
          console() << _T("\n  fail: ") << a_str.c_str() 
                        << _T(" (incorrect result; expected: ") << a_fRes
                        << _T(" ;calculated: ") << fVal[0] << ";" 
                                                << fVal[1] << ";" 
                                                << fVal[2] << ";" 
                                                << fVal[3] << ";" 
                                                << fVal[4] << ";" 
                                                << fVal[5] << _T(").");
        }
      }
      catch(ParserError &e)
      {
        if (a_fPass)
        {
          if (fVal[0]!=fVal[2] && fVal[0]!=-999 && fVal[1]!=-998)
            console() << _T("\n  fail: ") << a_str.c_str() << _T(" (copy construction)");
          else
            console() << _T("\n  fail: ") << a_str.c_str() << _T(" (") << e.GetMsg() << _T(")");
          return 1;
        }
      }
      catch(std::exception &e)
      {
        console() << _T("\n  fail: ") << a_str.c_str() << _T(" (") << e.what() << _T(")");
        return 1;  // always return a failure since this exception is not expected
      }
      catch(...)
      {
        console() << _T("\n  fail: ") << a_str.c_str() <<  _T(" (unexpected exception)");
        return 1;  // exceptions other than ParserException are not allowed
      }

      return iRet;
    }

    //---------------------------------------------------------------------------
    /** \brief Internal error in test class Test is going to be aborted. */
    void UnitTest::Abort() const
    {
      console() << _T("Test failed (internal error in test class)") << endl;
      while (!getchar());
      exit(-1);
    }
  } // namespace test
} // namespace mec

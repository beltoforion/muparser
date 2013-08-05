#ifndef _MU_PARSER_TEST_H
#define _MU_PARSER_TEST_H

#include <string>
#include <cstdlib>
#include <numeric> // for accumulate
#include "muParser.h"


MUP_NAMESPACE_START

  namespace Test
  {
    template<typename TValue, typename TString>
    class ParserTester 
    {
    private:

      typedef std::basic_stringstream<typename TString::value_type,
                                      std::char_traits<typename TString::value_type>,  
                                      std::allocator<typename TString::value_type> > stringstream_type;

      static int c_iCount;

      // Multiarg callbacks
      	
      static void MUP_FASTCALL arg2(TValue *arg, int)
      {
        arg[0] = arg[1];
      }

      static void MUP_FASTCALL Min(TValue *arg, int)
      {
        arg[0] = (arg[0]<arg[1]) ? arg[0] : arg[1];
      }

      static void MUP_FASTCALL Max(TValue* arg, int /*argc*/)
      {
        arg[0] = (arg[0]>arg[1]) ? arg[0] : arg[1];
      }

      static void MUP_FASTCALL plus2(TValue* arg, int /*argc*/)
      { 
        arg[0] += 2;
      }

      static void MUP_FASTCALL times3(TValue* arg, int /*argc*/)
      { 
        arg[0] *= 3; 
      }
        
      static void MUP_FASTCALL sqr(TValue* arg, int /*argc*/)
      { 
        arg[0] = arg[0]*arg[0]; 
      }

      static void MUP_FASTCALL land(TValue* arg, int /*argc*/)
      {
        arg[0] = (int)arg[0] & (int)arg[1];
      }
        
      static void MUP_FASTCALL FirstArg(TValue* /*arg*/, int argc)
      {
        if (!argc)	
          throw ParserError<TString>(_SL("too few arguments for function FirstArg."));
      }

      static void MUP_FASTCALL LastArg(TValue *arg, int argc)
      {
        if (!argc)	
          throw ParserError<TString>(_SL("too few arguments for function LastArg."));

        arg[0] = arg[argc-1];
      }

      static void MUP_FASTCALL Ping(TValue *arg, int)
      { 
        arg[0] = 10;
      }

      // postfix operator callback
      static void MUP_FASTCALL Mega(TValue *arg , int) { arg[0] *= (TValue)1e6;  }
      static void MUP_FASTCALL Micro(TValue *arg, int) { arg[0] *= (TValue)1e-6; }
      static void MUP_FASTCALL Milli(TValue *arg, int) { arg[0] *= (TValue)1e-3; }

      // Custom value recognition

      //-----------------------------------------------------------------------------------------
      static int IsHexVal(const typename TString::value_type *a_szExpr, int *a_iPos, TValue *a_fVal)
      {
        if (a_szExpr[1]==0 || (a_szExpr[0]!='0' || a_szExpr[1]!='x') ) 
          return 0;

        unsigned iVal(0);

        // New code based on streams for UNICODE compliance:
        stringstream_type::pos_type nPos(0);
        stringstream_type ss(a_szExpr + 2);
        ss >> std::hex >> iVal;
        nPos = ss.tellg();

        if (nPos==(stringstream_type::pos_type)0)
          return 1;

        *a_iPos += (int)(2 + nPos);
        *a_fVal = (TValue)iVal;
        return 1;
      }

      //---------------------------------------------------------------------------------------------
      int TestInterface()
      {
        int iStat = 0;
        _OUT << _SL("testing member functions...");
   
        // Test RemoveVar
        TValue afVal[3] = {1,2,3};
        Parser<TValue, TString> p;
  
        try
        {
          p.DefineVar(_SL("a"), &afVal[0]);
          p.DefineVar(_SL("b"), &afVal[1]);
          p.DefineVar(_SL("c"), &afVal[2]);
          p.SetExpr(_SL("a+b+c"));
          p.Eval();
        }
        catch(...)
        {
          iStat += 1;  // this is not supposed to happen 
        }

        try
        {
          p.RemoveVar( _SL("c") );
          p.Eval();
          iStat += 1;  // not supposed to reach this, nonexisting variable "c" deleted...
        }
        catch(...)
        {
          // failure is expected...
        }

        if (iStat==0) 
          _OUT << _SL("passed") << endl;
        else 
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;

        return iStat;
      }

      //---------------------------------------------------------------------------------------------
      int TestOptimizer()
      {
        int iStat = 0;
        _OUT << _SL("testing optimizer...");
    
        // Test substitution of consequtive binary operators:
        iStat += EqnTest(_SL("b*(a-b/a)"), -2, true);

        if (iStat==0)
          _OUT << _SL("passed") << endl;
        else 
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;

        return iStat;
      }

      //---------------------------------------------------------------------------------------------
      int TestBinOprt()
      {
        int iStat = 0;
        _OUT << _SL("testing binary operators...");
    
        // associativity
        iStat += EqnTest(_SL("4-5+6"), 4-5+6, true);

        //
        iStat += EqnTest(_SL("a++b"), 3, true);
        iStat += EqnTest(_SL("a ++ b"), 3, true);
        iStat += EqnTest(_SL("1++2"), 3, true);
        iStat += EqnTest(_SL("1 ++ 2"), 3, true);
        iStat += EqnTest(_SL("a add b"), 3, true);
        iStat += EqnTest(_SL("1 add 2"), 3, true);
        iStat += EqnTest(_SL("a<b"), 1, true);
        iStat += EqnTest(_SL("b>a"), 1, true);
        iStat += EqnTest(_SL("a>a"), 0, true);
        iStat += EqnTest(_SL("a<a"), 0, true);
        iStat += EqnTest(_SL("a>a"), 0, true);
        iStat += EqnTest(_SL("a<=a"), 1, true);
        iStat += EqnTest(_SL("a<=b"), 1, true);
        iStat += EqnTest(_SL("b<=a"), 0, true);
        iStat += EqnTest(_SL("a>=a"), 1, true);
        iStat += EqnTest(_SL("b>=a"), 1, true);
        iStat += EqnTest(_SL("a>=b"), 0, true);

        // Test logical operators, expecially if user defined "&" and the internal "&&" collide
        iStat += EqnTest(_SL("1 && 1"), 1, true); 
        iStat += EqnTest(_SL("1 && 0"), 0, true); 
        iStat += EqnTest(_SL("(a<b) && (b>a)"), 1, true); 
        iStat += EqnTest(_SL("(a<b) && (a>b)"), 0, true); 
        iStat += EqnTest(_SL("12 & 255"), 12, true); 
        iStat += EqnTest(_SL("12 & 0"), 0, true); 
        iStat += EqnTest(_SL("12&255"), 12, true); 
        iStat += EqnTest(_SL("12&0"), 0, true); 

        // Assignement operator
        iStat += EqnTest(_SL("a = b"), 2, true); 
        iStat += EqnTest(_SL("a = sin(b)"), (TValue)0.909297, true); 
        iStat += EqnTest(_SL("a = 1+sin(b)"), (TValue)1.909297, true);
        iStat += EqnTest(_SL("(a=b)*2"), 4, true);
        iStat += EqnTest(_SL("2*(a=b)"), 4, true);
        iStat += EqnTest(_SL("2*(a=b+1)"), 6, true);
        iStat += EqnTest(_SL("(a=b+1)*2"), 6, true);

        iStat += EqnTest(_SL("2^2^3"), 256, true); 
        iStat += EqnTest(_SL("1/2/3"), (TValue)1.0/(TValue)6.0, true); 

        iStat += EqnTest(_SL("b^-2^3+1"), (TValue)1.00390625, true); 

        // reference: http://www.wolframalpha.com/input/?i=3%2B4*2%2F%281-5%29^2^3
        iStat += EqnTest(_SL("3+4*2/(1-5)^2^3"), 3.0001220703125, true); 

        if (iStat==0)
          _OUT << _SL("passed") << endl;
        else 
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;

        return iStat;
      }

      //---------------------------------------------------------------------------
      int TestSyntax()
      {
        int iStat = 0;
        _OUT << _SL("testing syntax engine...");

        iStat += ThrowTest(_SL("1,"), ecUNEXPECTED_EOF);  // incomplete hex definition
        iStat += ThrowTest(_SL("a,"), ecUNEXPECTED_EOF);  // incomplete hex definition
        iStat += ThrowTest(_SL("sin(8),"), ecUNEXPECTED_EOF);  // incomplete hex definition
        iStat += ThrowTest(_SL("(sin(8)),"), ecUNEXPECTED_EOF);  // incomplete hex definition
        iStat += ThrowTest(_SL("a{m},"), ecUNEXPECTED_EOF);  // incomplete hex definition

        iStat += EqnTest(_SL("(1+ 2*a)"), 3, true);   // Spaces within formula
        iStat += EqnTest(_SL("sqrt((4))"), 2, true);  // Multiple brackets
        iStat += EqnTest(_SL("sqrt((2)+2)"), 2, true);// Multiple brackets
        iStat += EqnTest(_SL("sqrt(2+(2))"), 2, true);// Multiple brackets
        iStat += EqnTest(_SL("sqrt(a+(3))"), 2, true);// Multiple brackets
        iStat += EqnTest(_SL("sqrt((3)+a)"), 2, true);// Multiple brackets
        iStat += EqnTest(_SL("(2+"), 0, false);       // missing closing bracket 
        iStat += EqnTest(_SL("2++4"), 0, false);      // unexpected operator
        iStat += EqnTest(_SL("2+-4"), 0, false);      // unexpected operator
        iStat += EqnTest(_SL("(2+)"), 0, false);      // unexpected closing bracket
        iStat += EqnTest(_SL("--2"), 0, false);       // sign
        iStat += EqnTest(_SL("ksdfj"), 0, false);     // unknown token
        iStat += EqnTest(_SL("()"), 0, false);        // empty bracket without a function
        iStat += EqnTest(_SL("5+()"), 0, false);      // empty bracket without a function
        iStat += EqnTest(_SL("sin(cos)"), 0, false);  // unexpected function
        iStat += EqnTest(_SL("5t6"), 0, false);       // unknown token
        iStat += EqnTest(_SL("5 t 6"), 0, false);     // unknown token
        iStat += EqnTest(_SL("8*"), 0, false);        // unexpected end of formula
        iStat += EqnTest(_SL(",3"), 0, false);        // unexpected comma
        iStat += EqnTest(_SL("3,5"), 0, false);       // unexpected comma
        iStat += EqnTest(_SL("sin(8,8)"), 0, false);  // too many function args
        iStat += EqnTest(_SL("(7,8)"), 0, false);     // too many function args
        iStat += EqnTest(_SL("sin)"), 0, false);      // unexpected closing bracket
        iStat += EqnTest(_SL("a)"), 0, false);        // unexpected closing bracket
        iStat += EqnTest(_SL("pi)"), 0, false);       // unexpected closing bracket
        iStat += EqnTest(_SL("sin(())"), 0, false);   // unexpected closing bracket
        iStat += EqnTest(_SL("sin()"), 0, false);     // unexpected closing bracket

        if (iStat==0)
          _OUT << _SL("passed") << endl;
        else 
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;

        return iStat;
      }

      //---------------------------------------------------------------------------
      int TestVarConst()
      {
        int iStat = 0;
        _OUT << _SL("testing variable/constant detection...");

        // Test if the result changes when a variable changes
        iStat += EqnTestWithVarChange( _SL("a"), 1, 1, 2, 2 );
        iStat += EqnTestWithVarChange( _SL("2*a"), 2, 4, 3, 6 );

        // distinguish constants with same basename
        iStat += EqnTest( _SL("const"), 1, true);
        iStat += EqnTest( _SL("const1"), 2, true);
        iStat += EqnTest( _SL("const2"), 3, true);
        iStat += EqnTest( _SL("2*const"), 2, true);
        iStat += EqnTest( _SL("2*const1"), 4, true);
        iStat += EqnTest( _SL("2*const2"), 6, true);
        iStat += EqnTest( _SL("2*const+1"), 3, true);
        iStat += EqnTest( _SL("2*const1+1"), 5, true);
        iStat += EqnTest( _SL("2*const2+1"), 7, true);
        iStat += EqnTest( _SL("const"), 0, false);
        iStat += EqnTest( _SL("const1"), 0, false);
        iStat += EqnTest( _SL("const2"), 0, false);

        // distinguish variables with same basename
        iStat += EqnTest( _SL("a"), 1, true);
        iStat += EqnTest( _SL("aa"), 2, true);
        iStat += EqnTest( _SL("2*a"), 2, true);
        iStat += EqnTest( _SL("2*aa"), 4, true);
        iStat += EqnTest( _SL("2*a-1"), 1, true);
        iStat += EqnTest( _SL("2*aa-1"), 3, true);

        // custom value recognition
        iStat += EqnTest( _SL("0xff"), 255, true);
        iStat += EqnTest( _SL("0x97 + 0xff"), 406, true);

        // Finally test querying of used variables
        try
        {
          int idx;
          Parser<TValue, TString> p;
          TValue vVarVal[] = { 1, 2, 3, 4, 5};
          p.DefineVar( _SL("a"), &vVarVal[0]);
          p.DefineVar( _SL("b"), &vVarVal[1]);
          p.DefineVar( _SL("c"), &vVarVal[2]);
          p.DefineVar( _SL("d"), &vVarVal[3]);
          p.DefineVar( _SL("e"), &vVarVal[4]);

          // Test lookup of defined variables
          // 4 used variables
          p.SetExpr( _SL("a+b+c+d") );
          std::map<TString, TValue*> UsedVar = p.GetUsedVar();
          int iCount = (int)UsedVar.size();
          if (iCount!=4) 
            throw false;
        
          // the next check will fail if the parser 
          // erroneousely creates new variables internally
          if (p.GetVar().size()!=5)
            throw false;

          auto item = UsedVar.begin();
          for (idx=0; item!=UsedVar.end(); ++item)
          {
            if (&vVarVal[idx++]!=item->second) 
              throw false;
          }

          // Test lookup of undefined variables
          p.SetExpr( _SL("undef1+undef2+undef3") );
          UsedVar = p.GetUsedVar();
          iCount = (int)UsedVar.size();
          if (iCount!=3) 
            throw false;

          // the next check will fail if the parser 
          // erroneousely creates new variables internally
          if (p.GetVar().size()!=5)
            throw false;

          for (item = UsedVar.begin(); item!=UsedVar.end(); ++item)
          {
            if (item->second!=0) 
              throw false; // all pointers to undefined variables must be null
          }

          // 1 used variables
          p.SetExpr( _SL("a+b") );
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
          _OUT << _SL("passed") << endl;
        else
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;

        return iStat;
      }

      //---------------------------------------------------------------------------
      int TestMultiArg()
      {
        int iStat = 0;
        _OUT << _SL("testing multiarg functions...");
    
        // Compound expressions
        iStat += EqnTest( _SL("1,2,3"), 3, true);
        iStat += EqnTest( _SL("a,b,c"), 3, true);
        iStat += EqnTest( _SL("a=10,b=20,c=a*b"), 200, true);
        iStat += EqnTest( _SL("1,\n2,\n3"), 3, true);
        iStat += EqnTest( _SL("a,\nb,\nc"), 3, true);
        iStat += EqnTest( _SL("a=10,\nb=20,\nc=a*b"), 200, true);
        iStat += EqnTest( _SL("1,\r\n2,\r\n3"), 3, true);
        iStat += EqnTest( _SL("a,\r\nb,\r\nc"), 3, true);
        iStat += EqnTest( _SL("a=10,\r\nb=20,\r\nc=a*b"), 200, true);

        // picking the right argument
        iStat += EqnTest( _SL("f1of1(1)"), 1, true);
        iStat += EqnTest( _SL("f1of2(1, 2)"), 1, true);
        iStat += EqnTest( _SL("f2of2(1, 2)"), 2, true);
        // Too few arguments / Too many arguments
        iStat += EqnTest( _SL("1+ping()"), 11, true);
        iStat += EqnTest( _SL("ping()+1"), 11, true);
        iStat += EqnTest( _SL("2*ping()"), 20, true);
        iStat += EqnTest( _SL("ping()*2"), 20, true);
        iStat += EqnTest( _SL("ping(1,2)"), 0, false);
        iStat += EqnTest( _SL("1+ping(1,2)"), 0, false);
        iStat += EqnTest( _SL("f1of1(1,2)"), 0, false);
        iStat += EqnTest( _SL("f1of1()"), 0, false);
        iStat += EqnTest( _SL("f1of2(1, 2, 3)"), 0, false);
        iStat += EqnTest( _SL("f1of2(1)"), 0, false);
        iStat += EqnTest( _SL("(1,2,3)"), 0, false);
        iStat += EqnTest( _SL("1,2,3"), 0, false);
        iStat += EqnTest( _SL("(1*a,2,3)"), 0, false);
        iStat += EqnTest( _SL("1,2*a,3"), 0, false);
     
        // correct calculation of arguments
        iStat += EqnTest( _SL("min(a, 1)"),  1, true);
        iStat += EqnTest( _SL("min(3*2, 1)"),  1, true);
        iStat += EqnTest( _SL("min(3*2, 1)"),  6, false);
        iStat += EqnTest( _SL("min(3*a+1, 1)"),  1, true);
        iStat += EqnTest( _SL("max(3*a+1, 1)"),  4, true);
        iStat += EqnTest( _SL("max(3*a+1, 1)*2"),  8, true);
        iStat += EqnTest( _SL("2*max(3*a+1, 1)+2"),  10, true);

        // functions with Variable argument count
        iStat += EqnTest( _SL("sum(a)"), 1, true);
        iStat += EqnTest( _SL("sum(1,2,3)"),  6, true);
        iStat += EqnTest( _SL("sum(a,b,c)"),  6, true);
        iStat += EqnTest( _SL("sum(1,-max(1,2),3)*2"),  4, true);
        iStat += EqnTest( _SL("2*sum(1,2,3)"),  12, true);
        iStat += EqnTest( _SL("2*sum(1,2,3)+2"),  14, true);
        iStat += EqnTest( _SL("2*sum(-1,2,3)+2"),  10, true);
        iStat += EqnTest( _SL("2*sum(-1,2,-(-a))+2"),  6, true);
        iStat += EqnTest( _SL("2*sum(-1,10,-a)+2"),  18, true);
        iStat += EqnTest( _SL("2*sum(1,2,3)*2"),  24, true);
        iStat += EqnTest( _SL("sum(1,-max(1,2),3)*2"),  4, true);
        iStat += EqnTest( _SL("sum(1*3, 4, a+2)"),  10, true);
        iStat += EqnTest( _SL("sum(1*3, 2*sum(1,2,2), a+2)"),  16, true);
        iStat += EqnTest( _SL("sum(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2)"), 24, true);

        // some failures
        iStat += EqnTest( _SL("sum()"),  0, false);
        iStat += EqnTest( _SL("sum(,)"),  0, false);
        iStat += EqnTest( _SL("sum(1,2,)"),  0, false);
        iStat += EqnTest( _SL("sum(,1,2)"),  0, false);

        if (iStat==0) 
          _OUT << _SL("passed") << endl;
        else
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;
  
        return iStat;
      }


      //---------------------------------------------------------------------------
      int TestInfixOprt()
      {
        int iStat(0);
        _OUT << "testing infix operators...";

        iStat += EqnTest( _SL("-1"),    -1, true);
        iStat += EqnTest( _SL("-(-1)"),  1, true);
        iStat += EqnTest( _SL("-(-1)*2"),  2, true);
        iStat += EqnTest( _SL("-(-2)*sqrt(4)"),  4, true);
        iStat += EqnTest( _SL("-_pi"), -MathImpl<TValue, TString>::c_pi, true);
        iStat += EqnTest( _SL("-a"),  -1, true);
        iStat += EqnTest( _SL("-(a)"),  -1, true);
        iStat += EqnTest( _SL("-(-a)"),  1, true);
        iStat += EqnTest( _SL("-(-a)*2"),  2, true);
        iStat += EqnTest( _SL("-(8)"), -8, true);
        iStat += EqnTest( _SL("-8"), -8, true);
        iStat += EqnTest( _SL("-(2+1)"), -3, true);
        iStat += EqnTest( _SL("-(f1of1(1+2*3)+1*2)"), -9, true);
        iStat += EqnTest( _SL("-(-f1of1(1+2*3)+1*2)"), 5, true);
        iStat += EqnTest( _SL("-sin(8)"), (TValue)-0.989358, true);
        iStat += EqnTest( _SL("3-(-a)"), 4, true);
        iStat += EqnTest( _SL("3--a"), 4, true);
        iStat += EqnTest( _SL("-1*3"),  -3, true);

        // Postfix / infix priorities
        iStat += EqnTest( _SL("~2#"), 8, true);
        iStat += EqnTest( _SL("~f1of1(2)#"), 8, true);
        iStat += EqnTest( _SL("~(b)#"), 8, true);
        iStat += EqnTest( _SL("(~b)#"), 12, true);
        iStat += EqnTest( _SL("~(2#)"), 8, true);
        iStat += EqnTest( _SL("~(f1of1(2)#)"), 8, true);
        //
        iStat += EqnTest( _SL("-2^2"),-4, true);
        iStat += EqnTest( _SL("-(a+b)^2"),-9, true);
        iStat += EqnTest( _SL("(-3)^2"),9, true);
        iStat += EqnTest( _SL("-(-2^2)"),4, true);
        iStat += EqnTest( _SL("3+-3^2"),-6, true);
        // The following assumes use of sqr as postfix operator ("§") together
        // with a sign operator of low priority:
        iStat += EqnTest( _SL("-2'"), -4, true);
        iStat += EqnTest( _SL("-(1+1)'"),-4, true);
        iStat += EqnTest( _SL("2+-(1+1)'"),-2, true);
        iStat += EqnTest( _SL("2+-2'"), -2, true);
        // This is the classic behaviour of the infix sign operator (here: "$") which is
        // now deprecated:
        iStat += EqnTest( _SL("$2^2"),4, true);
        iStat += EqnTest( _SL("$(a+b)^2"),9, true);
        iStat += EqnTest( _SL("($3)^2"),9, true);
        iStat += EqnTest( _SL("$($2^2)"),-4, true);
        iStat += EqnTest( _SL("3+$3^2"),12, true);

        // infix operators sharing the first few characters
        iStat += EqnTest( _SL("~ 123"),  123+2, true);
        iStat += EqnTest( _SL("~~ 123"),  123+2, true);

        if (iStat==0)
          _OUT << _SL("passed") << endl;
        else
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;

        return iStat;
      }


      //---------------------------------------------------------------------------
      int TestPostFix()
      {
        int iStat = 0;
        _OUT << _SL("testing postfix operators...");

        // application
        iStat += EqnTest( _SL("3{m}+5"), (TValue)5.003, true);
        iStat += EqnTest( _SL("1000{m}"), 1, true);
        iStat += EqnTest( _SL("1000 {m}"), 1, true);
        iStat += EqnTest( _SL("(a){m}"), (TValue)1e-3, true);
        iStat += EqnTest( _SL("a{m}"), (TValue)1e-3, true);
        iStat += EqnTest( _SL("a {m}"), (TValue)1e-3, true);
        iStat += EqnTest( _SL("-(a){m}"), (TValue)-1e-3, true);
        iStat += EqnTest( _SL("-2{m}"), (TValue)-2e-3, true);
        iStat += EqnTest( _SL("-2 {m}"), (TValue)-2e-3, true);
        iStat += EqnTest( _SL("f1of1(1000){m}"), 1, true);
        iStat += EqnTest( _SL("-f1of1(1000){m}"), -1, true);
        iStat += EqnTest( _SL("-f1of1(-1000){m}"), 1, true);
        iStat += EqnTest( _SL("2+(a*1000){m}"), 3, true);

        // can postfix operators "m" und "meg" be told apart properly?
        iStat += EqnTest( _SL("2*3000meg+2"), (TValue)(2*3e9+2), true);   

        // some incorrect results
        iStat += EqnTest( _SL("1000{m}"), (TValue)0.1, false);
        iStat += EqnTest( _SL("(a){m}"), 2, false);

        // failure due to syntax checking
        iStat += ThrowTest( _SL("0x"), ecUNASSIGNABLE_TOKEN);  // incomplete hex definition
        iStat += ThrowTest( _SL("3+"), ecUNEXPECTED_EOF);
        iStat += ThrowTest( _SL("4 + {m}"), ecUNASSIGNABLE_TOKEN);
        iStat += ThrowTest( _SL("{m}4"), ecUNASSIGNABLE_TOKEN);
        iStat += ThrowTest( _SL("sin({m})"), ecUNASSIGNABLE_TOKEN);
        iStat += ThrowTest( _SL("{m} {m}"), ecUNASSIGNABLE_TOKEN);
        iStat += ThrowTest( _SL("{m}(8)"), ecUNASSIGNABLE_TOKEN);
        iStat += ThrowTest( _SL("4,{m}"), ecUNASSIGNABLE_TOKEN);
        iStat += ThrowTest( _SL("-{m}"), ecUNASSIGNABLE_TOKEN);
        iStat += ThrowTest( _SL("2(-{m})"), ecUNEXPECTED_PARENS);
        iStat += ThrowTest( _SL("2({m})"), ecUNEXPECTED_PARENS);
 
        iStat += ThrowTest( _SL("multi*1.0"), ecUNASSIGNABLE_TOKEN);

        if (iStat==0)
          _OUT << _SL("passed") << endl;
        else
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;

        return iStat;
      }

      //---------------------------------------------------------------------------
      int TestExpression()
      {
        int iStat = 0;
        _OUT << _SL("testing expression samples...");

        TValue b = 2;

        iStat += EqnTest(_SL("1 - ((a * b) + (a / b)) - 3"), (TValue)-4.5, true);  // was an actual bug... (Result was -0.5)
        
        // Optimization
        iStat += EqnTest(_SL("1-b-3"), (TValue)-4, true);
        iStat += EqnTest( _SL("2*b*5"), 20, true);
        iStat += EqnTest( _SL("2*b*5 + 4*b"), 28, true);
        iStat += EqnTest( _SL("2*a/3"), (TValue)2.0/(TValue)3.0, true);

        // Addition auf cmVARMUL 
        iStat += EqnTest( _SL("3+b"), b+3, true);
        iStat += EqnTest( _SL("b+3"), b+3, true);
        iStat += EqnTest( _SL("b*3+2"), b*3+2, true);
        iStat += EqnTest( _SL("3*b+2"), b*3+2, true);
        iStat += EqnTest( _SL("2+b*3"), b*3+2, true);
        iStat += EqnTest( _SL("2+3*b"), b*3+2, true);
        iStat += EqnTest( _SL("b+3*b"), b+3*b, true);
        iStat += EqnTest( _SL("3*b+b"), b+3*b, true);

        iStat += EqnTest( _SL("2+b*3+b"), 2+b*3+b, true);
        iStat += EqnTest( _SL("b+2+b*3"), b+2+b*3, true);

        iStat += EqnTest( _SL("(2*b+1)*4"), (2*b+1)*4, true);
        iStat += EqnTest( _SL("4*(2*b+1)"), (2*b+1)*4, true);

        // operator precedencs
        iStat += EqnTest( _SL("1+2-3*4/5^6"), (TValue)2.99923, true);
        iStat += EqnTest( _SL("1^2/3*4-5+6"), (TValue)2.3333, true);
        iStat += EqnTest( _SL("1+2*3"), 7, true);
        iStat += EqnTest( _SL("1+2*3"), 7, true);
        iStat += EqnTest( _SL("(1+2)*3"), 9, true);
        iStat += EqnTest( _SL("(1+2)*(-3)"), -9, true);
        iStat += EqnTest( _SL("2/4"), 0.5, true);

        iStat += EqnTest( _SL("exp(ln(7))"), 7, true);
        iStat += EqnTest( _SL("e^ln(7)"), 7, true);
        iStat += EqnTest( _SL("e^(ln(7))"), 7, true);
        iStat += EqnTest( _SL("(e^(ln(7)))"), 7, true);
        iStat += EqnTest( _SL("1-(e^(ln(7)))"), -6, true);
        iStat += EqnTest( _SL("2*(e^(ln(7)))"), 14, true);
        iStat += EqnTest( _SL("10^log10(5)"), 5, true);
        iStat += EqnTest( _SL("2^log2(4)"), 4, true);
        iStat += EqnTest( _SL("-(sin(0)+1)"), -1, true);
        iStat += EqnTest( _SL("-(2^1.1)"), (TValue)-2.14354692, true);

        iStat += EqnTest( _SL("(cos(2.41)/b)"), (TValue)-0.372056, true);
        iStat += EqnTest( _SL("(1*(2*(3*(4*(5*(6*(a+b)))))))"), 2160, true);
        iStat += EqnTest( _SL("(1*(2*(3*(4*(5*(6*(7*(a+b))))))))"), 15120, true);
        iStat += EqnTest( _SL("(a/((((b+(((e*(((((pi*((((3.45*((pi+a)+pi))+b)+b)*a))+0.68)+e)+a)/a))+a)+b))+b)*a)-pi))"), (TValue)0.00377999, true);

        // long formula (Reference: Matlab)
        iStat += EqnTest(
          TString(_SL("(((-9))-e/(((((((pi-(((-7)+(-3)/4/e))))/(((-5))-2)-((pi+(-0))*(sqrt((e+e))*(-8))*(((-pi)+(-pi)-(-9)*(6*5))")) +
                  _SL("/(-e)-e))/2)/((((sqrt(2/(-e)+6)-(4-2))+((5/(-2))/(1*(-pi)+3))/8)*pi*((pi/((-2)/(-6)*1*(-1))*(-6)+(-e)))))/") +
                  _SL("((e+(-2)+(-e)*((((-3)*9+(-e)))+(-9)))))))-((((e-7+(((5/pi-(3/1+pi)))))/e)/(-5))/(sqrt((((((1+(-7))))+((((-") +
                  _SL("e)*(-e)))-8))*(-5)/((-e)))*(-6)-((((((-2)-(-9)-(-e)-1)/3))))/(sqrt((8+(e-((-6))+(9*(-9))))*(((3+2-8))*(7+6") +
                  _SL("+(-5))+((0/(-e)*(-pi))+7)))+(((((-e)/e/e)+((-6)*5)*e+(3+(-5)/pi))))+pi))/sqrt((((9))+((((pi))-8+2))+pi))/e") +
                  _SL("*4)*((-5)/(((-pi))*(sqrt(e)))))-(((((((-e)*(e)-pi))/4+(pi)*(-9)))))))+(-pi)"), (TValue)-12.23016549, true);

        // long formula (Reference: Matlab)
        iStat += EqnTest(
            TString(_SL("(atan(sin((((((((((((((((pi/cos((a/((((0.53-b)-pi)*e)/b))))+2.51)+a)-0.54)/0.98)+b)*b)+e)/a)+b)+a)+b)+pi)/e")) + 
                    _SL(")+a)))*2.77)"), (TValue)-2.16995656, true);


        // long formula (Reference: Matlab)
        iStat += EqnTest( _SL("1+2-3*4/5^6*(2*(1-5+(3*7^9)*(4+6*7-3)))+12"), (TValue)-7995810.09926, true);
	  
        iStat += EqnTest( _SL("(b+1)*(b+2)*(b+3)*(b+4)*(b+5)*(b+6)*(b+7)*(b+8)*(b+9)*(b+10)*(b+11)*(b+12)"), (b+1)*(b+2)*(b+3)*(b+4)*(b+5)*(b+6)*(b+7)*(b+8)*(b+9)*(b+10)*(b+11)*(b+12), true);
 
        if (iStat==0) 
          _OUT << _SL("passed") << endl;  
        else 
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;

        return iStat;
      }



      //---------------------------------------------------------------------------
      int TestIfThenElse()
      {
        int iStat = 0;
        _OUT << _SL("testing if-then-else operator...");

        // Test error detection
        iStat += ThrowTest(_SL(":3"), ecUNEXPECTED_CONDITIONAL); 
        iStat += ThrowTest(_SL("? 1 : 2"), ecUNEXPECTED_CONDITIONAL); 
        iStat += ThrowTest(_SL("(a<b) ? (b<c) ? 1 : 2"), ecMISSING_ELSE_CLAUSE); 
        iStat += ThrowTest(_SL("(a<b) ? 1"), ecMISSING_ELSE_CLAUSE); 
        iStat += ThrowTest(_SL("(a<b) ? a"), ecMISSING_ELSE_CLAUSE); 
        iStat += ThrowTest(_SL("(a<b) ? a+b"), ecMISSING_ELSE_CLAUSE); 
        iStat += ThrowTest(_SL("a : b"), ecMISPLACED_COLON); 
        iStat += ThrowTest(_SL("1 : 2"), ecMISPLACED_COLON); 
        iStat += ThrowTest(_SL("(1) ? 1 : 2 : 3"), ecMISPLACED_COLON); 
        iStat += ThrowTest(_SL("(true) ? 1 : 2 : 3"), ecUNASSIGNABLE_TOKEN); 

        iStat += EqnTest(_SL("1 ? 128 : 255"), 128, true);
        iStat += EqnTest(_SL("1<2 ? 128 : 255"), 128, true);
        iStat += EqnTest(_SL("a<b ? 128 : 255"), 128, true);
        iStat += EqnTest(_SL("(a<b) ? 128 : 255"), 128, true);
        iStat += EqnTest(_SL("(1) ? 10 : 11"), 10, true);
        iStat += EqnTest(_SL("(0) ? 10 : 11"), 11, true);
        iStat += EqnTest(_SL("(1) ? a+b : c+d"), 3, true);
        iStat += EqnTest(_SL("(0) ? a+b : c+d"), 1, true);
        iStat += EqnTest(_SL("(1) ? 0 : 1"), 0, true);
        iStat += EqnTest(_SL("(0) ? 0 : 1"), 1, true);
        iStat += EqnTest(_SL("(a<b) ? 10 : 11"), 10, true);
        iStat += EqnTest(_SL("(a>b) ? 10 : 11"), 11, true);
        iStat += EqnTest(_SL("(a<b) ? c : d"), 3, true);
        iStat += EqnTest(_SL("(a>b) ? c : d"), -2, true);

        iStat += EqnTest(_SL("(a>b) ? 1 : 0"), 0, true);
        iStat += EqnTest(_SL("((a>b) ? 1 : 0) ? 1 : 2"), 2, true);
        iStat += EqnTest(_SL("((a>b) ? 1 : 0) ? 1 : sum((a>b) ? 1 : 2)"), 2, true);
        iStat += EqnTest(_SL("((a>b) ? 0 : 1) ? 1 : sum((a>b) ? 1 : 2)"), 1, true);

        iStat += EqnTest(_SL("sum((a>b) ? 1 : 2)"), 2, true);
        iStat += EqnTest(_SL("sum((1) ? 1 : 2)"), 1, true);
        iStat += EqnTest(_SL("sum((a>b) ? 1 : 2, 100)"), 102, true);
        iStat += EqnTest(_SL("sum((1) ? 1 : 2, 100)"), 101, true);
        iStat += EqnTest(_SL("sum(3, (a>b) ? 3 : 10)"), 13, true);
        iStat += EqnTest(_SL("sum(3, (a<b) ? 3 : 10)"), 6, true);
        iStat += EqnTest(_SL("10*sum(3, (a>b) ? 3 : 10)"), 130, true);
        iStat += EqnTest(_SL("10*sum(3, (a<b) ? 3 : 10)"), 60, true);
        iStat += EqnTest(_SL("sum(3, (a>b) ? 3 : 10)*10"), 130, true);
        iStat += EqnTest(_SL("sum(3, (a<b) ? 3 : 10)*10"), 60, true);
        iStat += EqnTest(_SL("(a<b) ? sum(3, (a<b) ? 3 : 10)*10 : 99"), 60, true);
        iStat += EqnTest(_SL("(a>b) ? sum(3, (a<b) ? 3 : 10)*10 : 99"), 99, true);
        iStat += EqnTest(_SL("(a<b) ? sum(3, (a<b) ? 3 : 10,10,20)*10 : 99"), 360, true);
        iStat += EqnTest(_SL("(a>b) ? sum(3, (a<b) ? 3 : 10,10,20)*10 : 99"), 99, true);
        iStat += EqnTest(_SL("(a>b) ? sum(3, (a<b) ? 3 : 10,10,20)*10 : sum(3, (a<b) ? 3 : 10)*10"), 60, true);

        // todo: auch für muParserX hinzufügen!
        iStat += EqnTest(_SL("(a<b)&&(a<b) ? 128 : 255"), 128, true);
        iStat += EqnTest(_SL("(a>b)&&(a<b) ? 128 : 255"), 255, true);
        iStat += EqnTest(_SL("(1<2)&&(1<2) ? 128 : 255"), 128, true);
        iStat += EqnTest(_SL("(1>2)&&(1<2) ? 128 : 255"), 255, true);
        iStat += EqnTest(_SL("((1<2)&&(1<2)) ? 128 : 255"), 128, true);
        iStat += EqnTest(_SL("((1>2)&&(1<2)) ? 128 : 255"), 255, true);
        iStat += EqnTest(_SL("((a<b)&&(a<b)) ? 128 : 255"), 128, true);
        iStat += EqnTest(_SL("((a>b)&&(a<b)) ? 128 : 255"), 255, true);

        iStat += EqnTest(_SL("1>0 ? 1>2 ? 128 : 255 : 1>0 ? 32 : 64"), 255, true);
        iStat += EqnTest(_SL("1>0 ? 1>2 ? 128 : 255 :(1>0 ? 32 : 64)"), 255, true);
        iStat += EqnTest(_SL("1>0 ? 1>0 ? 128 : 255 : 1>2 ? 32 : 64"), 128, true);
        iStat += EqnTest(_SL("1>0 ? 1>0 ? 128 : 255 :(1>2 ? 32 : 64)"), 128, true);
        iStat += EqnTest(_SL("1>2 ? 1>2 ? 128 : 255 : 1>0 ? 32 : 64"), 32, true);
        iStat += EqnTest(_SL("1>2 ? 1>0 ? 128 : 255 : 1>2 ? 32 : 64"), 64, true);
        iStat += EqnTest(_SL("1>0 ? 50 :  1>0 ? 128 : 255"), 50, true);
        iStat += EqnTest(_SL("1>0 ? 50 : (1>0 ? 128 : 255)"), 50, true);
        iStat += EqnTest(_SL("1>0 ? 1>0 ? 128 : 255 : 50"), 128, true);
        iStat += EqnTest(_SL("1>2 ? 1>2 ? 128 : 255 : 1>0 ? 32 : 1>2 ? 64 : 16"), 32, true);
        iStat += EqnTest(_SL("1>2 ? 1>2 ? 128 : 255 : 1>0 ? 32 :(1>2 ? 64 : 16)"), 32, true);
        iStat += EqnTest(_SL("1>0 ? 1>2 ? 128 : 255 :  1>0 ? 32 :1>2 ? 64 : 16"), 255, true);
        iStat += EqnTest(_SL("1>0 ? 1>2 ? 128 : 255 : (1>0 ? 32 :1>2 ? 64 : 16)"), 255, true);
        iStat += EqnTest(_SL("1 ? 0 ? 128 : 255 : 1 ? 32 : 64"), 255, true);

        // assignment operators
        iStat += EqnTest(_SL("a= 0 ? 128 : 255, a"), 255, true);
        iStat += EqnTest(_SL("a=((a>b)&&(a<b)) ? 128 : 255, a"), 255, true);
        iStat += EqnTest(_SL("c=(a<b)&&(a<b) ? 128 : 255, c"), 128, true);
        iStat += EqnTest(_SL("0 ? a=a+1 : 666, a"), 1, true);
        iStat += EqnTest(_SL("1?a=10:a=20, a"), 10, true);
        iStat += EqnTest(_SL("0?a=10:a=20, a"), 20, true);
        iStat += EqnTest(_SL("0?a=sum(3,4):10, a"), 1, true);  // a should not change its value due to lazy calculation
      
        iStat += EqnTest(_SL("a=1?b=1?3:4:5, a"), 3, true);
        iStat += EqnTest(_SL("a=1?b=1?3:4:5, b"), 3, true);
        iStat += EqnTest(_SL("a=0?b=1?3:4:5, a"), 5, true);
        iStat += EqnTest(_SL("a=0?b=1?3:4:5, b"), 2, true);

        iStat += EqnTest(_SL("a=1?5:b=1?3:4, a"), 5, true);
        iStat += EqnTest(_SL("a=1?5:b=1?3:4, b"), 2, true);
        iStat += EqnTest(_SL("a=0?5:b=1?3:4, a"), 3, true);
        iStat += EqnTest(_SL("a=0?5:b=1?3:4, b"), 3, true);

        if (iStat==0) 
          _OUT << _SL("passed") << endl;  
        else 
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;

        return iStat;
      }

      //---------------------------------------------------------------------------
      int TestException()
      {
        int  iStat = 0;
        _OUT << _SL("testing error codes...");

        iStat += ThrowTest(_SL("3+"),           ecUNEXPECTED_EOF);
        iStat += ThrowTest(_SL("3+)"),          ecUNEXPECTED_PARENS);
        iStat += ThrowTest(_SL("()"),           ecUNEXPECTED_PARENS);
        iStat += ThrowTest(_SL("3+()"),         ecUNEXPECTED_PARENS);
        iStat += ThrowTest(_SL("sin(3,4)"),     ecTOO_MANY_PARAMS);
        iStat += ThrowTest(_SL("sin()"),        ecTOO_FEW_PARAMS);
        iStat += ThrowTest(_SL("(1+2"),         ecMISSING_PARENS);
        iStat += ThrowTest(_SL("sin(3)3"),      ecUNEXPECTED_VAL);
        iStat += ThrowTest(_SL("sin(3)xyz"),    ecUNASSIGNABLE_TOKEN);
        iStat += ThrowTest(_SL("sin(3)cos(3)"), ecUNEXPECTED_FUN);
        iStat += ThrowTest(_SL("a+b+c=10"),     ecUNEXPECTED_OPERATOR);
        iStat += ThrowTest(_SL("a=b=3"),        ecUNEXPECTED_OPERATOR);

        // functions without parameter
        iStat += ThrowTest(_SL("3+ping(2)"),    ecTOO_MANY_PARAMS);
        iStat += ThrowTest(_SL("3+ping(a+2)"),  ecTOO_MANY_PARAMS);
        iStat += ThrowTest(_SL("3+ping(sin(a)+2)"),  ecTOO_MANY_PARAMS);
        iStat += ThrowTest(_SL("3+ping(1+sin(a))"),  ecTOO_MANY_PARAMS);

        // assignement operator
        iStat += ThrowTest(_SL("3=4"), ecUNEXPECTED_OPERATOR);
        iStat += ThrowTest(_SL("sin(8)=4"), ecUNEXPECTED_OPERATOR);
        iStat += ThrowTest(_SL("(8)=5"), ecUNEXPECTED_OPERATOR);
        iStat += ThrowTest(_SL("(a)=5"), ecUNEXPECTED_OPERATOR);

        if (iStat==0) 
          _OUT << _SL("passed") << endl;
        else 
          _OUT << _SL("\n  failed with ") << iStat << _SL(" errors") << endl;

        return iStat;
      }

    public:

      typedef int (ParserTester<TValue, TString>::*testfun_typeype)();

      //---------------------------------------------------------------------------------------------
      ParserTester()
        :m_vTestFun()
      {
        AddTest(&ParserTester<TValue, TString>::TestSyntax);
        AddTest(&ParserTester<TValue, TString>::TestPostFix);
        AddTest(&ParserTester<TValue, TString>::TestInfixOprt);
        AddTest(&ParserTester<TValue, TString>::TestVarConst);
        AddTest(&ParserTester<TValue, TString>::TestMultiArg);
        AddTest(&ParserTester<TValue, TString>::TestExpression);
        AddTest(&ParserTester<TValue, TString>::TestIfThenElse);
        AddTest(&ParserTester<TValue, TString>::TestInterface);
        AddTest(&ParserTester<TValue, TString>::TestBinOprt);
        AddTest(&ParserTester<TValue, TString>::TestOptimizer);
        AddTest(&ParserTester<TValue, TString>::TestException);

        ParserTester<TValue, TString>::c_iCount = 0;
      }

      //-------------------------------------------------------------------------------------------
      void Run()
      {
        int iStat = 0;
        try
        {
          cout << "Running test suite (value type:" 
               << typeid(TValue).name() << "; char type:" 
               << typeid(typename TString::value_type).name() << ")\n";

          for (int i=0; i<(int)m_vTestFun.size(); ++i)
            iStat += (this->*m_vTestFun[i])();
        }
        catch(ParserError<TString> &e)
        {
          _OUT << _SL("\n") << e.GetMsg() << endl;
          _OUT << e.GetToken() << endl;
          Abort();
        }
        catch(std::exception &e)
        {
          _OUT << e.what() << endl;
          Abort();
        }
        catch(...)
        {
          _OUT << "Internal error";
          Abort();
        }

        if (iStat==0) 
        {
          _OUT << "Test passed (" <<  ParserTester::c_iCount << " expressions)" << endl;
        }
        else 
        {
          _OUT << "Test failed with " << iStat 
                    << " errors (" <<  ParserTester::c_iCount 
                    << " expressions)" << endl;
        }
        ParserTester::c_iCount = 0;
      }

    private:

      std::vector<testfun_typeype> m_vTestFun;

      //-------------------------------------------------------------------------------------------
      void AddTest(testfun_typeype a_pFun)
      {
        m_vTestFun.push_back(a_pFun);
      }

      //-------------------------------------------------------------------------------------------
      int ThrowTest(const TString& a_str, int a_iErrc, bool a_bFail = true)
      {
        ParserTester<TValue, TString>::c_iCount++;

        try
        {
          TValue fVal[] = {1,1,1};
          Parser<TValue, TString> p;

          p.DefineVar( _SL("a"), &fVal[0]);
          p.DefineVar( _SL("b"), &fVal[1]);
          p.DefineVar( _SL("c"), &fVal[2]);
          p.DefinePostfixOprt( _SL("{m}"), Milli);
          p.DefinePostfixOprt( _SL("m"), Milli);
          p.DefineFun( _SL("ping"), Ping, 0);
          p.SetExpr(a_str);
          p.Eval();
        }
        catch(ParserError<TString> &e)
        {
          // output the formula in case of an failed test
          if (a_bFail==false || (a_bFail==true && a_iErrc!=e.GetCode()) )
          {
            _OUT << _SL("\n  ") 
                          << _SL("Expression: ") << a_str 
                          << _SL("  Code:") << e.GetCode() << _SL("(") << e.GetMsg() << _SL(")")
                          << _SL("  Expected:") << a_iErrc;
          }

          return (a_iErrc==e.GetCode()) ? 0 : 1;
        }

        // if a_bFail==false no exception is expected
        bool bRet((a_bFail==false) ? 0 : 1);
        if (bRet==1)
        {
          _OUT << _SL("\n  ") 
                        << _SL("Expression: ") << a_str 
                        << _SL("  did evaluate; Expected error:") << a_iErrc;
        }

        return bRet; 
      }

      //---------------------------------------------------------------------------------------------
      /** \brief Evaluate a tet expression. 

          \return 1 in case of a failure, 0 otherwise.
      */
      int EqnTestWithVarChange(const TString &a_str, 
                               TValue a_fVar1, 
                               TValue a_fRes1, 
                               TValue a_fVar2, 
                               TValue a_fRes2)
      {
        ParserTester::c_iCount++;
        TValue fVal[2] = {-999, -999 }; // should be equalinitially

        try
        {
          Parser<TValue, TString>  p;

          // variable
          TValue var = 0;
          p.DefineVar( _SL("a"), &var);
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
        catch(ParserError<TString> &e)
        {
          _OUT << _SL("\n  fail: ") << a_str.c_str() << _SL(" (") << e.GetMsg() << _SL(")");
          return 1;
        }
        catch(std::exception &e)
        {
          _OUT << _SL("\n  fail: ") << a_str.c_str() << _SL(" (") << e.what() << _SL(")");
          return 1;  // always return a failure since this exception is not expected
        }
        catch(...)
        {
          _OUT << _SL("\n  fail: ") << a_str.c_str() <<  _SL(" (unexpected exception)");
          return 1;  // exceptions other than ParserException are not allowed
        }

        return 0;
      }

      //---------------------------------------------------------------------------
      /** \brief Evaluate a tet expression. 

          \return 1 in case of a failure, 0 otherwise.
      */
      int EqnTest(const TString &a_str, TValue a_fRes, bool a_fPass)
      {
        ParserTester<TValue, TString>::c_iCount++;
        int iRet(0);
        TValue fVal[5] = {-999, -998, -997, -996, -995}; // initially should be different

        try
        {
          std::unique_ptr< Parser<TValue, TString> > p1;
          Parser<TValue, TString>  p2, p3; // three parser objects
                                           // they will be used for testing copy and assihnment operators
          // p1 is a pointer since i'm going to delete it in order to test if
          // parsers after copy construction still refer to members of it.
          // !! If this is the case this function will crash !!
      
          p1.reset(new Parser<TValue, TString>()); 

          // Add constants
          p1->DefineConst(_SL("pi"), MathImpl<TValue, TString>::c_pi);
          p1->DefineConst(_SL("e"),  MathImpl<TValue, TString>::c_e);
          p1->DefineConst(_SL("const"), 1);
          p1->DefineConst(_SL("const1"), 2);
          p1->DefineConst(_SL("const2"), 3);
          
          // variables
          TValue vVarVal[] = { 1, 2, 3, -2};
          p1->DefineVar(_SL("a"),  &vVarVal[0]);
          p1->DefineVar(_SL("aa"), &vVarVal[1]);
          p1->DefineVar(_SL("b"),  &vVarVal[1]);
          p1->DefineVar(_SL("c"),  &vVarVal[2]);
          p1->DefineVar(_SL("d"),  &vVarVal[3]);
        
          // custom value ident functions
          p1->AddValIdent(&ParserTester<TValue, TString>::IsHexVal);        

          // functions
          p1->DefineFun(_SL("ping"),  Ping, 0);
          p1->DefineFun(_SL("f1of1"), FirstArg, 1);
          p1->DefineFun(_SL("f1of2"), FirstArg, 2);
          p1->DefineFun(_SL("f2of2"), arg2, 2);

          // binary operators
          p1->DefineOprt(_SL("add"), MathImpl<TValue, TString>::Add, 0);
          p1->DefineOprt(_SL("++"),  MathImpl<TValue, TString>::Add, 0);
          p1->DefineOprt(_SL("&"), land, prLAND);

          // sample functions
          p1->DefineFun(_SL("min"), Min, 2);
          p1->DefineFun(_SL("max"), Max, 2);

          // infix / postfix operator
          // Note: Identifiers used here do not have any meaning 
          //       they are mere placeholders to test certain features.
          p1->DefineInfixOprt(_SL("$"), MathImpl<TValue, TString>::UnaryMinus, prPOW+1);  // sign with high priority
          p1->DefineInfixOprt(_SL("~"), plus2);          // high priority
          p1->DefineInfixOprt(_SL("~~"), plus2);
          p1->DefinePostfixOprt(_SL("{m}"), Milli);
          p1->DefinePostfixOprt(_SL("{M}"), Mega);
          p1->DefinePostfixOprt(_SL("m"), Milli);
          p1->DefinePostfixOprt(_SL("meg"), Mega);
          p1->DefinePostfixOprt(_SL("#"), times3);
          p1->DefinePostfixOprt(_SL("'"), sqr); 
          p1->SetExpr(a_str);

          // Test bytecode integrity
          // String parsing and bytecode parsing must yield the same result
          fVal[0] = p1->Eval(); // result from stringparsing
          fVal[1] = p1->Eval(); // result from bytecode
          if (fVal[0]!=fVal[1])
            throw ParserError<TString>(_SL("Bytecode / string parsing mismatch."));

          // Test copy and assignement operators
          try
          {
            // Test copy constructor
            std::vector<Parser<TValue, TString>> vParser;
            vParser.push_back(*(p1.get()));
            Parser<TValue, TString> p2 = vParser[0];   // take parser from vector
        
            // destroy the originals from p2
            vParser.clear();              // delete the vector
            p1.reset(0);

            fVal[2] = p2.Eval();

            // Test assignement operator
            // additionally  disable Optimizer this time
            Parser<TValue, TString> p3;
            p3 = p2;
            fVal[3] = p3.Eval();

            // Test Eval function for multiple return values
            // use p2 since it has the optimizer enabled!
            int nNum;
            TValue *v = p2.Eval(nNum);
            fVal[4] = v[nNum-1];
          }
          catch(std::exception &e)
          {
            _OUT << _SL("\n  ") << e.what() << _SL("\n");
          }

          // limited floating point accuracy requires the following test
          bool bCloseEnough(true);
          for (unsigned i=0; i<sizeof(fVal)/sizeof(TValue); ++i)
          {
            bCloseEnough &= (fabs(a_fRes-fVal[i]) <= fabs(fVal[i]*0.0001));

            // The tests equations never result in infinity, if they do thats a bug.
            // reference:
            // http://sourceforge.net/projects/muparser/forums/forum/462843/topic/5037825
            if (numeric_limits<TValue>::has_infinity)
              bCloseEnough &= (fabs(fVal[i]) != numeric_limits<TValue>::infinity());
          }

          iRet = ((bCloseEnough && a_fPass) || (!bCloseEnough && !a_fPass)) ? 0 : 1;
          if (iRet==1)
          {
            _OUT << _SL("\n  fail: ") << a_str.c_str() 
                          << _SL(" (incorrect result; expected: ") << a_fRes
                          << _SL(" ;calculated: ") << fVal[0] << _SL(",") 
                                                   << fVal[1] << _SL(",")
                                                   << fVal[2] << _SL(",")
                                                   << fVal[3] << _SL(",")
                                                   << fVal[4] << _SL(").");
          }
        }
        catch(ParserError<TString> &e)
        {
          if (a_fPass)
          {
            if (fVal[0]!=fVal[2] && fVal[0]!=-999 && fVal[1]!=-998)
              _OUT << _SL("\n  fail: ") << a_str.c_str() << _SL(" (copy construction)");
            else
              _OUT << _SL("\n  fail: ") << a_str.c_str() << _SL(" (") << e.GetMsg() << _SL(")");
            return 1;
          }
        }
        catch(std::exception &e)
        {
          _OUT << _SL("\n  fail: ") << a_str.c_str() << _SL(" (") << e.what() << _SL(")");
          return 1;  // always return a failure since this exception is not expected
        }
        catch(...)
        {
          _OUT << _SL("\n  fail: ") << a_str.c_str() <<  _SL(" (unexpected exception)");
          return 1;  // exceptions other than ParserException are not allowed
        }

        return iRet;
      }

      //---------------------------------------------------------------------------
      /** \brief Internal error in test class Test is going to be aborted. */
      void Abort() const
      {
        _OUT << _SL("Test failed (internal error in test class)") << endl;
        while (!getchar());
        exit(-1);
      }
    };

    template<typename TValue, typename TString>
    int ParserTester<TValue, TString>::c_iCount = 0;
  } // namespace Test
} // namespace mu

#endif



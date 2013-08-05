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

#ifndef MEC_UNIT_TEST_H
#define MEC_UNIT_TEST_H

#include <string>
#include <cstdlib>
#include <numeric> // for accumulate
#include "mecParser.h"

/** \file
    \brief This file contains the parser test class.
*/

namespace mec
{
  /** \brief Namespace for test cases. */
  namespace Test
  {
    //------------------------------------------------------------------------------
    /** \brief Test cases for unit testing.
    */
    class UnitTest // final
    {
    private:
        // Multiarg callbacks
        static value_type f1of1(value_type v) { return v;};
      	
        static value_type f1of2(value_type v, value_type  ) {return v;};
        static value_type f2of2(value_type  , value_type v) {return v;};

        static value_type f1of3(value_type v, value_type  , value_type  ) {return v;};
        static value_type f2of3(value_type  , value_type v, value_type  ) {return v;};
        static value_type f3of3(value_type  , value_type  , value_type v) {return v;};
      	
        static value_type f1of4(value_type v, value_type,   value_type  , value_type  ) {return v;}
        static value_type f2of4(value_type  , value_type v, value_type  , value_type  ) {return v;}
        static value_type f3of4(value_type  , value_type,   value_type v, value_type  ) {return v;}
        static value_type f4of4(value_type  , value_type,   value_type  , value_type v) {return v;}

	      static value_type f1of5(value_type v, value_type,   value_type  , value_type  , value_type  ) { return v; }
	      static value_type f2of5(value_type  , value_type v, value_type  , value_type  , value_type  ) { return v; }
	      static value_type f3of5(value_type  , value_type,   value_type v, value_type  , value_type  ) { return v; }
	      static value_type f4of5(value_type  , value_type,   value_type  , value_type v, value_type  ) { return v; }
	      static value_type f5of5(value_type  , value_type,   value_type  , value_type  , value_type v) { return v; }

        static value_type Min(value_type a_fVal1, value_type a_fVal2) { return (a_fVal1<a_fVal2) ? a_fVal1 : a_fVal2; }
  	    static value_type Max(value_type a_fVal1, value_type a_fVal2) { return (a_fVal1>a_fVal2) ? a_fVal1 : a_fVal2; }

        static value_type plus2(value_type v1) { return v1+2; }
        static value_type times3(value_type v1) { return v1*3; }
        static value_type sqr(value_type v1) { return v1*v1; }
        static value_type sign(value_type v) { return -v; }
        static value_type add(value_type v1, value_type v2) { return v1+v2; }
        static value_type and(value_type v1, value_type v2) { return (value_type)((int)v1 & (int)v2); }
        
        static value_type Rnd(value_type v)
        {
          return (value_type)(1+(v*std::rand()/(RAND_MAX+1.0)));
        }

        static value_type Ping()
        { 
          return 10; 
        }

        static int c_iCount;

	      int TestNames();
	      int TestSyntax();
	      int TestPostFix();
        int TestLogic();
	      int TestExpression();
	      int TestInfixOprt();
	      int TestBinOprt();
	      int TestVarConst();
	      int TestInterface();
	      int TestException();
        int TestMultiArg();
        int TestIfThenElse();

        void Abort() const;

    public:
        typedef int (UnitTest::*testfun_type)();

	      UnitTest();
	      void Run();

    private:
        std::vector<testfun_type> m_vTestFun;
	      void AddTest(testfun_type a_pFun);

        // Test Double Parser
        int EqnTest(const string_type& a_str, value_type a_fRes, bool a_fPass);
        int ThrowTest(const string_type& a_str, int a_iErrc, bool a_bFail = true);
    };
  } // namespace Test
} // namespace mec

#endif
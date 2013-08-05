#ifndef MU_PARSER_TEMPLATE_MAGIC_H
#define MU_PARSER_TEMPLATE_MAGIC_H

//--- Standard includes ---------------------------------------------------------------------------
#include <cmath>

//--- muparser framework --------------------------------------------------------------------------
#include "muParserDef.h"


MUP_NAMESPACE_START

    //---------------------------------------------------------------------------------------------
    /** \brief A template class for providing wrappers for essential math functions.

      This template is spezialized for several types in order to provide a unified interface
      for parser internal math function calls regardless of the data type.
    */
    template<typename T, typename TString>
    struct MathImpl
    {
      // Constants
      static const T c_e;
      static const T c_pi;

      // basic arithmetic operations
      static void MUP_FASTCALL Add(T *arg, int /*argc*/) { *arg += arg[1]; }
      static void MUP_FASTCALL Sub(T *arg, int /*argc*/) { *arg -= arg[1]; }
      static void MUP_FASTCALL Mul(T *arg, int /*argc*/) { *arg *= arg[1]; }
      static void MUP_FASTCALL Div(T *arg, int /*argc*/) { *arg /= arg[1]; }
      static void MUP_FASTCALL Pow(T *arg, int /*argc*/)   
      { 
        T v2 = arg[1]; 

        int v2i = (int)v2;
        if (v2==v2i)
          arg[0] = std::pow(arg[0], v2i);
        else
          arg[0] = std::pow(arg[0], v2); 
      }

      static void MUP_FASTCALL And(T *arg,       int) { *arg = *arg && arg[1]; }
      static void MUP_FASTCALL Or(T *arg,        int) { *arg = *arg || arg[1]; }
      static void MUP_FASTCALL Less(T *arg,      int) { *arg = *arg <  arg[1]; }
      static void MUP_FASTCALL Greater(T *arg,   int) { *arg = *arg >  arg[1]; }
      static void MUP_FASTCALL LessEq(T *arg,    int) { *arg = *arg <= arg[1]; }
      static void MUP_FASTCALL GreaterEq(T *arg, int) { *arg = *arg >= arg[1]; }
      static void MUP_FASTCALL Equal(T *arg,     int) { *arg = *arg == arg[1]; }
      static void MUP_FASTCALL NotEqual(T *arg,  int) { *arg = *arg != arg[1]; }

      // 
      static void MUP_FASTCALL Sin(T *arg,   int) { *arg = sin(*arg);  }
      static void MUP_FASTCALL Cos(T *arg,   int) { *arg = cos(*arg);  }
      static void MUP_FASTCALL Tan(T *arg,   int) { *arg = tan(*arg);  }
      static void MUP_FASTCALL ASin(T *arg,  int) { *arg = asin(*arg); }
      static void MUP_FASTCALL ACos(T *arg,  int) { *arg = acos(*arg); }
      static void MUP_FASTCALL ATan(T *arg,  int) { *arg = atan(*arg); }
      static void MUP_FASTCALL ATan2(T *arg, int) { *arg = atan2(*arg, arg[1]); }
      static void MUP_FASTCALL Sinh(T *arg,  int) { *arg = sinh(*arg); }
      static void MUP_FASTCALL Cosh(T *arg,  int) { *arg = cosh(*arg); }
      static void MUP_FASTCALL Tanh(T *arg,  int) { *arg = tanh(*arg); }
      static void MUP_FASTCALL ASinh(T *arg, int) { T &v = *arg; *arg = log(v + sqrt(v * v + 1)); }
      static void MUP_FASTCALL ACosh(T *arg, int) { T &v = *arg; *arg = log(v + sqrt(v * v - 1)); }
      static void MUP_FASTCALL ATanh(T *arg, int) { T &v = *arg; *arg = ((T)0.5 * log((1 + v) / (1 - v))); }
      
      // Logarithms and exponential functions
      static void MUP_FASTCALL Log(T *arg,   int) { *arg = log(*arg); } 
      static void MUP_FASTCALL Log2(T *arg,  int) { *arg = log(*arg)/log((T)2); } // Logarithm base 2
      static void MUP_FASTCALL Log10(T *arg, int) { *arg = log10(*arg); }         // Logarithm base 10
      static void MUP_FASTCALL Exp(T *arg,   int) { *arg = exp(*arg);   }
      static void MUP_FASTCALL Abs(T *arg,   int) { T &v = arg[0]; *arg = (v>=0) ? v : -v; }
      static void MUP_FASTCALL Sqrt(T *arg,  int) { *arg = sqrt(*arg);  }
      static void MUP_FASTCALL Rint(T *arg,  int) { *arg = floor(*arg + (T)0.5); }
      static void MUP_FASTCALL Sign(T *arg,  int) { T &v = arg[0]; *arg = (T)((v<0) ? -1 : (v>0) ? 1 : 0); }

      //---------------------------------------------------------------------------------------------
      static void MUP_FASTCALL UnaryMinus(T *arg, int)  { *arg *= -1; }
      static void MUP_FASTCALL UnaryPlus(T *arg, int)  { }

      //---------------------------------------------------------------------------------------------
      // Functions with unlimited number of arguments
      static void MUP_FASTCALL Sum(T *arg, int a_iArgc)
      { 
        if (!a_iArgc)	
          throw ParserError<TString>(_SL("too few arguments for function sum."));

        T sum = 0;
        for (int i=0; i<a_iArgc; ++i) 
          sum += arg[i];

        arg[0] = sum;
      }

      //---------------------------------------------------------------------------
      static void MUP_FASTCALL Avg(T *arg, int a_iArgc)
      { 
        if (!a_iArgc)	
          throw ParserError<TString>(_SL("too few arguments for function sum."));

        T avg = 0;
        for (int i=0; i<a_iArgc; ++i) 
          avg += arg[i];

        arg[0] = avg;
      }

      //---------------------------------------------------------------------------
      static void MUP_FASTCALL Min(T *arg, int a_iArgc)
      { 
        if (!a_iArgc)	
          throw ParserError<TString>(_SL("too few arguments for function min."));

        T min = arg[0];
        for (int i=0; i<a_iArgc; ++i) 
          min = std::min(min, arg[i]);

        arg[0] = min;
      }

      //---------------------------------------------------------------------------
      static void MUP_FASTCALL Max(T *arg, int a_iArgc)
      { 
        if (!a_iArgc)	
          throw ParserError<TString>(_SL("too few arguments for function min."));

        T max = arg[0];
        for (int i=0; i<a_iArgc; ++i) 
          max = std::max(max, arg[i]);

        arg[0] = max;
      }
    };

#if defined (__GNUG__)
    // Bei zu genauer definition von pi kann die Berechnung von
    // sin(pi*a) mit a=1 10 x langsamer sein! compiler BUG???
    template<typename T, typename TString>
    const T MathImpl<T, TString>::c_pi  = (T)3.141592653589;
#else
    template<typename T, typename TString>
    const T MathImpl<T, TString>::c_pi  = (T)3.141592653589793238462643;
#endif

    template<typename T, typename TString>
    const T MathImpl<T, TString>::c_e  = (T)2.718281828459045235360287;


    //---------------------------------------------------------------------------------------------
    /** \brief Mathematical functions for integer values.
    */
    template<typename TString>
    struct MathImpl<int, TString>
    {
      // Constants
      static const int c_e  = 3;
      static const int c_pi = 4;

      static void MUP_FASTCALL Sin(int*,   int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Cos(int*,   int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Tan(int*,   int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL ASin(int*,  int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL ACos(int*,  int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL ATan(int*,  int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL ATan2(int*, int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Sinh(int*,  int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Cosh(int*,  int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Tanh(int*,  int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL ASinh(int*, int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL ACosh(int*, int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL ATanh(int*, int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Log(int*,   int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Log2(int*,  int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Log10(int*, int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Exp(int*,   int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Sqrt(int*,  int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Rint(int*,  int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      static void MUP_FASTCALL Avg(int*,   int) { throw ParserError<TString>(_SL("unimplemented function.")); }
      
      static void MUP_FASTCALL Sign      (int *arg, int) { int &v = arg[0]; *arg = (v<0) ? -1 : (v>0) ? 1 : 0; }
      static void MUP_FASTCALL Add       (int *arg, int) { *arg += arg[1]; }
      static void MUP_FASTCALL Sub       (int *arg, int) { *arg -= arg[1]; }
      static void MUP_FASTCALL Mul       (int *arg, int) { *arg *= arg[1]; }
      static void MUP_FASTCALL Div       (int *arg, int) { *arg /= arg[1]; }
      static void MUP_FASTCALL Pow       (int *arg, int) {  arg[0] = std::pow((long double)arg[0], arg[1]); }
      static void MUP_FASTCALL Abs       (int *arg, int) { *arg = std::abs(*arg); }
      static void MUP_FASTCALL And       (int *arg, int) { *arg = *arg && arg[1]; }
      static void MUP_FASTCALL Or        (int *arg, int) { *arg = *arg || arg[1]; }
      static void MUP_FASTCALL Less      (int *arg, int) { *arg = *arg <  arg[1]; }
      static void MUP_FASTCALL Greater   (int *arg, int) { *arg = *arg >  arg[1]; }
      static void MUP_FASTCALL LessEq    (int *arg, int) { *arg = *arg <= arg[1]; }
      static void MUP_FASTCALL GreaterEq (int *arg, int) { *arg = *arg >= arg[1]; }
      static void MUP_FASTCALL Equal     (int *arg, int) { *arg = *arg == arg[1]; }
      static void MUP_FASTCALL NotEqual  (int *arg, int) { *arg = *arg != arg[1]; }
      static void MUP_FASTCALL UnaryMinus(int *arg, int) { *arg *= -1; }
      static void MUP_FASTCALL UnaryPlus(int *arg, int) { }

      //---------------------------------------------------------------------------------------------
      // Functions with unlimited number of arguments
      static void MUP_FASTCALL Sum(int *arg, int a_iArgc)
      { 
        if (!a_iArgc)	
          throw ParserError<TString>(_SL("too few arguments for function sum."));

        int sum = 0;
        for (int i=0; i<a_iArgc; ++i) 
          sum += arg[i];

        arg[0] = sum;
      }

      //-------------------------------------------------------------------------------------------
      static void MUP_FASTCALL Min(int *arg, int a_iArgc)
      { 
        if (!a_iArgc)	
          throw ParserError<TString>(_SL("too few arguments for function min."));

        int min = arg[0];
        for (int i=0; i<a_iArgc; ++i) 
          min = std::min(min, arg[i]);

        arg[0] = min;
      }

      //-------------------------------------------------------------------------------------------
      static void MUP_FASTCALL Max(int *arg, int a_iArgc)
      { 
        if (!a_iArgc)	
          throw ParserError<TString>(_SL("too few arguments for function min."));

        int max = arg[0];
        for (int i=0; i<a_iArgc; ++i) 
          max = std::max(max, arg[i]);

        arg[0] = max;
      }
    };
}

#endif

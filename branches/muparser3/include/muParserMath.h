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
      static void Add(T *arg, int /*argc*/) { *arg += arg[1]; }
      static void Sub(T *arg, int /*argc*/) { *arg -= arg[1]; }
      static void Mul(T *arg, int /*argc*/) { *arg *= arg[1]; }
      static void Div(T *arg, int /*argc*/) { *arg /= arg[1]; }
      static void Pow(T *arg, int /*argc*/)
      { 
        T v2 = arg[1]; 

        int v2i = (int)v2;
        if (v2==v2i)
          arg[0] = std::pow(arg[0], v2i);
        else
          arg[0] = std::pow(arg[0], v2); 
      }

      static void And(T *arg,       int) { *arg = *arg && arg[1]; }
      static void Or(T *arg,        int) { *arg = *arg || arg[1]; }
      static void Less(T *arg,      int) { *arg = *arg <  arg[1]; }
      static void Greater(T *arg,   int) { *arg = *arg >  arg[1]; }
      static void LessEq(T *arg,    int) { *arg = *arg <= arg[1]; }
      static void GreaterEq(T *arg, int) { *arg = *arg >= arg[1]; }
      static void Equal(T *arg,     int) { *arg = *arg == arg[1]; }
      static void NotEqual(T *arg,  int) { *arg = *arg != arg[1]; }

      // 
      static void Sin(T *arg,   int) { *arg = sin(*arg);  }
      static void Cos(T *arg,   int) { *arg = cos(*arg);  }
      static void Tan(T *arg,   int) { *arg = tan(*arg);  }
      static void ASin(T *arg,  int) { *arg = asin(*arg); }
      static void ACos(T *arg,  int) { *arg = acos(*arg); }
      static void ATan(T *arg,  int) { *arg = atan(*arg); }
      static void ATan2(T *arg, int) { *arg = atan2(*arg, arg[1]); }
      static void Sinh(T *arg,  int) { *arg = sinh(*arg); }
      static void Cosh(T *arg,  int) { *arg = cosh(*arg); }
      static void Tanh(T *arg,  int) { *arg = tanh(*arg); }
      static void ASinh(T *arg, int) { T &v = *arg; *arg = log(v + sqrt(v * v + 1)); }
      static void ACosh(T *arg, int) { T &v = *arg; *arg = log(v + sqrt(v * v - 1)); }
      static void ATanh(T *arg, int) { T &v = *arg; *arg = ((T)0.5 * log((1 + v) / (1 - v))); }
      
      // Logarithms and exponential functions
      static void Log(T *arg,   int) { *arg = log(*arg); }
      static void Log2(T *arg,  int) { *arg = log(*arg)/log((T)2); } // Logarithm base 2
      static void Log10(T *arg, int) { *arg = log10(*arg); }         // Logarithm base 10
      static void Exp(T *arg,   int) { *arg = exp(*arg);   }
      static void Abs(T *arg,   int) { T &v = arg[0]; *arg = (v>=0) ? v : -v; }
      static void Sqrt(T *arg,  int) { *arg = sqrt(*arg);  }
      static void Rint(T *arg,  int) { *arg = floor(*arg + (T)0.5); }
      static void Sign(T *arg,  int) { T &v = arg[0]; *arg = (T)((v<0) ? -1 : (v>0) ? 1 : 0); }

      //---------------------------------------------------------------------------------------------
      static void UnaryMinus(T *arg, int)  { *arg *= -1; }
      static void UnaryPlus(T * /*arg*/, int)  { }

      //---------------------------------------------------------------------------------------------
      // Functions with unlimited number of arguments
      static void Sum(T *arg, int a_iArgc)
      { 
        if (!a_iArgc)	
          throw ParserError<TString>(_SL("too few arguments for function sum."));

        T sum = 0;
        for (int i=0; i<a_iArgc; ++i) 
          sum += arg[i];

        arg[0] = sum;
      }

      //---------------------------------------------------------------------------
      static void Avg(T *arg, int a_iArgc)
      { 
        if (!a_iArgc)	
          throw ParserError<TString>(_SL("too few arguments for function sum."));

        T avg = 0;
        for (int i=0; i<a_iArgc; ++i) 
          avg += arg[i];

        arg[0] = avg;
      }

      //---------------------------------------------------------------------------
      static void Min(T *arg, int a_iArgc)
      { 
        if (!a_iArgc)	
          throw ParserError<TString>(_SL("too few arguments for function min."));

        T min = arg[0];
        for (int i=0; i<a_iArgc; ++i) 
          min = std::min(min, arg[i]);

        arg[0] = min;
      }

      //---------------------------------------------------------------------------
      static void Max(T *arg, int a_iArgc)
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
}

#endif

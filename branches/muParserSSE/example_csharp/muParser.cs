/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
  Copyright (C) 2007-2010 Ingo Berg

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
using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;


namespace muParser
{
    //---------------------------------------------------------------------------
    public class ParserException : System.Exception
    {
        public ParserException(string sExpr, string sMsg, int nPos, string sTok)
        {
            m_sExpr = sExpr;
            m_sTok  = sTok;
            m_nPos  = nPos;
            m_sMsg  = sMsg;
        }

        public string Expression
        {
            get
            {
                return m_sExpr;
            }
        }

        override public string Message
        {
            get
            {
                return m_sMsg;
            }
        }

        public string Token
        {
            get
            {
                return m_sTok;
            }
        }

        public int Position
        {
            get 
            {
                return m_nPos;
            }
        }

        private string m_sMsg;
        private string m_sExpr;
        private string m_sTok;
        private int m_nPos;
    }

    //---------------------------------------------------------------------------
    public class ParserVariable
    {
      public unsafe ParserVariable()
      {
        m_pVar = mecCreateVar();
        *((float*)m_pVar.ToPointer()) = 0;
      }

      public unsafe ParserVariable(float val)
      {
        m_pVar = mecCreateVar();
        *((float*)m_pVar.ToPointer()) = val;
      }

      ~ParserVariable()
      {
        mecReleaseVar(m_pVar);
      }

      public unsafe float Value
      {
        get
        {
          return *((float*)m_pVar.ToPointer());
        }
        
        set
        {
          *((float*)m_pVar.ToPointer()) = value;
        }
      }

      public IntPtr Pointer
      {
        get
        {
          return m_pVar;
        }
      }
      
      private IntPtr m_pVar;
      
      #region DLL imports

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern IntPtr mecCreateVar();

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern IntPtr mecReleaseVar(IntPtr var);
      
      #endregion
    }

    //---------------------------------------------------------------------------
    public class Parser 
    {
      private IntPtr m_parser = IntPtr.Zero;

      // Keep the delegate in order to prevent deletion
      private List<Delegate> m_binOprtDelegates = new List<Delegate>();

      // Keep the delegate in order to prevent deletion
      private List<Delegate> m_funDelegates = new List<Delegate>();

      // Buffer with all parser variables
      private Dictionary<string, ParserVariable> m_varBuf = new Dictionary<string, ParserVariable>();

      // Keep reference to the delegate of the error function
      private ErrorDelegate m_errCallback;
     
      //------------------------------------------------------------------------------
      public enum EPrec : int
      {
        // binary operators
        prLOGIC = 1,
        prCMP = 2,
        prADD_SUB = 3,
        prMUL_DIV = 4,
        prPOW = 5,

        // infix operators
        prINFIX = 4,
        prPOSTFIX = 4
      }

      //---------------------------------------------------------------------------
      // Delegates
      //---------------------------------------------------------------------------
      #region Delegate definitions

      // Value identification callback
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate int IdentFunDelegate(String name, ref int pos, ref float val);

      // Delegate for the just in time compiled function
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float CompiledFunDelegate();

      // Callback for errors 
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      protected delegate void ErrorDelegate();

      // Functions taking float arguments
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float Fun1Delegate(float val1);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float Fun2Delegate(float val1, float val2);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float Fun3Delegate(float val1, float val2, float val3);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float Fun4Delegate(float val1, float val2, float val3, float val4);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float Fun5Delegate(float val1, float val2, float val3, float val4, float val5);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float Fun6Delegate(float val1, float val2, float val3, float val4, float val5, float val6);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float Fun7Delegate(float val1, float val2, float val3, float val4, float val5, float val6, float val7);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float Fun8Delegate(float val1, float val2, float val3, float val4, float val5, float val6, float val7, float val8);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float Fun9Delegate(float val1, float val2, float val3, float val4, float val5, float val6, float val7, float val8, float val9);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate float Fun10Delegate(float val1, float val2, float val3, float val4, float val5, float val6, float val7, float val8, float val9, float val10);
      
      #endregion

      //---------------------------------------------------------------------------
      // Parser methode wrappers
      //---------------------------------------------------------------------------
      #region Parser methode wrappers

      public Parser()
      {
        m_parser = mecCreate();

        Debug.Assert(m_parser != null, "Parser object is null");
        m_errCallback = new ErrorDelegate(RaiseException);
        mecSetErrorHandler(m_parser, m_errCallback);
      }

     ~Parser()
      {
          mecRelease(m_parser);
      }

      protected void RaiseException()
      {
        string s = mecGetExpr(m_parser);

        ParserException exc = new ParserException(mecGetExpr(m_parser),
                                                  mecGetErrorMsg(m_parser),
                                                  mecGetErrorPos(m_parser),
                                                  mecGetErrorToken(m_parser) );
        throw exc;
      }

      public void AddValIdent(IdentFunDelegate fun)
      {
        mecAddValIdent(m_parser, fun);
      }

      public void SetExpr(string expr)
      {
        mecSetExpr(m_parser, expr);
      }

      public string GetExpr() 
      {
        return mecGetExpr(m_parser);
      }

      public CompiledFunDelegate Compile()
      {
        return mecCompile(m_parser);
      }

      public float Eval()
      {
        return mecEval(m_parser);
      }

      public string GetVersion()
      {
        return mecGetVersion(m_parser);
      }

      public void DefineConst(string name, float val)
      {
          mecDefineConst(m_parser, name, val);
      }

      public void DefineStrConst(string name, String str )
      {
          mecDefineStrConst(m_parser, name, str);
      }
      
      public void DefineVar(string name, ParserVariable var)
      {
        mecDefineVar(m_parser, name, var.Pointer);
        m_varBuf[name] = var;
      }

      public void RemoveVar(string name)
      {
        mecRemoveVar(m_parser, name);
        m_varBuf.Remove(name);
      }

      public void ClearVar()
      {
          mecClearVar(m_parser);
      }

      public void ClearConst()
      {
          mecClearConst(m_parser);
      }

      public void ClearOprt()
      {
        m_binOprtDelegates.Clear();  
        mecClearOprt(m_parser);
      }

      public void ClearFun()
      {
        m_funDelegates.Clear();
        mecClearFun(m_parser);
      }

      public void DefineFun(string name, Fun1Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mecDefineFun1(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun2Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mecDefineFun2(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun3Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mecDefineFun3(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun4Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mecDefineFun4(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun5Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mecDefineFun5(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun6Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mecDefineFun6(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun7Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mecDefineFun7(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun8Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mecDefineFun8(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun9Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mecDefineFun9(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun10Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mecDefineFun10(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }
      
      public void DefineOprt(string name, Fun2Delegate function, int precedence)
      {
        m_binOprtDelegates.Add(function);
        mecDefineOprt(m_parser, name, function, precedence, 0);
      }

      public void DefinePostfixOprt(string name, Fun1Delegate oprt)
      {
        m_binOprtDelegates.Add(oprt);
        mecDefinePostfixOprt(m_parser, name, oprt, 0);    
      }

      public void DefineInfixOprt(string name, Fun1Delegate oprt, EPrec precedence)
      {
        m_binOprtDelegates.Add(oprt);
        mecDefineInfixOprt(m_parser, name, oprt, 0);    
      }

      public Dictionary<string, float> GetConst()
      {
        int num = mecGetConstNum(m_parser);

        Dictionary<string, float> map = new Dictionary<string, float>();
        for (int i = 0; i < num; ++i)
        {
          string name = "";
          float value = 0;
          mecGetConst(m_parser, i, ref name, ref value);

          map[name] = value;
        }

        return map;
      }

      public Dictionary<string, ParserVariable> GetVar()
      {
        return m_varBuf;
      }

      public Dictionary<string, IntPtr> GetExprVar()
      {
        int num = mecGetExprVarNum(m_parser);

        Dictionary<string, IntPtr> map = new Dictionary<string, IntPtr>();
        for (int i = 0; i < num; ++i)
        {
          string name = "";
          IntPtr ptr = IntPtr.Zero;
          mecGetExprVar(m_parser, i, ref name, ref ptr);

          map[name] = ptr;
        }

        return map;
      }

      public void SetArgSep(char cArgSep)
      {
        mecSetArgSep(m_parser, Convert.ToByte(cArgSep));  
      }

      public void SetDecSep(char cDecSep)
      {
        mecSetDecSep(m_parser, Convert.ToByte(cDecSep));
      }

      public void SetThousandsSep(char cThSep)
      {
        mecSetThousandsSep(m_parser, Convert.ToByte(cThSep));
      }

      public void ResetLocale()
      {
        mecResetLocale(m_parser);
      }
      #endregion


      #region DLL function bindings

      //----------------------------------------------------------
      // Basic operations / initialization  
      //----------------------------------------------------------

      [DllImport("muparserSSE.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern IntPtr mecCreate();

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern  CompiledFunDelegate mecCompile(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mecRelease(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecResetLocale(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern string mecGetVersion(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern string mecGetExpr(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecSetExpr(IntPtr a_pParser, string a_szExpr);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecSetErrorHandler(IntPtr a_pParser, ErrorDelegate errFun);

      //---------------------------------------------------------------------------
      // Non numeric callbacks
      //---------------------------------------------------------------------------

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecAddValIdent(IntPtr a_parser, IdentFunDelegate fun);

      //----------------------------------------------------------
      // Defining variables and constants
      //----------------------------------------------------------

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineConst(IntPtr a_pParser, 
                                                  string a_szName,
                                                  float a_fVal);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineStrConst(IntPtr parser, 
                                                      string name, 
                                                      string val );

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineVar(IntPtr parser, 
                                                 string name,
                                                 IntPtr var );

      //----------------------------------------------------------
      // Querying variables / expression variables / constants
      //----------------------------------------------------------

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern int mecGetExprVarNum(IntPtr a_parser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecGetExprVar(IntPtr a_parser,
                                                 int idx,
                                                 ref string name,
                                                 ref IntPtr ptr);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern int mecGetVarNum(IntPtr a_parser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecGetVar(IntPtr a_parser,
                                             int idx,
                                             ref string name,
                                             ref IntPtr ptr);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern int mecGetConstNum(IntPtr a_parser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecGetConst(IntPtr a_parser, 
                                                int idx, 
                                                ref string str,
                                                ref float value);

      //----------------------------------------------------------
      // Remove all / single variables
      //----------------------------------------------------------

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecRemoveVar(IntPtr a_parser, string name);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecClearVar(IntPtr a_parser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecClearConst(IntPtr a_parser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecClearOprt(IntPtr a_parser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecClearFun(IntPtr a_parser);

      //----------------------------------------------------------
      // Define character sets for identifiers
      //----------------------------------------------------------

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineNameChars(IntPtr a_parser, string charset);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineOprtChars(IntPtr a_parser, string charset);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineInfixOprtChars(IntPtr a_parser, string charset);

      //----------------------------------------------------------
      // Defining callbacks / variables / constants
      //----------------------------------------------------------

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineFun1(IntPtr a_parser, string name, Fun1Delegate fun, int optimize);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineFun2(IntPtr a_parser, string name, Fun2Delegate fun, int optimize);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineFun3(IntPtr a_parser, string name, Fun3Delegate fun, int optimize);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineFun4(IntPtr a_parser, string name, Fun4Delegate fun, int optimize);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineFun5(IntPtr a_parser, string name, Fun5Delegate fun, int optimize);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineFun6(IntPtr a_parser, string name, Fun6Delegate fun, int optimize);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineFun7(IntPtr a_parser, string name, Fun7Delegate fun, int optimize);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineFun8(IntPtr a_parser, string name, Fun8Delegate fun, int optimize);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineFun9(IntPtr a_parser, string name, Fun9Delegate fun, int optimize);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineFun10(IntPtr a_parser, string name, Fun10Delegate fun, int optimize);

      //----------------------------------------------------------
      // Operator definitions
      //----------------------------------------------------------

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineOprt(IntPtr a_pParser, 
                                                 string name, 
                                                 Fun2Delegate fun, 
                                                 int precedence, 
                                                 int optimize);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefinePostfixOprt(IntPtr a_pParser, 
                                                        string id, 
                                                        Fun1Delegate fun, 
                                                        int optimize );

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecDefineInfixOprt(IntPtr a_pParser, 
                                                      string id, 
                                                      Fun1Delegate fun,
                                                      int optimize);
      
      //----------------------------------------------------------
      // 
      //----------------------------------------------------------

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern float mecEval(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern int mecError(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecErrorReset(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern int mecGetErrorCode(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern int mecGetErrorPos(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)] //[return : MarshalAs(UnmanagedType.LPStr)]
      protected static extern string mecGetErrorMsg(IntPtr a_pParser);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern string mecGetErrorToken(IntPtr a_pParser);
        
      //----------------------------------------------------------
      // Localization
      //----------------------------------------------------------

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecSetArgSep(IntPtr a_pParser, byte cArgSep);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecSetDecSep(IntPtr a_pParser, byte cArgSep);

      [DllImport("muparserSSE.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mecSetThousandsSep(IntPtr a_pParser, byte cArgSep);
        
      #endregion
    }
}

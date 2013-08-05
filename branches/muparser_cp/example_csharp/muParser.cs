/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
  Copyright (C) 2007-2011 Ingo Berg

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
        m_pVar = mupCreateVar();
        *((double*)m_pVar.ToPointer()) = 0;
      }

      public unsafe ParserVariable(double val)
      {
        m_pVar = mupCreateVar();
        *((double*)m_pVar.ToPointer()) = val;
      }

      ~ParserVariable()
      {
        mupReleaseVar(m_pVar);
      }

      public unsafe double Value
      {
        get
        {
          return *((double*)m_pVar.ToPointer());
        }
        
        set
        {
          *((double*)m_pVar.ToPointer()) = value;
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

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern IntPtr mupCreateVar();

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern IntPtr mupReleaseVar(IntPtr var);
      
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

      public enum EBaseType : int
      {
        tpDOUBLE = 0,
        tpINT = 1
      }
      
      //---------------------------------------------------------------------------
      // Delegates
      //---------------------------------------------------------------------------
      #region Delegate definitions

      //[UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      //protected delegate IntPtr FactoryDelegate(String name, IntPtr parser);

      // Value identification callback
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate int IdentFunDelegate(String name, ref int pos, ref double val);

      // Callback for errors 
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      protected delegate void ErrorDelegate();

      // Functions taking double arguments
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double Fun1Delegate(double val1);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double Fun2Delegate(double val1, double val2);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double Fun3Delegate(double val1, double val2, double val3);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double Fun4Delegate(double val1, double val2, double val3, double val4);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double Fun5Delegate(double val1, double val2, double val3, double val4, double val5);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double Fun6Delegate(double val1, double val2, double val3, double val4, double val5, double val6);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double Fun7Delegate(double val1, double val2, double val3, double val4, double val5, double val6, double val7);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double Fun8Delegate(double val1, double val2, double val3, double val4, double val5, double val6, double val7, double val8);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double Fun9Delegate(double val1, double val2, double val3, double val4, double val5, double val6, double val7, double val8, double val9);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double Fun10Delegate(double val1, double val2, double val3, double val4, double val5, double val6, double val7, double val8, double val9, double val10);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double BulkFun1Delegate(int nBulkIdx, int nThreadIdx, double val1);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double BulkFun2Delegate(int nBulkIdx, int nThreadIdx, double val1, double val2);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double BulkFun3Delegate(int nBulkIdx, int nThreadIdx, double val1, double val2, double val3);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double BulkFun4Delegate(int nBulkIdx, int nThreadIdx, double val1, double val2, double val3, double val4);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double BulkFun5Delegate(int nBulkIdx, int nThreadIdx, double val1, double val2, double val3, double val4, double val5);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double BulkFun6Delegate(int nBulkIdx, int nThreadIdx, double val1, double val2, double val3, double val4, double val5, double val6);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double BulkFun7Delegate(int nBulkIdx, int nThreadIdx, double val1, double val2, double val3, double val4, double val5, double val6, double val7);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double BulkFun8Delegate(int nBulkIdx, int nThreadIdx, double val1, double val2, double val3, double val4, double val5, double val6, double val7, double val8);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double BulkFun9Delegate(int nBulkIdx, int nThreadIdx, double val1, double val2, double val3, double val4, double val5, double val6, double val7, double val8, double val9);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double BulkFun10Delegate(int nBulkIdx, int nThreadIdx, double val1, double val2, double val3, double val4, double val5, double val6, double val7, double val8, double val9, double val10);

      // Functions taking an additional string parameter
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double StrFun1Delegate(String name);
      
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double StrFun2Delegate(String name, double val1);

      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double StrFun3Delegate(String name, double val1, double val2);

      // Functions taking an additional string parameter
      [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
      public delegate double MultFunDelegate(
                  [MarshalAs(UnmanagedType.LPArray, SizeParamIndex=1)] double[] array,
                  int size);

      #endregion

      //---------------------------------------------------------------------------
      // Parser methode wrappers
      //---------------------------------------------------------------------------
      #region Parser methode wrappers

      public Parser(EBaseType eType)
      {
        m_parser = mupCreate((int)eType);
        Debug.Assert(m_parser != null, "Parser object is null");
        m_errCallback = new ErrorDelegate(RaiseException);
        mupSetErrorHandler(m_parser, m_errCallback);
      }

     ~Parser()
      {
          mupRelease(m_parser);
      }

      protected void RaiseException()
      {
        string s = GetExpr();

        ParserException exc = new ParserException(GetExpr(),
                                                  GetErrorMsg(),
                                                  mupGetErrorPos(m_parser),
                                                  GetErrorToken() );
        throw exc;
      }

      public void AddValIdent(IdentFunDelegate fun)
      {
        mupAddValIdent(m_parser, fun);
      }

      public void SetExpr(string expr)
      {
        mupSetExpr(m_parser, expr);
      }

      public string GetVersion()
      {
        return Marshal.PtrToStringAnsi(mupGetVersion(m_parser));
      }

      private string GetErrorMsg()
      {
        return Marshal.PtrToStringAnsi(mupGetErrorMsg(m_parser));
      }

      private string GetErrorToken()
      {
        return Marshal.PtrToStringAnsi(mupGetErrorToken(m_parser));
      }

      public string GetExpr() 
      {
        return Marshal.PtrToStringAnsi(mupGetExpr(m_parser));
      }

      public double Eval()
      {
        return mupEval(m_parser);
      }

      public double[] EvalMultiExpr()
      {
        int nNum;
        IntPtr p = mupEvalMulti(m_parser, out nNum);
        double[] array = new double[nNum];
        Marshal.Copy(p, array, 0, nNum);
        return array;
      }

      public void Eval(double [] results, int nSize)
      {
        mupEvalBulk(m_parser, results, nSize);
      }

      public void DefineConst(string name, double val )
      {
        mupDefineConst(m_parser, name, val);
      }

      public void DefineStrConst(string name, String str )
      {
        mupDefineStrConst(m_parser, name, str);
      }

      public void DefineVar(string name, double[] var)
      {
          mupDefineBulkVar(m_parser, name, var);
      }

      public void DefineVar(string name, ParserVariable var)
      {
        mupDefineVar(m_parser, name, var.Pointer);
        m_varBuf[name] = var;
      }

      public void RemoveVar(string name)
      {
        mupRemoveVar(m_parser, name);
        m_varBuf.Remove(name);
      }

      public void ClearVar()
      {
          mupClearVar(m_parser);
      }

      public void ClearConst()
      {
          mupClearConst(m_parser);
      }

      public void ClearOprt()
      {
        m_binOprtDelegates.Clear();  
        mupClearOprt(m_parser);
      }

      public void ClearFun()
      {
        m_funDelegates.Clear();
        mupClearFun(m_parser);
      }

      #region define numeric functions with fixed number of arguments

      public void DefineFun(string name, Fun1Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineFun1(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun2Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineFun2(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun3Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineFun3(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun4Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineFun4(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun5Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineFun5(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun6Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineFun6(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun7Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineFun7(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun8Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineFun8(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun9Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineFun9(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      public void DefineFun(string name, Fun10Delegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineFun10(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      #endregion

      #region Defining bulk mode functions

      public void DefineFun(string name, BulkFun1Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineBulkFun1(m_parser, name, function);
      }

      public void DefineFun(string name, BulkFun2Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineBulkFun2(m_parser, name, function);
      }

      public void DefineFun(string name, BulkFun3Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineBulkFun3(m_parser, name, function);
      }

      public void DefineFun(string name, BulkFun4Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineBulkFun4(m_parser, name, function);
      }

      public void DefineFun(string name, BulkFun5Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineBulkFun5(m_parser, name, function);
      }

      public void DefineFun(string name, BulkFun6Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineBulkFun6(m_parser, name, function);
      }

      public void DefineFun(string name, BulkFun7Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineBulkFun7(m_parser, name, function);
      }

      public void DefineFun(string name, BulkFun8Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineBulkFun8(m_parser, name, function);
      }

      public void DefineFun(string name, BulkFun9Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineBulkFun9(m_parser, name, function);
      }

      public void DefineFun(string name, BulkFun10Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineBulkFun10(m_parser, name, function);
      }

      #endregion

      #region define other functions

      public void DefineFun(string name, StrFun1Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineStrFun1(m_parser, name, function);
      }

      public void DefineFun(string name, StrFun2Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineStrFun2(m_parser, name, function);
      }

      public void DefineFun(string name, StrFun3Delegate function)
      {
        m_funDelegates.Add(function);
        mupDefineStrFun3(m_parser, name, function);
      }

      public void DefineFun(string name, MultFunDelegate function, bool bAllowOptimization)
      {
        m_funDelegates.Add(function);
        mupDefineMultFun(m_parser, name, function, (bAllowOptimization) ? 1 : 0);
      }

      #endregion

      #region define operators

      public void DefineOprt(string name, Fun2Delegate function, int precedence)
      {
        m_binOprtDelegates.Add(function);
        mupDefineOprt(m_parser, name, function, precedence, 0);
      }

      public void DefinePostfixOprt(string name, Fun1Delegate oprt)
      {
        m_binOprtDelegates.Add(oprt);
        mupDefinePostfixOprt(m_parser, name, oprt, 0);    
      }

      public void DefineInfixOprt(string name, Fun1Delegate oprt, EPrec precedence)
      {
        m_binOprtDelegates.Add(oprt);
        mupDefineInfixOprt(m_parser, name, oprt, 0 );
      }

      #endregion

      public Dictionary<string, double> GetConst()
      {
        int num = mupGetConstNum(m_parser);

        Dictionary<string, double> map = new Dictionary<string, double>();
        for (int i = 0; i < num; ++i)
        {
          string name = "";
          double value = 0;
          mupGetConst(m_parser, i, ref name, ref value);

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
        int num = mupGetExprVarNum(m_parser);

        Dictionary<string, IntPtr> map = new Dictionary<string, IntPtr>();
        for (int i = 0; i < num; ++i)
        {
          string name = "";
          IntPtr ptr = IntPtr.Zero;
          mupGetExprVar(m_parser, i, ref name, ref ptr);

          map[name] = ptr;
        }

        return map;
      }

      public void SetArgSep(char cArgSep)
      {
        mupSetArgSep(m_parser, Convert.ToByte(cArgSep));  
      }

      public void SetDecSep(char cDecSep)
      {
        mupSetDecSep(m_parser, Convert.ToByte(cDecSep));
      }

      public void SetThousandsSep(char cThSep)
      {
        mupSetThousandsSep(m_parser, Convert.ToByte(cThSep));
      }

      public void ResetLocale()
      {
        mupResetLocale(m_parser);
      }
      #endregion


      #region DLL function bindings

      //----------------------------------------------------------
      // Basic operations / initialization  
      //----------------------------------------------------------

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern IntPtr mupCreate(int nType);
      
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupRelease(IntPtr a_pParser);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupResetLocale(IntPtr a_pParser);

      // Achtung Marshalling von Classen, die strings zurückgeben funktioniert über IntPtr
      // weil C# den string sonst freigeben wird!
      //
      // siehe auch:
      // http://discuss.fogcreek.com/dotnetquestions/default.asp?cmd=show&ixPost=1108
      // http://groups.google.com/group/microsoft.public.dotnet.framework/msg/9807f3b190c31f6d
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern IntPtr mupGetVersion(IntPtr a_pParser);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern IntPtr mupGetExpr(IntPtr a_pParser);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern IntPtr mupGetErrorMsg(IntPtr a_pParser);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern IntPtr mupGetErrorToken(IntPtr a_pParser);
      // ende

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupSetExpr(IntPtr a_pParser, string a_szExpr);
      
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupSetErrorHandler(IntPtr a_pParser, ErrorDelegate errFun);

      //---------------------------------------------------------------------------
      // Non numeric callbacks
      //---------------------------------------------------------------------------

      //[DllImport("muparser.dll")]
      //protected static extern void mupSetVarFactory(HandleRef a_pParser, muFacFun_t a_pFactory, void* pUserData);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupAddValIdent(IntPtr a_parser, IdentFunDelegate fun);

      //----------------------------------------------------------
      // Defining variables and constants
      //----------------------------------------------------------

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineConst( IntPtr a_pParser, 
                                                   string a_szName, 
                                                   double a_fVal );

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineStrConst( IntPtr parser, 
                                                      string name, 
                                                      string val );

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineVar(IntPtr parser, 
                                                string name,
                                                IntPtr var);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkVar(IntPtr parser,
                                                    string name,
                                                    double [] var);

      //----------------------------------------------------------
      // Querying variables / expression variables / constants
      //----------------------------------------------------------

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern int mupGetExprVarNum(IntPtr a_parser);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupGetExprVar(IntPtr a_parser,
                                                 int idx,
                                                 ref string name,
                                                 ref IntPtr ptr);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern int mupGetVarNum(IntPtr a_parser);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupGetVar(IntPtr a_parser,
                                             int idx,
                                             ref string name,
                                             ref IntPtr ptr);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern int mupGetConstNum(IntPtr a_parser);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupGetConst( IntPtr a_parser, 
                                                int idx, 
                                                ref string str,
                                                ref double value);

      //[DllImport("muparser.dll")]
      //protected static extern void mupGetExprVar(IntPtr a_parser, unsigned a_iVar, const muChar_t** a_pszName, muFloat_t** a_pVar);

      //----------------------------------------------------------
      // Remove all / single variables
      //----------------------------------------------------------

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupRemoveVar(IntPtr a_parser, string name);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupClearVar(IntPtr a_parser);
      
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupClearConst(IntPtr a_parser);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupClearOprt(IntPtr a_parser);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupClearFun(IntPtr a_parser);

      //----------------------------------------------------------
      // Define character sets for identifiers
      //----------------------------------------------------------

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineNameChars(IntPtr a_parser, string charset);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineOprtChars(IntPtr a_parser, string charset);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineInfixOprtChars(IntPtr a_parser, string charset);

      //----------------------------------------------------------
      // Defining callbacks / variables / constants
      //----------------------------------------------------------

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineFun1(IntPtr a_parser, string name, Fun1Delegate fun, int optimize);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineFun2(IntPtr a_parser, string name, Fun2Delegate fun, int optimize);
        
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
       protected static extern void mupDefineFun3(IntPtr a_parser, string name, Fun3Delegate fun, int optimize);
       
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineFun4(IntPtr a_parser, string name, Fun4Delegate fun, int optimize);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineFun5(IntPtr a_parser, string name, Fun5Delegate fun, int optimize);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineFun6(IntPtr a_parser, string name, Fun6Delegate fun, int optimize);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineFun7(IntPtr a_parser, string name, Fun7Delegate fun, int optimize);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineFun8(IntPtr a_parser, string name, Fun8Delegate fun, int optimize);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineFun9(IntPtr a_parser, string name, Fun9Delegate fun, int optimize);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineFun10(IntPtr a_parser, string name, Fun10Delegate fun, int optimize);

      // Bulk mode functions
      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkFun1(IntPtr a_parser, string name, BulkFun1Delegate fun);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkFun2(IntPtr a_parser, string name, BulkFun2Delegate fun);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkFun3(IntPtr a_parser, string name, BulkFun3Delegate fun);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkFun4(IntPtr a_parser, string name, BulkFun4Delegate fun);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkFun5(IntPtr a_parser, string name, BulkFun5Delegate fun);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkFun6(IntPtr a_parser, string name, BulkFun6Delegate fun);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkFun7(IntPtr a_parser, string name, BulkFun7Delegate fun);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkFun8(IntPtr a_parser, string name, BulkFun8Delegate fun);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkFun9(IntPtr a_parser, string name, BulkFun9Delegate fun);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupDefineBulkFun10(IntPtr a_parser, string name, BulkFun10Delegate fun);

      // string functions
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineStrFun1(IntPtr a_parser, string name, StrFun1Delegate fun);
        
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineStrFun2(IntPtr a_parser, string name, StrFun2Delegate fun);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineStrFun3(IntPtr a_parser, string name, StrFun3Delegate fun);

      // Multiple argument functions
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineMultFun(IntPtr a_parser, string name, MultFunDelegate fun, int optimize);

      //----------------------------------------------------------
      // Operator definitions
      //----------------------------------------------------------

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineOprt(IntPtr a_pParser, 
                                                 string name, 
                                                 Fun2Delegate fun, 
                                                 int precedence, 
                                                 int optimize);

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefinePostfixOprt(IntPtr a_pParser, 
                                                        string id, 
                                                        Fun1Delegate fun, 
                                                        int optimize );

      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupDefineInfixOprt(IntPtr a_pParser, 
                                                      string id, 
                                                      Fun1Delegate fun,
                                                      int optimize);
      
      //----------------------------------------------------------
      // 
      //----------------------------------------------------------

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern double mupEval(IntPtr a_pParser);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern double mupEvalBulk(IntPtr a_pParser, double[] results, int nBulkSize);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern IntPtr mupEvalMulti(IntPtr a_pParser, out int nSize);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern int mupError(IntPtr a_pParser);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupErrorReset(IntPtr a_pParser);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern int mupGetErrorCode(IntPtr a_pParser);

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern int mupGetErrorPos(IntPtr a_pParser);

      //----------------------------------------------------------
      // Localization
      //----------------------------------------------------------

      [DllImport("muparser.dll", CallingConvention = CallingConvention.Cdecl)]
      protected static extern void mupSetArgSep(IntPtr a_pParser, byte cArgSep);
      
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupSetDecSep(IntPtr a_pParser, byte cArgSep);
      
      [DllImport("muparser.dll", CallingConvention=CallingConvention.Cdecl)]
      protected static extern void mupSetThousandsSep(IntPtr a_pParser, byte cArgSep);
        
      #endregion
    }
}

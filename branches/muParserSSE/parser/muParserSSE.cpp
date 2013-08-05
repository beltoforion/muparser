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
#if defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

#include "muParserSSE.h"

//#include "mecParserDLL.h"
#include "mecError.h"
#include "mecUnitTest.h"

#define MU_TRY  \
        try     \
        {       \
          using namespace mec; \

#define MU_CATCH                                                 \
        }                                                        \
        catch(mecError_t &e)                                      \
        {                                                        \
          ParserTag *pTag = static_cast<ParserTag*>(a_hParser);  \
          pTag->exc = e;                                         \
          pTag->bError = true;                                   \
          if (pTag->errHandler)                                  \
            (pTag->errHandler)(a_hParser);                       \
        }                                                        \
        catch(...)                                               \
        {                                                        \
          ParserTag *pTag = static_cast<ParserTag*>(a_hParser);  \
          pTag->exc = mecError_t(mec::ecINTERNAL_ERROR);          \
          pTag->bError = true;                                   \
          if (pTag->errHandler)                                  \
            (pTag->errHandler)(a_hParser);                       \
        }

/** \file 
    \brief This file contains the implementation of the DLL interface of muparser.
*/

//---------------------------------------------------------------------------
class ParserTag
{
public:
  ParserTag()
    :pParser(new mec::Parser())
    ,exc()
    ,errHandler(NULL)
    ,bError(false)
  {}
 
 ~ParserTag()
  {
    delete pParser;
  }

  mec::Parser *pParser;
  mec::ParserBase::exception_type exc;
  mecErrorHandler_t errHandler;
  bool bError;

private:
  ParserTag(const ParserTag &ref);
  ParserTag& operator=(const ParserTag &ref);
};

static mecChar_t s_tmpOutBuf[2048];

//---------------------------------------------------------------------------
// private types
typedef mec::ParserBase::exception_type mecError_t;
typedef mec::Parser mecParser_t;

//---------------------------------------------------------------------------
// constants
int mecOPRT_ASCT_LEFT = 0;
int mecOPRT_ASCT_RIGHT = 1;

// Error codes
int mecUNEXPECTED_OPERATOR = 0;  ///< Unexpected binary operator found
int mecUNASSIGNABLE_TOKEN  = 1;  ///< Token cant be identified.
int mecUNEXPECTED_EOF      = 2;  ///< Unexpected end of formula. (Example: "2+sin(")
int mecUNEXPECTED_ARG_SEP  = 3;  ///< An unexpected comma has been found. (Example: "1,23")
int mecUNEXPECTED_ARG      = 4;  ///< An unexpected argument has been found
int mecUNEXPECTED_VAL      = 5;  ///< An unexpected value token has been found
int mecUNEXPECTED_VAR      = 6;  ///< An unexpected variable token has been found
int mecUNEXPECTED_PARENS   = 7;  ///< Unexpected Parenthesis, opening or closing
int mecUNEXPECTED_STR      = 8;  ///< A string has been found at an inapropriate position
int mecSTRING_EXPECTED     = 9;  ///< A string function has been called with a different type of argument
int mecVAL_EXPECTED        = 10; ///< A numerical function has been called with a non value type of argument
int mecMISSING_PARENS      = 11; ///< Missing parens. (Example: "3*sin(3")
int mecUNEXPECTED_FUN      = 12; ///< Unexpected function found. (Example: "sin(8)cos(9)")
int mecUNTERMINATED_STRING = 13; ///< unterminated string constant. (Example: "3*valueof("hello)")
int mecTOO_MANY_PARAMS     = 14; ///< Too many function parameters
int mecTOO_FEW_PARAMS      = 15; ///< Too few function parameters. (Example: "ite(1<2,2)")
int mecOPRT_TYPE_CONFLICT  = 16; ///< binary operators may only be applied to value items of the same type
int mecSTR_RESULT          = 17; ///< result is a string
int mecINVALID_NAME        = 18; ///< Invalid function, variable or constant name.
int mecBUILTIN_OVERLOAD    = 19; ///< Trying to overload builtin operator
int mecINVALID_FUN_PTR     = 20; ///< Invalid callback function pointer 
int mecINVALID_VAR_PTR     = 21; ///< Invalid variable pointer 
int mecEMPTY_EXPRESSION    = 22; ///< The Expression is empty
int mecNAME_CONFLICT       = 23; ///< Name conflict
int mecOPT_PRI             = 24; ///< Invalid operator priority
int mecDOMAIN_ERROR        = 25; ///< catch division by zero, sqrt(-1), log(0) (currently unused)
int mecDIV_BY_ZERO         = 26; ///< Division by zero (currently unused)
int mecGENERIC             = 27; ///< Generic error
int mecLOCALE              = 28; ///< Conflict with current locale
int mecINTERNAL_ERROR      = 29; ///< Internal error of any kind.
int mecUNDEFINED           = -1; 

//---------------------------------------------------------------------------
//
//
//  unexported functions
//
//
//---------------------------------------------------------------------------

mecParser_t* AsParser(mecParserHandle_t a_hParser)
{
  return static_cast<ParserTag*>(a_hParser)->pParser;
}

//---------------------------------------------------------------------------
ParserTag* AsParserTag(mecParserHandle_t a_hParser)
{
  return static_cast<ParserTag*>(a_hParser);
}

//---------------------------------------------------------------------------
#if defined(_WIN32)
  #define _CRT_SECURE_NO_DEPRECATE

  BOOL APIENTRY DllMain( HANDLE /*hModule*/, 
                         DWORD ul_reason_for_call, 
                         LPVOID /*lpReserved*/ )
  {
	  switch (ul_reason_for_call)
	  {
	  case  DLL_PROCESS_ATTACH:
          break;

    case  DLL_THREAD_ATTACH:
    case  DLL_THREAD_DETACH:
    case  DLL_PROCESS_DETACH:
    		  break;
	  }

    return TRUE;
  }

#endif

//---------------------------------------------------------------------------
//
//
//  exported functions
//
//
//---------------------------------------------------------------------------

//API_EXPORT(void) mecSetVarFactory(mecParserHandle_t a_hParser, mecFacFun_t a_pFactory, void *pUserData)
//{
//  MU_TRY
//    mecParser_t* p(AsParser(a_hParser));
//    p->SetVarFactory(a_pFactory, pUserData);
//  MU_CATCH
//}

//---------------------------------------------------------------------------
API_EXPORT(void) mecSelfTest()
{
  mec::Test::UnitTest pt;
  pt.Run();
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDebugDump(int nDumpCmd, int nDumpStack)
{
 mec::g_DbgDumpCmdCode = nDumpCmd!=0;
 mec::g_DbgDumpStack = nDumpStack!=0;
}

//---------------------------------------------------------------------------
/** \brief Create a new Parser instance and return its handle.
*/
API_EXPORT(mecParserHandle_t) mecCreate()
{
  return (void*)(new ParserTag());
}

//---------------------------------------------------------------------------
/** \brief Release the parser instance related with a parser handle.
*/
API_EXPORT(void) mecRelease(mecParserHandle_t a_hParser)
{
  MU_TRY
    ParserTag* p = static_cast<ParserTag*>(a_hParser);
    delete p;
  MU_CATCH
}

//---------------------------------------------------------------------------
/** \brief Release the parser instance related with a parser handle.
*/
API_EXPORT(mecEvalFun_t) mecCompile(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));

  return p->Compile(5);
  MU_CATCH

  return NULL;
}

//---------------------------------------------------------------------------
/** \brief Release the parser instance related with a parser handle.
*/
API_EXPORT(mecEvalFun_t) mecDbgCompile(mecParserHandle_t a_hParser, int nRegNum)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));

  return p->Compile(nRegNum);
  MU_CATCH

  return NULL;
}

//---------------------------------------------------------------------------
API_EXPORT(float) mecEval(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
/*
#if !defined(NO_MICROSOFT_STYLE_INLINE_ASSEMBLY)
    p->SetParserEngine(peBYTECODE_ASM);
#else
    p->SetParserEngine(peBYTECODE);
#endif
*/
    return p->Eval();
  MU_CATCH
  
  return 0;
}

//---------------------------------------------------------------------------
API_EXPORT(const mecChar_t*) mecGetVersion(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    sprintf(s_tmpOutBuf, "%s", p->GetVersion().c_str());
    return s_tmpOutBuf;
  MU_CATCH

  return "";
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecSetExpr(mecParserHandle_t a_hParser, const mecChar_t* a_szExpr)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->SetExpr(a_szExpr);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecRemoveVar(mecParserHandle_t a_hParser, const mecChar_t* a_szName)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->RemoveVar( a_szName );
  MU_CATCH
}

//---------------------------------------------------------------------------
/** \brief Release all parser variables.
    \param a_hParser Handle to the parser instance.
*/
API_EXPORT(void) mecClearVar(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->ClearVar();
  MU_CATCH
}

//---------------------------------------------------------------------------
/** \brief Release all parser variables.
    \param a_hParser Handle to the parser instance.
*/
API_EXPORT(void) mecClearConst(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->ClearConst();
  MU_CATCH
}

//---------------------------------------------------------------------------
/** \brief Clear all user defined operators.
    \param a_hParser Handle to the parser instance.
*/
API_EXPORT(void) mecClearOprt(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->ClearOprt();
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecClearFun(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->ClearFun();
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun0( mecParserHandle_t a_hParser, 
                                const mecChar_t* a_szName, 
                                mecFun0_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun1( mecParserHandle_t a_hParser, 
                                const mecChar_t* a_szName, 
                                mecFun1_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun2( mecParserHandle_t a_hParser, 
                                const mecChar_t* a_szName, 
                                mecFun2_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun3( mecParserHandle_t a_hParser, 
                                const mecChar_t *a_szName, 
                                mecFun3_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun4( mecParserHandle_t a_hParser, 
                                const mecChar_t *a_szName, 
                                mecFun4_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun5( mecParserHandle_t a_hParser, 
                                const mecChar_t *a_szName, 
                                mecFun5_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun6( mecParserHandle_t a_hParser, 
                                const mecChar_t *a_szName, 
                                mecFun6_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun7( mecParserHandle_t a_hParser, 
                                const mecChar_t *a_szName, 
                                mecFun7_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun8( mecParserHandle_t a_hParser, 
                                const mecChar_t *a_szName, 
                                mecFun8_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun9( mecParserHandle_t a_hParser, 
                                const mecChar_t *a_szName, 
                                mecFun9_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineFun10( mecParserHandle_t a_hParser, 
                                const mecChar_t *a_szName, 
                                mecFun10_t a_pFun, 
                                mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineFun(a_szName, a_pFun, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineOprt( mecParserHandle_t a_hParser, 
                                const mecChar_t* a_szName, 
                                mecFun2_t a_pFun, 
                                mecInt_t a_nPrec, 
                                mecInt_t a_nOprtAsct,
                                mecBool_t a_bAllowOpt)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineOprt(a_szName, 
                  a_pFun, 
                  a_nPrec, 
                  (EOprtAssociativity)a_nOprtAsct, 
                  a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineVar(mecParserHandle_t a_hParser, 
                              const char *a_szName, 
                              float *a_pVar)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineVar(a_szName, a_pVar);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineConst(mecParserHandle_t a_hParser, const char *a_szName, float a_fVal)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineConst(a_szName, a_fVal);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(const mecChar_t*) mecGetExpr(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    sprintf(s_tmpOutBuf, "%s", p->GetExpr().c_str());
    return s_tmpOutBuf;
  MU_CATCH

  return "";
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefinePostfixOprt( mecParserHandle_t a_hParser,
                                       const mecChar_t* a_szName,
                                       mecFun1_t a_pOprt,
                                       mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefinePostfixOprt(a_szName, a_pOprt, a_bAllowOpt!=0);
  MU_CATCH
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineInfixOprt( mecParserHandle_t a_hParser,
                                     const mecChar_t* a_szName,
                                     mecFun1_t a_pOprt,
                                     mecBool_t a_bAllowOpt )
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->DefineInfixOprt(a_szName, a_pOprt, a_bAllowOpt!=0);
  MU_CATCH
}

// Define character sets for identifiers
//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineNameChars( mecParserHandle_t a_hParser, 
                                     const mecChar_t* a_szCharset )
{
    mecParser_t* const p(AsParser(a_hParser));
  p->DefineNameChars(a_szCharset);
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineOprtChars( mecParserHandle_t a_hParser, 
                                     const mecChar_t* a_szCharset )
{
  mecParser_t* const p(AsParser(a_hParser));
  p->DefineOprtChars(a_szCharset);
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecDefineInfixOprtChars(mecParserHandle_t a_hParser, const char *a_szCharset)
{
  mecParser_t* const p(AsParser(a_hParser));
  p->DefineInfixOprtChars(a_szCharset);
}

//---------------------------------------------------------------------------
/** \brief Get the number of variables defined in the parser.
    \param a_hParser [in] Must be a valid parser handle.
    \return The number of used variables.
    \sa mecGetExprVar
*/
API_EXPORT(int) mecGetVarNum(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    const varmap_type VarMap = p->GetVar();
    return (int)VarMap.size();
  MU_CATCH

  return 0; // never reached
}

//---------------------------------------------------------------------------
/** \brief Return a variable that is used in an expression.
    \param a_hParser [in] A valid parser handle.
    \param a_iVar [in] The index of the variable to return.
    \param a_szName [out] Pointer to the variable name.
    \param a_pVar [out] Pointer to the variable.
    \throw nothrow

    Prior to calling this function call mecGetExprVarNum in order to get the
    number of variables in the expression. If the parameter a_iVar is greater
    than the number of variables both a_szName and a_pVar will be set to zero.
    As a side effect this function will trigger an internal calculation of the
    expression undefined variables will be set to zero during this calculation.
    During the calculation user defined callback functions present in the expression
    will be called, this is unavoidable.
*/
API_EXPORT(void) mecGetVar(mecParserHandle_t a_hParser, unsigned a_iVar, const char **a_szName, mecFloat_t **a_pVar)
{
  // A static buffer is needed for the name since i cant return the
  // pointer from the map.
  static char  szName[1024];

  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    const varmap_type VarMap = p->GetVar();

    if (a_iVar>=VarMap.size())
    {
     *a_szName = 0;
     *a_pVar = 0;
      return;
    }
    varmap_type::const_iterator item;

    item = VarMap.begin();
    for (unsigned i=0; i<a_iVar; ++i)
      item++;

     strncpy(szName, item->first.c_str(), sizeof(szName));
     szName[sizeof(szName)-1] = 0;

    *a_szName = &szName[0];
    *a_pVar = item->second;
     return;

  MU_CATCH

  *a_szName = 0;
  *a_pVar = 0;
}

//---------------------------------------------------------------------------
/** \brief Get the number of variables used in the expression currently set in the parser.
    \param a_hParser [in] Must be a valid parser handle.
    \return The number of used variables.
    \sa mecGetExprVar
*/
API_EXPORT(int) mecGetExprVarNum(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    const varmap_type VarMap = p->GetUsedVar();
    return (int)VarMap.size();
  MU_CATCH

  return 0; // never reached
}

//---------------------------------------------------------------------------
/** \brief Return a variable that is used in an expression.

    Prior to calling this function call mecGetExprVarNum in order to get the
    number of variables in the expression. If the parameter a_iVar is greater
    than the number of variables both a_szName and a_pVar will be set to zero.
    As a side effect this function will trigger an internal calculation of the
    expression undefined variables will be set to zero during this calculation.
    During the calculation user defined callback functions present in the expression
    will be called, this is unavoidable.

    \param a_hParser [in] A valid parser handle.
    \param a_iVar [in] The index of the variable to return.
    \param a_szName [out] Pointer to the variable name.
    \param a_pVar [out] Pointer to the variable.
    \throw nothrow
*/
API_EXPORT(void) mecGetExprVar(mecParserHandle_t a_hParser, unsigned a_iVar, const char **a_szName, mecFloat_t **a_pVar)
{
  // A static buffer is needed for the name since i cant return the
  // pointer from the map.
  static char  szName[1024];

  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    const varmap_type VarMap = p->GetUsedVar();

    if (a_iVar>=VarMap.size())
    {
     *a_szName = 0;
     *a_pVar = 0;
      return;
    }
    varmap_type::const_iterator item;

    item = VarMap.begin();
    for (unsigned i=0; i<a_iVar; ++i)
      item++;

     strncpy(szName, item->first.c_str(), sizeof(szName));
     szName[sizeof(szName)-1] = 0;

    *a_szName = &szName[0];
    *a_pVar = item->second;
     return;

  MU_CATCH

  *a_szName = 0;
  *a_pVar = 0;
}

//---------------------------------------------------------------------------
/** \brief Return the number of constants defined in a parser. */
API_EXPORT(int) mecGetConstNum(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    const valmap_type ValMap = p->GetConst();
    return (int)ValMap.size();
  MU_CATCH

  return 0; // never reached
}

//-----------------------------------------------------------------------------------------------------
API_EXPORT(void) mecSetArgSep(mecParserHandle_t a_hParser, const mecChar_t cArgSep)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->SetArgSep(cArgSep);
  MU_CATCH
}

//-----------------------------------------------------------------------------------------------------
API_EXPORT(void) mecResetLocale(mecParserHandle_t a_hParser)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->ResetLocale();
  MU_CATCH
}

//-----------------------------------------------------------------------------------------------------
API_EXPORT(void) mecSetDecSep(mecParserHandle_t a_hParser, const mecChar_t cDecSep)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->SetDecSep(cDecSep);
  MU_CATCH
}

//-----------------------------------------------------------------------------------------------------
API_EXPORT(void) mecSetThousandsSep(mecParserHandle_t a_hParser, const mecChar_t cThousandsSep)
{
  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    p->SetThousandsSep(cThousandsSep);
  MU_CATCH
}

//---------------------------------------------------------------------------
/** \brief Retrieve name and value of a single parser constant.
    \param a_hParser [in] a valid parser handle
    \param a_iVar [in] Index of the constant to query
    \param a_pszName [out] pointer to a null terminated string with the constant name
    \param [out] The constant value
*/
API_EXPORT(void) mecGetConst( mecParserHandle_t a_hParser, 
                              unsigned a_iVar,
                              const mecChar_t **a_pszName, 
                              mecFloat_t *a_fVal)
{
  // A static buffer is needed for the name since i cant return the
  // pointer from the map.
  static char  szName[1024];

  MU_TRY
    mecParser_t* const p(AsParser(a_hParser));
    const valmap_type ValMap = p->GetConst();

    if (a_iVar>=ValMap.size())
    {
     *a_pszName = 0;
     *a_fVal = 0;
      return;
    }

    valmap_type::const_iterator item;
    item = ValMap.begin();
    for (unsigned i=0; i<a_iVar; ++i)
      item++;

    strncpy(szName, item->first.c_str(), sizeof(szName));
    szName[sizeof(szName)-1] = 0;

    *a_pszName = &szName[0];
    *a_fVal = item->second;
     return;

  MU_CATCH

  *a_pszName = 0;
  *a_fVal = 0;
}

//---------------------------------------------------------------------------
/** \brief Add a custom value regognition function.
*/
API_EXPORT(void) mecAddValIdent(mecParserHandle_t a_hParser, 
                                mecIdentFun_t a_pFun)
{
  MU_TRY
    mecParser_t* p(AsParser(a_hParser));
    p->AddValIdent(a_pFun);
  MU_CATCH
}

//---------------------------------------------------------------------------
/** \brief Query if an error occured.

    After querying the internal error bit will be reset. So a consecutive call
    will return false.
*/
API_EXPORT(mecBool_t) mecError(mecParserHandle_t a_hParser)
{
  bool bError( AsParserTag(a_hParser)->bError );
  AsParserTag(a_hParser)->bError = false;
  return bError;
}

//---------------------------------------------------------------------------
/** \brief Reset the internal error flag.
*/
API_EXPORT(void) mecErrorReset(mecParserHandle_t a_hParser)
{
  AsParserTag(a_hParser)->bError = false;
}

//---------------------------------------------------------------------------
API_EXPORT(void) mecSetErrorHandler(mecParserHandle_t a_hParser, mecErrorHandler_t a_pHandler)
{
  AsParserTag(a_hParser)->errHandler = a_pHandler;
}

//---------------------------------------------------------------------------
/** \brief Return the message associated with the last error.
*/
API_EXPORT(const mecChar_t*) mecGetErrorMsg(mecParserHandle_t a_hParser)
{
  ParserTag* const p(AsParserTag(a_hParser));
  const char *pMsg = p->exc.GetMsg().c_str();

  // C# explodes when pMsg is returned directly. For some reason it can't access
  // the memory where the message lies directly.
//  static char szBuf[1024];
  sprintf(s_tmpOutBuf, "%s", pMsg);
  return s_tmpOutBuf;
}

//---------------------------------------------------------------------------
/** \brief Return the message associated with the last error.
*/
API_EXPORT(const mecChar_t*) mecGetErrorToken(mecParserHandle_t a_hParser)
{
  ParserTag* const p(AsParserTag(a_hParser));
  const char *pToken = p->exc.GetToken().c_str();

  // C# explodes when pMsg is returned directly. For some reason it can't access
  // the memory where the message lies directly.
//  static char szBuf[1024];
  sprintf(s_tmpOutBuf, "%s", pToken);
  return s_tmpOutBuf;
}

//---------------------------------------------------------------------------
/** \brief Return the code associated with the last error.
*/
API_EXPORT(int) mecGetErrorCode(mecParserHandle_t a_hParser)
{
  return AsParserTag(a_hParser)->exc.GetCode();
}

//---------------------------------------------------------------------------
/** \brief Return the postion associated with the last error. */
API_EXPORT(int) mecGetErrorPos(mecParserHandle_t a_hParser)
{
  return (int)AsParserTag(a_hParser)->exc.GetPos();
}

////-----------------------------------------------------------------------------------------------------
//API_EXPORT(const mecChar_t*) mecGetErrorExpr(mecParserHandle_t a_hParser)
//{
//  return AsParserTag(a_hParser)->exc.GetExpr().c_str();
//}

//-----------------------------------------------------------------------------------------------------
API_EXPORT(mecFloat_t*) mecCreateVar()
{
  return new mecFloat_t(0);
}

//-----------------------------------------------------------------------------------------------------
API_EXPORT(void) mecReleaseVar(mecFloat_t *ptr)
{
  delete ptr;
}

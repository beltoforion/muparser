/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
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
#ifndef MEC_PARSER_DLL_H
#define MEC_PARSER_DLL_H

#if defined(WIN32) || defined(_WIN32)
    #ifdef MUPARSERLIB_EXPORTS
        #define API_EXPORT(TYPE) __declspec(dllexport) TYPE __cdecl
    #else
        #define API_EXPORT(TYPE) __declspec(dllimport) TYPE __cdecl
    #endif
#else
    #define API_EXPORT(TYPE) TYPE
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/** \file 
    \brief This file contains the DLL interface of muparser.
*/

// Basic types
typedef void*  mecParserHandle_t;    // parser handle
typedef char   mecChar_t;            // character type
typedef int    mecBool_t;            // boolean type
typedef int    mecInt_t;             // integer type 
typedef float  mecFloat_t;           // floating point type

// function types for calculation
typedef mecFloat_t (*mecFun0_t)(); 
typedef mecFloat_t (*mecFun1_t)(mecFloat_t); 
typedef mecFloat_t (*mecFun2_t)(mecFloat_t, mecFloat_t); 
typedef mecFloat_t (*mecFun3_t)(mecFloat_t, mecFloat_t, mecFloat_t); 
typedef mecFloat_t (*mecFun4_t)(mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t); 
typedef mecFloat_t (*mecFun5_t)(mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t); 
typedef mecFloat_t (*mecFun6_t)(mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t); 
typedef mecFloat_t (*mecFun7_t)(mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t); 
typedef mecFloat_t (*mecFun8_t)(mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t); 
typedef mecFloat_t (*mecFun9_t)(mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t); 
typedef mecFloat_t (*mecFun10_t)(mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t, mecFloat_t); 

// Functions for parser management
typedef void        (*mecErrorHandler_t)(mecParserHandle_t a_hParser);             // [optional] callback to an error handler
typedef mecFloat_t  (*mecEvalFun_t)();
typedef mecFloat_t* (*mecFacFun_t)(const mecChar_t*, void*);                // [optional] callback for creating new variables
typedef mecInt_t    (*mecIdentFun_t)(const mecChar_t*, mecInt_t*, mecFloat_t*); // [optional] value identification callbacks


//-----------------------------------------------------------------------------------------------------
// Constants
#ifdef MUPARSERLIB_EXPORTS
__declspec(dllexport) extern int mecOPRT_ASCT_LEFT;
__declspec(dllexport) extern int mecOPRT_ASCT_RIGHT;

  // Error codes
__declspec(dllexport) extern int mecUNEXPECTED_OPERATOR;
__declspec(dllexport) extern int mecUNASSIGNABLE_TOKEN;
__declspec(dllexport) extern int mecUNEXPECTED_EOF;
__declspec(dllexport) extern int mecUNEXPECTED_ARG_SEP;
__declspec(dllexport) extern int mecUNEXPECTED_ARG;
__declspec(dllexport) extern int mecUNEXPECTED_VAL;
__declspec(dllexport) extern int mecUNEXPECTED_VAR;
__declspec(dllexport) extern int mecUNEXPECTED_PARENS;
__declspec(dllexport) extern int mecUNEXPECTED_STR;
__declspec(dllexport) extern int mecSTRING_EXPECTED;
__declspec(dllexport) extern int mecVAL_EXPECTED;
__declspec(dllexport) extern int mecMISSING_PARENS;
__declspec(dllexport) extern int mecUNEXPECTED_FUN;
__declspec(dllexport) extern int mecUNTERMINATED_STRING;
__declspec(dllexport) extern int mecTOO_MANY_PARAMS;
__declspec(dllexport) extern int mecTOO_FEW_PARAMS;
__declspec(dllexport) extern int mecOPRT_TYPE_CONFLICT;
__declspec(dllexport) extern int mecSTR_RESULT;
__declspec(dllexport) extern int mecINVALID_NAME;
__declspec(dllexport) extern int mecBUILTIN_OVERLOAD;
__declspec(dllexport) extern int mecINVALID_FUN_PTR;
__declspec(dllexport) extern int mecINVALID_VAR_PTR;
__declspec(dllexport) extern int mecEMPTY_EXPRESSION;
__declspec(dllexport) extern int mecNAME_CONFLICT;
__declspec(dllexport) extern int mecOPT_PRI;
__declspec(dllexport) extern int mecDOMAIN_ERROR;
__declspec(dllexport) extern int mecDIV_BY_ZERO;
__declspec(dllexport) extern int mecGENERIC;
__declspec(dllexport) extern int mecLOCALE;
__declspec(dllexport) extern int mecINTERNAL_ERROR;
__declspec(dllexport) extern int mecUNDEFINED;

#else
__declspec(dllimport) extern int mecOPRT_ASCT_LEFT;
__declspec(dllimport) extern int mecOPRT_ASCT_RIGHT;

  // Error codes
__declspec(dllimport) extern int mecUNEXPECTED_OPERATOR;
__declspec(dllimport) extern int mecUNASSIGNABLE_TOKEN;
__declspec(dllimport) extern int mecUNEXPECTED_EOF;
__declspec(dllimport) extern int mecUNEXPECTED_ARG_SEP;
__declspec(dllimport) extern int mecUNEXPECTED_ARG;
__declspec(dllimport) extern int mecUNEXPECTED_VAL;
__declspec(dllimport) extern int mecUNEXPECTED_VAR;
__declspec(dllimport) extern int mecUNEXPECTED_PARENS;
__declspec(dllimport) extern int mecUNEXPECTED_STR;
__declspec(dllimport) extern int mecSTRING_EXPECTED;
__declspec(dllimport) extern int mecVAL_EXPECTED;
__declspec(dllimport) extern int mecMISSING_PARENS;
__declspec(dllimport) extern int mecUNEXPECTED_FUN;
__declspec(dllimport) extern int mecUNTERMINATED_STRING;
__declspec(dllimport) extern int mecTOO_MANY_PARAMS;
__declspec(dllimport) extern int mecTOO_FEW_PARAMS;
__declspec(dllimport) extern int mecOPRT_TYPE_CONFLICT;
__declspec(dllimport) extern int mecSTR_RESULT;
__declspec(dllimport) extern int mecINVALID_NAME;
__declspec(dllimport) extern int mecBUILTIN_OVERLOAD;
__declspec(dllimport) extern int mecINVALID_FUN_PTR;
__declspec(dllimport) extern int mecINVALID_VAR_PTR;
__declspec(dllimport) extern int mecEMPTY_EXPRESSION;
__declspec(dllimport) extern int mecNAME_CONFLICT;
__declspec(dllimport) extern int mecOPT_PRI;
__declspec(dllimport) extern int mecDOMAIN_ERROR;
__declspec(dllimport) extern int mecDIV_BY_ZERO;
__declspec(dllimport) extern int mecGENERIC;
__declspec(dllimport) extern int mecLOCALE;
__declspec(dllimport) extern int mecINTERNAL_ERROR;
__declspec(dllimport) extern int mecUNDEFINED;
#endif

//-----------------------------------------------------------------------------------------------------
//
//
// mecParser C compatible bindings
//
//
//-----------------------------------------------------------------------------------------------------

// Basic operations / initialization 
API_EXPORT(void) mecDebugDump(int nDumpCmd, int nDumpStack);
API_EXPORT(void) mecSelfTest();
API_EXPORT(mecParserHandle_t) mecCreate();
API_EXPORT(float) mecEval(mecParserHandle_t a_hParser);
API_EXPORT(mecEvalFun_t) mecCompile(mecParserHandle_t a_hParser);
API_EXPORT(mecEvalFun_t) mecDbgCompile(mecParserHandle_t a_hParser, int nRegNum);
API_EXPORT(void) mecRelease(mecParserHandle_t a_hParser);
API_EXPORT(const mecChar_t*) mecGetExpr(mecParserHandle_t a_hParser);
API_EXPORT(void) mecSetExpr(mecParserHandle_t a_hParser, const mecChar_t *a_szExpr);
//API_EXPORT(void) mecSetVarFactory(mecParserHandle_t a_hParser, mecFacFun_t a_pFactory, void* pUserData);
API_EXPORT(const mecChar_t*) mecGetVersion(mecParserHandle_t a_hParser);

// Defining callbacks / variables / constants
API_EXPORT(void) mecDefineFun0(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun0_t a_pFun, mecBool_t a_bOptimize);
API_EXPORT(void) mecDefineFun1(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun1_t a_pFun, mecBool_t a_bOptimize);
API_EXPORT(void) mecDefineFun2(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun2_t a_pFun, mecBool_t a_bOptimize);
API_EXPORT(void) mecDefineFun3(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun3_t a_pFun, mecBool_t a_bOptimize);
API_EXPORT(void) mecDefineFun4(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun4_t a_pFun, mecBool_t a_bOptimize);
API_EXPORT(void) mecDefineFun5(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun5_t a_pFun, mecBool_t a_bOptimize);
API_EXPORT(void) mecDefineFun6(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun5_t a_pFun, mecBool_t a_bOptimize);
API_EXPORT(void) mecDefineFun7(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun5_t a_pFun, mecBool_t a_bOptimize);
API_EXPORT(void) mecDefineFun8(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun5_t a_pFun, mecBool_t a_bOptimize);
API_EXPORT(void) mecDefineFun9(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun5_t a_pFun, mecBool_t a_bOptimize);
API_EXPORT(void) mecDefineFun10(mecParserHandle_t a_hParser, const mecChar_t *a_szName, mecFun5_t a_pFun, mecBool_t a_bOptimize);

API_EXPORT(void) mecDefineOprt( mecParserHandle_t a_hParser, 
                                const mecChar_t* a_szName, 
                                mecFun2_t a_pFun, 
                                mecInt_t a_nPrec, 
                                mecInt_t a_nOprtAsct,
                                mecBool_t a_bOptimize);

API_EXPORT(void) mecDefineConst( mecParserHandle_t a_hParser, 
                                 const mecChar_t* a_szName, 
                                 mecFloat_t a_fVal );

API_EXPORT(void) mecDefineVar( mecParserHandle_t a_hParser, 
                               const mecChar_t* a_szName, 
                               mecFloat_t *a_fVar);

API_EXPORT(void) mecDefinePostfixOprt( mecParserHandle_t a_hParser, 
                                       const mecChar_t* a_szName, 
                                       mecFun1_t a_pOprt, 
                                       mecBool_t a_bOptimize);


API_EXPORT(void) mecDefineInfixOprt( mecParserHandle_t a_hParser, 
                                     const mecChar_t* a_szName, 
                                     mecFun1_t a_pOprt, 
                                     mecBool_t a_bOptimize);

// Define character sets for identifiers
API_EXPORT(void) mecDefineNameChars(mecParserHandle_t a_hParser, const mecChar_t* a_szCharset);
API_EXPORT(void) mecDefineOprtChars(mecParserHandle_t a_hParser, const mecChar_t* a_szCharset);
API_EXPORT(void) mecDefineInfixOprtChars(mecParserHandle_t a_hParser, const mecChar_t* a_szCharset);

// Remove all / single variables
API_EXPORT(void) mecRemoveVar(mecParserHandle_t a_hParser, const mecChar_t* a_szName);
API_EXPORT(void) mecClearVar(mecParserHandle_t a_hParser);
API_EXPORT(void) mecClearConst(mecParserHandle_t a_hParser);
API_EXPORT(void) mecClearOprt(mecParserHandle_t a_hParser);
API_EXPORT(void) mecClearFun(mecParserHandle_t a_hParser);

// Querying variables / expression variables / constants
API_EXPORT(int)  mecGetExprVarNum(mecParserHandle_t a_hParser);
API_EXPORT(int)  mecGetVarNum(mecParserHandle_t a_hParser);
API_EXPORT(int)  mecGetConstNum(mecParserHandle_t a_hParser);
API_EXPORT(void) mecGetExprVar(mecParserHandle_t a_hParser, unsigned a_iVar, const mecChar_t** a_pszName, mecFloat_t** a_pVar);
API_EXPORT(void) mecGetVar(mecParserHandle_t a_hParser, unsigned a_iVar, const mecChar_t** a_pszName, mecFloat_t** a_pVar);
API_EXPORT(void) mecGetConst(mecParserHandle_t a_hParser, unsigned a_iVar, const mecChar_t** a_pszName, mecFloat_t* a_pVar);
API_EXPORT(void) mecSetArgSep(mecParserHandle_t a_hParser, const mecChar_t cArgSep);
API_EXPORT(void) mecSetDecSep(mecParserHandle_t a_hParser, const mecChar_t cArgSep);
API_EXPORT(void) mecSetThousandsSep(mecParserHandle_t a_hParser, const mecChar_t cArgSep);
API_EXPORT(void) mecResetLocale(mecParserHandle_t a_hParser);

// Add value recognition callbacks
API_EXPORT(void) mecAddValIdent(mecParserHandle_t a_hParser, mecIdentFun_t);

// Error handling
API_EXPORT(mecBool_t) mecError(mecParserHandle_t a_hParser);
API_EXPORT(void) mecErrorReset(mecParserHandle_t a_hParser);
API_EXPORT(void) mecSetErrorHandler(mecParserHandle_t a_hParser, mecErrorHandler_t a_pErrHandler);
API_EXPORT(const mecChar_t*) mecGetErrorMsg(mecParserHandle_t a_hParser);
API_EXPORT(mecInt_t) mecGetErrorCode(mecParserHandle_t a_hParser);
API_EXPORT(mecInt_t) mecGetErrorPos(mecParserHandle_t a_hParser);
API_EXPORT(const mecChar_t*) mecGetErrorToken(mecParserHandle_t a_hParser);
//API_EXPORT(const mecChar_t*) mecGetErrorExpr(mecParserHandle_t a_hParser);

// This is used for .NET only. It creates a new variable allowing the dll to
// manage the variable rather than the .NET garbage collector.
API_EXPORT(mecFloat_t*) mecCreateVar();
API_EXPORT(void) mecReleaseVar(mecFloat_t*);

#ifdef __cplusplus
}
#endif

#endif // include guard

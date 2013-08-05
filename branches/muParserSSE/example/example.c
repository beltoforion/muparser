#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "muParserSSE.h"

#define PARSER_MAXVARS		10


//---------------------------------------------------------------------------
// Callbacks for postfix operators
mecFloat_t ZeroArg()
{
  printf("i'm a function without arguments.\n");
  return 123;
}

//---------------------------------------------------------------------------
// Callbacks for infix operators
mecFloat_t Not(mecFloat_t v) { return (mecFloat_t)(v==0); }

//---------------------------------------------------------------------------
// Function callbacks
mecFloat_t Rnd(mecFloat_t v) { return (mecFloat_t)(v * rand() / (double)(RAND_MAX+1.0)); }


//---------------------------------------------------------------------------
// Binarty operator callbacks
mecFloat_t Add(mecFloat_t v1, mecFloat_t v2) 
{ 
  return v1+v2; 
}

mecFloat_t Mul(mecFloat_t v1, mecFloat_t v2) 
{ 
  return v1*v2; 
}

//---------------------------------------------------------------------------
mecFloat_t DebugDump(mecFloat_t v1, mecFloat_t v2) 
{ 
  mecDebugDump((int)v1, (int)v2);
  printf("Bytecode dumping %s\n", ((int)v1!=0) ? "active" : "inactive");
  return 1; 
}


//---------------------------------------------------------------------------
void Intro(mecParserHandle_t hParser)
{
  printf("                 __________                                       \n");
  printf("    _____   __ __\\______   \\_____  _______  ______  ____ _______\n");
  printf("   /     \\ |  |  \\|     ___/\\__  \\ \\_  __ \\/  ___/_/ __ \\\\_  __ \\ \n");
  printf("  |  Y Y  \\|  |  /|    |     / __ \\_|  | \\/\\___ \\ \\  ___/ |  | \\/ \n");
  printf("  |__|_|  /|____/ |____|    (____  /|__|  /____  > \\___  >|__|    \n");
  printf("        \\/                       \\/            \\/      \\/         \n");
  printf("-------------  Math expression compiler ----------------------\n");
  printf("\n");
  printf("  muParserSSE - V %s\n", mecGetVersion(hParser));
  printf("  (C) 2011 Ingo Berg\n");
  printf("\n");
  printf("--------------------------------------------------------------\n");
  printf("Running test suite:\n");

#ifndef MEC_DUMP_CMDCODE
  mecSelfTest();
#else
  printf("  Unit test skipped\n");
#endif

  printf("--------------------------------------------------------------\n");
  printf("Commands:\n");
  printf("  list var     - list parser variables\n");
  printf("  list exprvar - list expression variables\n");
  printf("  list const   - list all numeric parser constants\n");
  printf("  locale de    - switch to german locale\n");
  printf("  locale en    - switch to english locale\n");
  printf("  locale reset - reset locale\n");
  printf("  quit         - exits the parser\n\n");
  printf("Constants:\n");
  printf("  \"_e\"   2.718281828459045235360287\n");
  printf("  \"_pi\"  3.141592653589793238462643\n");
  printf("--------------------------------------------------------------\n");
  printf("Please enter a formula:\n");
}

//---------------------------------------------------------------------------
// Callback function for parser errors
void OnError(mecParserHandle_t hParser)
{
  printf("\nError:\n");
  printf("------\n");
  printf("Message:  \"%s\"\n", mecGetErrorMsg(hParser));
  printf("Token:    \"%s\"\n", mecGetErrorToken(hParser));
  printf("Position: %d\n", mecGetErrorPos(hParser));
  printf("Errc:     %d\n", mecGetErrorCode(hParser));
}

//---------------------------------------------------------------------------
void ListVar(mecParserHandle_t a_hParser)
{
  int iNumVar = mecGetVarNum(a_hParser);
  int i = 0;

  if (iNumVar==0)
  {
    printf("No variables defined\n");
    return;
  }

  printf("\nExpression variables:\n");
  printf("---------------------\n");
  printf("Number: %d\n", iNumVar);
  
  for (i=0; i<iNumVar; ++i)
  {
    const mecChar_t* szName = 0;
    mecFloat_t* pVar = 0;

    mecGetVar(a_hParser, i, &szName, &pVar);
    printf("Name: %s    Address: [0x%x]\n", szName, (long long)pVar);
  }
}

//---------------------------------------------------------------------------
void ListExprVar(mecParserHandle_t a_hParser)
{
  mecInt_t iNumVar = mecGetExprVarNum(a_hParser),
          i = 0;

  if (iNumVar==0)
  {
    printf("Expression dos not contain variables\n");
    return;
  }

  printf("\nExpression variables:\n");
  printf("---------------------\n");
  printf("Expression: %s\n", mecGetExpr(a_hParser) );
  printf("Number: %d\n", iNumVar);
  
  for (i=0; i<iNumVar; ++i)
  {
    const mecChar_t* szName = 0;
    mecFloat_t* pVar = 0;

    mecGetExprVar(a_hParser, i, &szName, &pVar);
    printf("Name: %s   Address: [0x%x]\n", szName, (long long)pVar);
  }
}

//---------------------------------------------------------------------------
void ListConst(mecParserHandle_t a_hParser)
{
  mecInt_t iNumVar = mecGetConstNum(a_hParser),
          i = 0;

  if (iNumVar==0)
  {
    printf("No constants defined\n");
    return;
  }

  printf("\nParser constants:\n");
  printf("---------------------\n");
  printf("Number: %d", iNumVar);

  for (i=0; i<iNumVar; ++i)
  {
    const mecChar_t* szName = 0;
    mecFloat_t fVal = 0;

    mecGetConst(a_hParser, i, &szName, &fVal);
    printf("  %s = %f\n", szName, fVal);
  }
}

//---------------------------------------------------------------------------
/** \brief Check for external keywords.
*/
int CheckKeywords(const char *a_szLine, mecParserHandle_t a_hParser)
{
  if (!strcmp(a_szLine, "quit"))
  {
    return -1;
  }
  else if (!strcmp(a_szLine,"list var"))
  {
    ListVar(a_hParser);
    return 1;
  }
  else if (!strcmp(a_szLine, "list exprvar"))
  {
    ListExprVar(a_hParser);
    return 1;
  }
  else if (!strcmp(a_szLine, "list const"))
  {
    ListConst(a_hParser);
    return 1;
  }
  else if (!strcmp(a_szLine, "locale de"))
  {
    printf("Setting german locale: ArgSep=';' DecSep=',' ThousandsSep='.'\n");
    mecSetArgSep(a_hParser, ';');
    mecSetDecSep(a_hParser, ',');
    mecSetThousandsSep(a_hParser, '.');
    return 1;
  }
  else if (!strcmp(a_szLine, "locale en"))
  {
    printf("Setting english locale: ArgSep=',' DecSep='.' ThousandsSep=''\n");
    mecSetArgSep(a_hParser, ',');
    mecSetDecSep(a_hParser, '.');
    mecSetThousandsSep(a_hParser, 0);
    return 1;
  }
  else if (!strcmp(a_szLine, "locale reset"))
  {
    printf("Resetting locale\n");
    mecResetLocale(a_hParser);
    return 1;
  }

  return 0;
}

//---------------------------------------------------------------------------
void Calc()
{
  mecChar_t szLine[100];
  mecFloat_t fVal = 0,
            afVarVal[] = { 1, 2, 7.2f, -2.1f }; // Values of the parser variables
  mecParserHandle_t hParser;
  mecEvalFun_t pFunEval = NULL;

  hParser = mecCreate();              // initialize the parser
  Intro(hParser);

  // Set an error handler [optional]
  // the only function that does not take a parser instance handle
  mecSetErrorHandler(hParser, OnError);

//#define GERMAN_LOCALS
#ifdef GERMAN_LOCALS
  mecSetArgSep(hParser, ';');
  mecSetDecSep(hParser, ',');
  mecSetThousandsSep(hParser, '.');
#else
  mecSetArgSep(hParser, ',');
  mecSetDecSep(hParser, '.');
#endif

  // Define parser variables and bind them to C++ variables [optional]
  mecDefineConst(hParser, "const1", 1);  
  mecDefineConst(hParser, "const2", 2);

  // Define parser variables and bind them to C++ variables [optional]
  mecDefineVar(hParser, "a", &afVarVal[0]);  
  mecDefineVar(hParser, "b", &afVarVal[1]);
  mecDefineVar(hParser, "c", &afVarVal[2]);  
  mecDefineVar(hParser, "d", &afVarVal[3]);

  // Define infix operator [optional]
  mecDefineInfixOprt(hParser, "!", Not, 0);

  // Define functions [optional]
  mecDefineFun0(hParser, "zero", ZeroArg, 0);
  mecDefineFun1(hParser, "rnd", Rnd, 0);             // Add an unoptimizeable function
  mecDefineFun2(hParser, "dump", DebugDump, 0);

  // Define binary operators [optional]
  mecDefineOprt(hParser, "add", Add, 0, mecOPRT_ASCT_LEFT, 0);
  mecDefineOprt(hParser, "mul", Mul, 1, mecOPRT_ASCT_LEFT, 0);
 
#ifdef _DEBUG
  mecDebugDump(1, 0);
#endif

  while ( fgets(szLine, 99, stdin) )
  {
    szLine[strlen(szLine)-1] = 0; // overwrite the newline
/*
    if (!strcmp("test", szLine))
    {
//    strcpy(szLine, "1 ? 0 ? 128 : 255 : 1 ? 32 : 64"); // = 255
//    strcpy(szLine, "1 ? 2 : 3 ? 4 : 5");               // = 2  
      strcpy(szLine, "1>0 ? 1>0 ? 128 : 255 : 50");
      printf("%s\n", szLine);
    }
*/
    switch(CheckKeywords(szLine, hParser))
    {
    case  0:  break;       // no keyword found; parse the line
    case  1:  continue;    // A Keyword was found do not parse the line
    case -1:  return;      // abort the application
    }

    // Set the expression
    mecSetExpr(hParser, szLine);


    // Compile the expression and get the pointer to the 
    // just in time compiled eval function
    pFunEval = mecDbgCompile(hParser, -1);
    if (pFunEval==NULL)
      continue;
    // calculate the expression
    fVal = pFunEval();

/* alternative:
    fVal = mecEval(hParser); // 1st time parse from string and compile expression
    fVal = mecEval(hParser); // 2nd time parse from JIT code (handled internally)
*/
    // Without an Error handler function 
    // you must use this for error treatment:
    //if (mecError(hParser))
    //{
    //  printf("\nError:\n");
    //  printf("------\n");
    //  printf("Message:  %s\n", mecGetErrorMsg(hParser) );
    //  printf("Token:    %s\n", mecGetErrorToken(hParser) );
    //  printf("Position: %s\n", mecGetErrorPos(hParser) );
    //  printf("Errc:     %s\n", mecGetErrorCode(hParser) );
    //  continue;
    //}

    if (!mecError(hParser))
      printf("%f\n", fVal);

  } // while 

  // finalle free the parser ressources
  mecRelease(hParser);
}

//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  Calc();
  printf("done...");
}

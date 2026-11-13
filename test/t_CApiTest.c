/*
	Regression tests for the C API (muParserDLL).

	https://github.com/beltoforion/muparser/issues/182:
	Passing a null parser handle, a null string or a null pointer argument
	to the C API must not crash. Calls with a null handle are ignored and
	return default values, invalid arguments are reported through the
	regular error mechanism (mupError / mupGetErrorCode).
*/

#include <stdio.h>
#include <stddef.h>

#include "muParserDLL.h"

#ifndef _UNICODE
	#define _T(x) x
#else
	#define _T(x) L##x
#endif

/* error codes from mu::EErrorCodes (muParserDef.h) */
#define EC_INVALID_FUN_PTR 23
#define EC_INVALID_VAR_PTR 24
#define EC_INVALID_ARG     30   /* ecGENERIC */

static int s_nFail = 0;

#define CHECK(cond)                                                        \
	do {                                                                   \
		if (!(cond)) {                                                     \
			printf("  fail (line %d): %s\n", __LINE__, #cond);             \
			++s_nFail;                                                     \
		}                                                                  \
	} while (0)

static muFloat_t Identity(muFloat_t v) { return v; }

static int s_nErrHandlerCalls = 0;
static void ErrHandler(muParserHandle_t h) { (void)h; ++s_nErrHandlerCalls; }

/* An invalid argument must set the error flag and the given error code. */
static void ExpectError(muParserHandle_t h, int nCode, int nLine)
{
	int nActualCode = mupGetErrorCode(h);
	if (!mupError(h))
	{
		printf("  fail (line %d): error flag not set\n", nLine);
		++s_nFail;
	}
	else if (nActualCode != nCode)
	{
		printf("  fail (line %d): error code %d (expected %d)\n", nLine, nActualCode, nCode);
		++s_nFail;
	}
}

#define EXPECT_ERROR(h, code) ExpectError(h, code, __LINE__)

static void TestNullHandle(void)
{
	muFloat_t fVal = 1;
	muFloat_t fBuf[2] = { 0, 0 };
	int nNum = 0;
	const muChar_t* szName = NULL;
	muFloat_t* pVar = NULL;

	printf("testing null parser handle...\n");

	/* none of these calls may crash */
	mupSetExpr(NULL, _T("1+1"));
	mupDefineVar(NULL, _T("a"), &fVal);
	mupDefineConst(NULL, _T("c"), 1);
	mupDefineStrConst(NULL, _T("s"), _T("str"));
	mupDefineFun1(NULL, _T("f"), Identity, 1);
	mupDefinePostfixOprt(NULL, _T("m"), Identity, 1);
	mupDefineInfixOprt(NULL, _T("!"), Identity, 0, 1);
	mupDefineNameChars(NULL, _T("abc"));
	mupDefineOprtChars(NULL, _T("+-"));
	mupDefineInfixOprtChars(NULL, _T("!"));
	mupRemoveVar(NULL, _T("a"));
	mupClearVar(NULL);
	mupClearConst(NULL);
	mupClearOprt(NULL);
	mupClearFun(NULL);
	mupSetArgSep(NULL, ',');
	mupSetDecSep(NULL, '.');
	mupSetThousandsSep(NULL, 0);
	mupResetLocale(NULL);
	mupSetVarFactory(NULL, NULL, NULL);
	mupAddValIdent(NULL, NULL);
	mupEvalBulk(NULL, fBuf, 2);
	mupGetVar(NULL, 0, &szName, &pVar);
	mupGetExprVar(NULL, 0, &szName, &pVar);
	mupGetConst(NULL, 0, &szName, &fVal);
	mupSetErrorHandler(NULL, ErrHandler);
	mupErrorReset(NULL);
	mupRelease(NULL);

	CHECK(mupEval(NULL) == 0);
	CHECK(mupEvalMulti(NULL, &nNum) == NULL);
	CHECK(mupGetVarNum(NULL) == 0);
	CHECK(mupGetExprVarNum(NULL) == 0);
	CHECK(mupGetConstNum(NULL) == 0);
	CHECK(mupGetExpr(NULL) != NULL && mupGetExpr(NULL)[0] == 0);
	CHECK(mupGetVersion(NULL) != NULL && mupGetVersion(NULL)[0] == 0);
	CHECK(mupError(NULL) == 0);
	CHECK(mupGetErrorMsg(NULL) != NULL && mupGetErrorMsg(NULL)[0] == 0);
	CHECK(mupGetErrorToken(NULL) != NULL && mupGetErrorToken(NULL)[0] == 0);
	CHECK(mupGetErrorPos(NULL) == -1);
	CHECK(mupGetErrorCode(NULL) == -1);

	CHECK(szName == NULL);
	CHECK(pVar == NULL);
	CHECK(s_nErrHandlerCalls == 0);
}

static void TestNullArguments(void)
{
	muFloat_t fVal = 1;
	muParserHandle_t h = mupCreate(muBASETYPE_FLOAT);

	printf("testing null arguments...\n");

	if (h == NULL)
	{
		printf("  fail: mupCreate returned NULL\n");
		++s_nFail;
		return;
	}

	mupSetErrorHandler(h, ErrHandler);
	s_nErrHandlerCalls = 0;

	mupSetExpr(h, NULL);
	EXPECT_ERROR(h, EC_INVALID_ARG);
	CHECK(s_nErrHandlerCalls == 1);

	mupDefineVar(h, NULL, &fVal);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	mupDefineVar(h, _T("a"), NULL);
	EXPECT_ERROR(h, EC_INVALID_VAR_PTR);

	mupDefineConst(h, NULL, 1);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	mupDefineStrConst(h, _T("s"), NULL);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	mupDefineFun1(h, _T("f"), NULL, 1);
	EXPECT_ERROR(h, EC_INVALID_FUN_PTR);

	mupDefineFun1(h, NULL, Identity, 1);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	mupDefinePostfixOprt(h, _T("m"), NULL, 1);
	EXPECT_ERROR(h, EC_INVALID_FUN_PTR);

	mupDefineNameChars(h, NULL);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	mupRemoveVar(h, NULL);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	mupAddValIdent(h, NULL);
	EXPECT_ERROR(h, EC_INVALID_FUN_PTR);

	mupSetExpr(h, _T("1+2"));
	CHECK(!mupError(h));

	mupEvalBulk(h, NULL, 2);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	CHECK(mupEvalMulti(h, NULL) == NULL);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	mupGetVar(h, 0, NULL, NULL);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	mupGetExprVar(h, 0, NULL, NULL);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	mupGetConst(h, 0, NULL, NULL);
	EXPECT_ERROR(h, EC_INVALID_ARG);

	/* the parser must still be usable afterwards */
	mupSetExpr(h, _T("1+2"));
	CHECK(mupEval(h) == 3);
	CHECK(!mupError(h));

	mupRelease(h);
}

int main(void)
{
	TestNullHandle();
	TestNullArguments();

	if (s_nFail == 0)
		printf("C API tests passed\n");
	else
		printf("C API tests failed with %d errors\n", s_nFail);

	return s_nFail == 0 ? 0 : 1;
}

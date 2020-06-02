/*
				 __________
	_____   __ __\______   \_____  _______  ______  ____ _______
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|
		\/                       \/            \/      \/
  Copyright (C) 2004 - 2020 Ingo Berg

	Redistribution and use in source and binary forms, with or without modification, are permitted
	provided that the following conditions are met:

	  * Redistributions of source code must retain the above copyright notice, this list of
		conditions and the following disclaimer.
	  * Redistributions in binary form must reproduce the above copyright notice, this list of
		conditions and the following disclaimer in the documentation and/or other materials provided
		with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
	IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
	OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MU_PARSER_H
#define MU_PARSER_H

//--- Standard includes ------------------------------------------------------------------------
#include <vector>

//--- Parser includes --------------------------------------------------------------------------
#include "muParserBase.h"
#include "muParserTemplateMagic.h"

/** \file
	\brief Definition of the standard floating point parser.
*/

namespace mu
{
	/** \brief Mathematical expressions parser.

	  Standard implementation of the mathematical expressions parser.
	  Can be used as a reference implementation for subclassing the parser.

	  <small>
	  (C) 2011 Ingo Berg<br>
	  muparser(at)beltoforion.de
	  </small>
	*/
	/* final */ class API_EXPORT_CXX Parser : public ParserBase
	{
	public:

		Parser();

		virtual void InitCharSets();
		virtual void InitFun();
		virtual void InitConst();
		virtual void InitOprt();
		virtual void OnDetectVar(string_type* pExpr, int& nStart, int& nEnd);

		value_type Diff(value_type* a_Var,
			value_type a_fPos,
			value_type a_fEpsilon = 0) const;

	protected:

		// Trigonometric functions
		static value_type  Sin(value_type);
		static value_type  Cos(value_type);
		static value_type  Tan(value_type);
		static value_type  Tan2(value_type, value_type);
		// arcus functions
		static value_type  ASin(value_type);
		static value_type  ACos(value_type);
		static value_type  ATan(value_type);
		static value_type  ATan2(value_type, value_type);

		// hyperbolic functions
		static value_type  Sinh(value_type);
		static value_type  Cosh(value_type);
		static value_type  Tanh(value_type);
		// arcus hyperbolic functions
		static value_type  ASinh(value_type);
		static value_type  ACosh(value_type);
		static value_type  ATanh(value_type);
		// Logarithm functions
		static value_type  Log2(value_type);  // Logarithm Base 2
		static value_type  Log10(value_type); // Logarithm Base 10
		static value_type  Ln(value_type);    // Logarithm Base e (natural logarithm)
		// misc
		static value_type  Exp(value_type);
		static value_type  Abs(value_type);
		static value_type  Sqrt(value_type);
		static value_type  Rint(value_type);
		static value_type  Sign(value_type);

		// Prefix operators
		// !!! Unary Minus is a MUST if you want to use negative signs !!!
		static value_type  UnaryMinus(value_type);
		static value_type  UnaryPlus(value_type);

		// Functions with variable number of arguments
		static value_type Sum(const value_type*, int);  // sum
		static value_type Avg(const value_type*, int);  // mean value
		static value_type Min(const value_type*, int);  // minimum
		static value_type Max(const value_type*, int);  // maximum

		static int IsVal(const char_type* a_szExpr, int* a_iPos, value_type* a_fVal);
	};
} // namespace mu

#endif


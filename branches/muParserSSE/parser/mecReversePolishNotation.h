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
#ifndef MEC_REVERSE_POLISH_NOTATION_H
#define MEC_REVERSE_POLISH_NOTATION_H

#include <cassert>
#include <string>
#include <stack>
#include <vector>

#include "mecDef.h"
#include "mecError.h"
#include "mecToken.h"

/** \file
    \brief This file contains the definition of the parsere reverse polish notation implementation.
*/


namespace mec
{
  /** \brief Bytecode implementation of the Math Parser.
      \author (C) 2011 Ingo Berg 

    The bytecode contains the formula converted to revers polish notation stored in a continious
    memory area. Associated with this data are operator codes, variable pointers, constant 
    values and function pointers. Those are necessary in order to calculate the result.
    All those data items will be casted to the underlying datatype of the bytecode.
  */
  class ReversePolishNotation
  {
  private:

      typedef std::vector<SPackedToken> rpn_type;

      /** \brief Position in the Calculation array. */
      unsigned m_iStackPos;

      /** \brief Maximum size needed for the stack. */
      std::size_t m_iMaxStackSize;

      /** \brief A vector of packed tokens representing the RPN. */
      rpn_type m_vRPN;

  public:

      ReversePolishNotation();
      ReversePolishNotation(const ReversePolishNotation &a_RPN);
      ReversePolishNotation& operator=(const ReversePolishNotation &a_RPN);
      void Assign(const ReversePolishNotation &a_RPN);

      void AddVar(value_type *a_pVar);
      void AddVal(value_type a_fVal);
      void AddOp(ECmdCode a_Oprt);
      void AddFun(void *a_pFun, int a_iArgc);
      void AddFun(ECmdCode a_Oprt);
      void AddIfElse(ECmdCode a_Oprt);

      void Finalize();
      void clear();
      std::size_t GetMaxStackSize() const;
      SPackedToken* GetRPNBasePtr();

      void RemoveValEntries(unsigned a_iNumber);
      void AsciiDump();
  };
} // namespace mec

#endif


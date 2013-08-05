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

#include "mecReversePolishNotation.h"

#include <cassert>
#include <string>
#include <stack>
#include <vector>
#include <iostream>

#include "mecDef.h"
#include "mecToken.h"
#include "mecError.h"
#include "mecStack.h"

/** \file
    \brief Implementation of the parser bytecode class.
*/


namespace mec
{
  //---------------------------------------------------------------------------
  /** Bytecode default constructor. */
  ReversePolishNotation::ReversePolishNotation()
    :m_iStackPos(0)
    ,m_iMaxStackSize(0)
    ,m_vRPN()
  {
    m_vRPN.reserve(200);
  }

  //---------------------------------------------------------------------------
  /** \brief Copy constructor. 
    
      Implemented in Terms of Assign(const ByteCode &a_ByteCode)
  */
  ReversePolishNotation::ReversePolishNotation(const ReversePolishNotation &a_RPN)
  {
    Assign(a_RPN);
  }

  //---------------------------------------------------------------------------
  /** \brief Assignment operator.
    
      Implemented in Terms of Assign(const ByteCode &a_ByteCode)
  */
  ReversePolishNotation& ReversePolishNotation::operator=(const ReversePolishNotation &a_RPN)
  {
    Assign(a_RPN);
    return *this;
  }

  //---------------------------------------------------------------------------
  /** \brief Copy state of another object to this. 
    
      \throw nowthrow
  */
  void ReversePolishNotation::Assign(const ReversePolishNotation &a_ByteCode)
  {
    if (this==&a_ByteCode)    
      return;

    m_iStackPos = a_ByteCode.m_iStackPos;
    m_vRPN = a_ByteCode.m_vRPN;
    m_iMaxStackSize = a_ByteCode.m_iMaxStackSize;
  }

  //---------------------------------------------------------------------------
  /** \brief Add a Variable pointer to bytecode. 
      \param a_pVar Pointer to be added.
      \throw nothrow
  */
  void ReversePolishNotation::AddVar(value_type *a_pVar)
  {
    SPackedToken tok;
    memset(&tok, 0, sizeof(SPackedToken));
    tok.m_nStackPos = ++m_iStackPos;
    tok.m_eCode = cmVAR;
    tok.m_pVar = a_pVar;

    m_vRPN.push_back(tok);
    m_iMaxStackSize = std::max(m_iMaxStackSize, (size_t)m_iStackPos);
  }

  //---------------------------------------------------------------------------
  /** \brief Add a Variable pointer to bytecode. 

      Value entries in byte code consist of:
      <ul>
        <li>value array position of the value</li>
        <li>the operator code according to Token::cmVAL</li>
        <li>the value stored in #mc_iSizeVal number of bytecode entries.</li>
      </ul>

      \param a_pVal Value to be added.
      \throw nothrow
  */
  void ReversePolishNotation::AddVal(value_type a_fVal)
  {
    SPackedToken tok;
    memset(&tok, 0, sizeof(SPackedToken));
    tok.m_nStackPos = ++m_iStackPos;
    tok.m_eCode     = cmVAL;
    tok.m_fVal      = a_fVal;

    m_vRPN.push_back(tok);
    m_iMaxStackSize = std::max(m_iMaxStackSize, (size_t)m_iStackPos);
  }

  //---------------------------------------------------------------------------
  void ReversePolishNotation::AddIfElse(ECmdCode a_Oprt)
  {
    SPackedToken tok;

    tok.m_nStackPos = m_iStackPos;

    if (a_Oprt==cmELSE || a_Oprt==cmIF)
      --m_iStackPos;
    
    tok.m_eCode = a_Oprt;
    m_vRPN.push_back(tok);
  }

  //---------------------------------------------------------------------------
  /** \brief Add an operator identifier to bytecode. 
    
      Operator entries in byte code consist of:
      <ul>
        <li>value array position of the result</li>
        <li>the operator code according to Token::ECmdCode</li>
      </ul>

      \sa  Token::ECmdCode
  */
  void ReversePolishNotation::AddOp(ECmdCode a_Oprt)
  {
    SPackedToken tok;
    memset(&tok, 0, sizeof(SPackedToken));
    tok.m_nStackPos = --m_iStackPos;
    tok.m_eCode = a_Oprt;
    m_vRPN.push_back(tok);
  }

  //---------------------------------------------------------------------------
  void ReversePolishNotation::AddFun(ECmdCode a_fun)
  {
    SPackedToken tok;
    memset(&tok, 0, sizeof(SPackedToken));
    tok.m_nStackPos = m_iStackPos;
    tok.m_eCode = a_fun;
    m_vRPN.push_back(tok);
  }

  //---------------------------------------------------------------------------
  /** \brief Add function to bytecode. 

      \param a_iArgc Number of arguments, negative numbers indicate multiarg functions.
      \param a_pFun Pointer to function callback.
  */
  void ReversePolishNotation::AddFun(void *a_pFun, int a_iArgc)
  {
    if (a_iArgc>=0)
      m_iStackPos = m_iStackPos - a_iArgc + 1; 
    else
      m_iStackPos = m_iStackPos + a_iArgc + 1; 

    SPackedToken tok;
    memset(&tok, 0, sizeof(SPackedToken));
    tok.m_nStackPos = m_iStackPos;
    tok.m_eCode     = cmFUNC;
    tok.Fun.m_nArgc = a_iArgc;
    tok.Fun.m_pFun  = a_pFun;

    m_vRPN.push_back(tok);
    m_iMaxStackSize = std::max(m_iMaxStackSize, (size_t)m_iStackPos);
  }

  //---------------------------------------------------------------------------
  /** \brief Add end marker to bytecode.
      
      \throw nothrow 
  */
  void ReversePolishNotation::Finalize()
  {
    SPackedToken tok;
    memset(&tok, 0, sizeof(SPackedToken));
    tok.m_eCode = cmEND;
    m_vRPN.push_back(tok);
    rpn_type(m_vRPN).swap(m_vRPN);

    // Determine the if-then-else jump offsets
    Stack<int> stIf, stElse;
    int idx;
    for (std::size_t i=0; i<m_vRPN.size(); ++i)
    {
      switch(m_vRPN[i].m_eCode)
      {
      case cmIF:
            stIf.push(i);
            break;

      case  cmELSE:
            stElse.push(i);
            idx = stIf.pop();
            m_vRPN[idx].Jmp.offset = i - idx;
            break;
      
      case cmENDIF:
            idx = stElse.pop();
            m_vRPN[idx].Jmp.offset = i - idx;
            break;
      }
    }
  }

  //---------------------------------------------------------------------------
  /** \brief Get Pointer to the rpn. */
  SPackedToken* ReversePolishNotation::GetRPNBasePtr()
  {
    assert(m_vRPN.size());
    return &m_vRPN[0];
  }

  //---------------------------------------------------------------------------
  std::size_t ReversePolishNotation::GetMaxStackSize() const
  {
    return m_iMaxStackSize+1;
  }

  //---------------------------------------------------------------------------
  /** \brief Delete the bytecode. 
  
      \throw nothrow

      The name of this function is a violation of my own coding guidelines
      but this way it's more in line with the STL functions thus more 
      intuitive.
  */
  void ReversePolishNotation::clear()
  {
    m_vRPN.clear();
    m_iStackPos = 0;
    m_iMaxStackSize = 0;
  }

  //---------------------------------------------------------------------------
  /** \brief Remove a value number of entries from the bytecode. 
    
      \attention Currently I don't test if the entries are really value entries.
  */
  void ReversePolishNotation::RemoveValEntries(unsigned a_iNumber)
  {
    m_vRPN.resize(m_vRPN.size() - a_iNumber);
    m_iStackPos -= (a_iNumber);
  }

  //---------------------------------------------------------------------------
  /** \brief Dump bytecode (for debugging only!). */
  void ReversePolishNotation::AsciiDump()
  {
    if (!m_vRPN.size()) 
    {
      console() << "No reverse polish notation available\n";
      return;
    }

    console() << "Entries:" << (int)m_vRPN.size()
              << " (Tokensize:" << sizeof(SPackedToken) << ")\n";
    for (std::size_t i=0; i<m_vRPN.size(); ++i)
    {
      SPackedToken tok = m_vRPN[i];        
      console() << i << ": ";

      if (tok.m_nStackPos!=-1)
      {
        console() << "Stack[" << tok.m_nStackPos << "]\t";
      }
      else
      {
        console() << "      \t";
      }

      switch (tok.m_eCode)
      {
        case cmMIN:   console() << "MIN\n";  break;
        case cmMAX:   console() << "MAX\n";  break;
        case cmLT:    console() << "LT\n";   break;
        case cmGT:    console() << "GT\n";   break;
        case cmLE:    console() << "LE\n";   break;
        case cmGE:    console() << "GE\n";   break;
        case cmEQ:    console() << "EQ\n";   break;
        case cmNEQ:   console() << "NEQ\n";  break;
        case cmAND:   console() << "AND\n";  break;
        case cmOR:    console() << "OR\n";   break;
        case cmADD:   console() << "ADD\n";  break;
        case cmSUB:   console() << "SUB\n";  break;
        case cmMUL:   console() << "MUL\n";  break;
        case cmDIV:   console() << "DIV\n";  break;
//        case cmMOD:   console() << "MOD\n";  break;
        case cmSIN:   console() << "SIN\n";  break;
        case cmCOS:   console() << "COS\n";  break;
        case cmTAN:   console() << "TAN\n";  break;
        case cmABS:   console() << "ABS\n";  break;
        case cmSQRT:  console() << "SQRT\n"; break;

        case cmVAL:   console() << "VAL\t";
                      console() << "[" << tok.m_fVal << "]\n";
                      break;

        case cmVAR:   console() << "VAR\t";
  	                  console() << "[ADDR: 0x" << std::hex << tok.m_pVar << "]\n"; 
                      break;
      			
        case cmFUNC:  console() << "FUN\t";
                      console() << "[ARG:" << tok.Fun.m_nArgc << "]";
                      console() << "[ADDR: 0x" << tok.Fun.m_pFun << "]\n"; 
                      break;

        case cmEND:   console() << "END\n"; 
                      break;

        case cmIF:    console() << _T("IF\t");
                      console() << _T("[OFFSET:") << std::dec << m_vRPN[i].Jmp.offset << _T("]\n");
                      break;

        case cmELSE:  console() << _T("ELSE\t");
                      console() << _T("[OFFSET:") << std::dec << m_vRPN[i].Jmp.offset << _T("]\n");
                      break;

        case cmENDIF: console() << _T("ENDIF\n"); break;

        default:    console() << "(unknown code: " << tok.m_eCode << ")\n"; 
                    break;
      } // switch cmdCode
    }
  }
} // namespace mec

/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
  Copyright (C) 2004-2012 Ingo Berg

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
#ifndef MU_PARSER_BYTECODE_H
#define MU_PARSER_BYTECODE_H

#include <vector>

#include "muParserDef.h"


MUP_NAMESPACE_START

  //-----------------------------------------------------------------------------------------------
  /** \brief Implementation of the parser bytecode.

    The bytecode is a vector of tokens obtained after compiling the expression.
  */
  template<typename TValue, typename TString>
  class ParserByteCode
  {
  private:
      
      typedef Token<TValue, TString> token_type;
      typedef std::vector< token_type > rpn_type;
      typedef std::basic_stringstream<typename TString::value_type,
                                      std::char_traits<typename TString::value_type>,  
                                      std::allocator<typename TString::value_type> > stringstream_type;
      typedef void (MUP_FASTCALL *fun_type)(TValue*, int narg);

      unsigned m_iStackPos;
      int m_nFoldableValues;
      std::size_t m_iMaxStackSize;
      rpn_type  m_vRPN;
      bool m_bEnableOptimizer;

      //-------------------------------------------------------------------------------------------
      static void MUP_FASTCALL FUN_AA(TValue *arg, int)  { arg[0] += arg[1] + arg[2]; }
      static void MUP_FASTCALL FUN_AS(TValue *arg, int)  { arg[0] -= arg[1] + arg[2]; }
      static void MUP_FASTCALL FUN_MA(TValue *arg, int)  { arg[0] += arg[1] * arg[2]; }
      static void MUP_FASTCALL FUN_AM(TValue *arg, int)  { arg[0] *= arg[1] + arg[2]; }
      static void MUP_FASTCALL FUN_MM(TValue *arg, int)  { arg[0] *= arg[1] * arg[2]; }
      static void MUP_FASTCALL FUN_DD(TValue *arg, int)  { arg[0] /= arg[1] / arg[2]; }
      static void MUP_FASTCALL FUN_MD(TValue *arg, int)  { arg[0] /= arg[1] * arg[2]; }
      static void MUP_FASTCALL FUN_DM(TValue *arg, int)  { arg[0] *= arg[1] / arg[2]; }
      static void MUP_FASTCALL FUN_DA(TValue *arg, int)  { arg[0] += arg[1] / arg[2]; }
      static void MUP_FASTCALL FUN_AD(TValue *arg, int)  { arg[0] /= arg[1] + arg[2]; }
      static void MUP_FASTCALL FUN_DS(TValue *arg, int)  { arg[0] -= arg[1] / arg[2]; }
      static void MUP_FASTCALL FUN_SD(TValue *arg, int)  { arg[0] /= arg[1] - arg[2]; }

      static void MUP_FASTCALL FUN_P2(TValue *arg, int)  { arg[0] *= arg[0]; }
      static void MUP_FASTCALL FUN_P3(TValue *arg, int)  { arg[0] *= arg[0] * arg[0]; }
      static void MUP_FASTCALL FUN_P4(TValue *arg, int)  { arg[0] *= arg[0] * arg[0] * arg[0]; }
      static void MUP_FASTCALL FUN_P5(TValue *arg, int)  { arg[0] *= arg[0] * arg[0] * arg[0] * arg[0]; }

      static void MUP_FASTCALL FUN_P2M(TValue *arg, int) { arg[0] *= arg[1] * arg[1]; }
      static void MUP_FASTCALL FUN_P3M(TValue *arg, int) { arg[0] *= arg[1] * arg[1] * arg[1]; }
      static void MUP_FASTCALL FUN_P4M(TValue *arg, int) { arg[0] *= arg[1] * arg[1] * arg[1] * arg[1]; }

      static void MUP_FASTCALL FUN_P2A(TValue *arg, int) { arg[0] += arg[1] * arg[1]; }
      static void MUP_FASTCALL FUN_P3A(TValue *arg, int) { arg[0] += arg[1] * arg[1] * arg[1]; }
      static void MUP_FASTCALL FUN_P4A(TValue *arg, int) { arg[0] += arg[1] * arg[1] * arg[1] * arg[1]; }

  public:

      //-------------------------------------------------------------------------------------------
      ParserByteCode()
        :m_iStackPos(0)
        ,m_iMaxStackSize(0)
        ,m_vRPN()
        ,m_bEnableOptimizer(true)
        ,m_eEngineCode(ecUNOPTIMIZABLE)
      {
        m_vRPN.reserve(50);
      }

      //-------------------------------------------------------------------------------------------
      ParserByteCode(const ParserByteCode &a_ByteCode)
      {
        Assign(a_ByteCode);
      }

      //-------------------------------------------------------------------------------------------
      ParserByteCode& operator=(const ParserByteCode &a_ByteCode)
      {
        Assign(a_ByteCode);
        return *this;
      }

      //-------------------------------------------------------------------------------------------
      void Assign(const ParserByteCode &a_ByteCode)
      {
        if (this==&a_ByteCode)    
          return;

        m_iStackPos = a_ByteCode.m_iStackPos;
        m_vRPN = a_ByteCode.m_vRPN;
        m_iMaxStackSize = a_ByteCode.m_iMaxStackSize;
      }

      //-------------------------------------------------------------------------------------------
      void AddVal(token_type &tok)
      {
        ++m_iStackPos;

        tok.Val.ptr2 = nullptr;  // NUR nullptr, steht für uninitialisiert!
        tok.Val.mul2 = 0;
        tok.Val.fixed2 = 0;

        m_iMaxStackSize = std::max(m_iMaxStackSize, (size_t)m_iStackPos);
        AddTok(tok);
      }

      //-------------------------------------------------------------------------------------------
      void AddTok(const token_type &tok)
      {
        MUP_ASSERT(m_iStackPos>=0);
        tok.StackPos = m_iStackPos;
        m_vRPN.push_back(tok);
      }

      //-------------------------------------------------------------------------------------------
      void RemoveTok()
      {
        m_vRPN.pop_back();
        m_iStackPos = m_vRPN.back().StackPos;
      }

      //-------------------------------------------------------------------------------------------
      void AddAssignOp(const token_type &tok)
      {
        --m_iStackPos;

        MUP_ASSERT(tok.Cmd==cmASSIGN);
        MUP_ASSERT(m_iStackPos>=1);

        AddTok(tok);
      }

      //-------------------------------------------------------------------------------------------
      void AddIf(const token_type &tok)
      {
        MUP_ASSERT(tok.Cmd==cmIF);
        m_iStackPos--;
        AddTok(tok);
      }

      //-------------------------------------------------------------------------------------------
      void AddElse(const token_type &tok)
      {
        MUP_ASSERT(tok.Cmd==cmELSE);
        m_iStackPos = tok.StackPos = m_vRPN.back().StackPos-1;
        AddTok(tok);
      }

      //-------------------------------------------------------------------------------------------
      void AddFun(token_type &tok)
      {
        bool bOptimized = false;
        if (m_bEnableOptimizer)
        {
          bOptimized = TryConstantFolding(tok);
          if (!bOptimized)
          {
            if (tok.Cmd==cmOPRT_BIN)
            {
              if ( tok.Ident==_SL("+") || tok.Ident==_SL("-"))
                bOptimized = TryOptimizeAddSub(tok);
              else if (tok.Ident==_SL("*"))
                bOptimized = TryOptimizeMul(tok);
              else if (tok.Ident==_SL("^"))
                bOptimized = TryOptimizePow(tok);
            }
          }
        }

        if (!bOptimized)
        {
          m_iStackPos = m_iStackPos - tok.Fun.argc + 1; 
          m_iMaxStackSize = std::max(m_iMaxStackSize, (size_t)m_iStackPos);

          // from this point on it doesn't matter whether it was an operator or a function
          token_type storedTok(tok);
          storedTok.Cmd = cmFUNC; 

          AddTok(storedTok);
        }
      }

      //-------------------------------------------------------------------------------------------
      void Finalize(TValue *pStack)
      {
        Substitute();
        
        // reduce the number of tokens by sqeezing multiple tokens into a single one
        Compress();

        // Add end marker
        token_type tok;
        tok.Cmd = cmEND;
        m_vRPN.push_back(tok);

        // Determine the if-then-else jump offsets
        ParserStack<int> stIf, stElse;
        int idx, nEngineCode = 0;
        bool bNoMul = true;
        for (int i=0; i<(int)m_vRPN.size(); ++i)
        {
          token_type &tok = m_vRPN[i];

          // Reintroduce cmVAL and cmVAR codes, they can be evaluated more 
          // efficiently (compared to cmVAL_EX)
          if (tok.Val.mul==0 && tok.Cmd==cmVAL_EX && tok.Val.ptr2==nullptr)
          {
            tok.Cmd = cmVAL;
          }
          else if (tok.Val.fixed==0 && tok.Val.mul==1 && tok.Cmd==cmVAL_EX && tok.Val.ptr2==nullptr)
          {
            tok.Cmd = cmVAR;
          }

          if (tok.Cmd == cmVAL_EX && tok.Val.mul!=1)
            bNoMul = false;

          switch(tok.Cmd)
          {
          case cmIF:      stIf.push(i);
                          nEngineCode |= ecUNOPTIMIZABLE;
                          break;

          case cmELSE:    stElse.push(i);
                          idx = stIf.pop();
                          m_vRPN[idx].Oprt.offset = i - idx;
                          nEngineCode |= ecUNOPTIMIZABLE;
                          break;
      
          case cmENDIF:   idx = stElse.pop();
                          m_vRPN[idx].Oprt.offset = i - idx;
                          nEngineCode |= ecUNOPTIMIZABLE;
                          break;

          case cmVAL:
          case cmVAR:
          case cmVAL_EX:  if (nEngineCode<ecUNOPTIMIZABLE)
                          {
                            nEngineCode = nEngineCode << 1;
                            nEngineCode |= 1;
                          }
                          break;

          case cmFUNC:    // Functions with more than one or no parameter 
                          // will not be subject to short expression optimization
                          if (tok.Fun.argc<1)
                          {
                            nEngineCode |= ecUNOPTIMIZABLE;
                            break;
                          }

                          if (nEngineCode<ecUNOPTIMIZABLE)
                            nEngineCode = nEngineCode << 1;
                          break;

          case cmEND:     break;

          default:        nEngineCode |= ecUNOPTIMIZABLE;
                          break;
          }
        }

        // Determine the code for the short expression optimization
        if (nEngineCode>=ecUNOPTIMIZABLE)
          nEngineCode = ecUNOPTIMIZABLE;
  
        if (bNoMul && nEngineCode != ecUNOPTIMIZABLE)
          m_eEngineCode = (EEngineCode)(nEngineCode | ecNO_MUL);
        else
          m_eEngineCode = (EEngineCode)nEngineCode;

        // reduce the number of tokens by sqeezing multiple tokens into a single one
//        Compress();
      }

      //-------------------------------------------------------------------------------------------
      void Clear()
      {
        m_vRPN.clear();
        m_iStackPos     = 0;
        m_iMaxStackSize = 0;
        m_eEngineCode   = ecUNOPTIMIZABLE;
      }

      //-------------------------------------------------------------------------------------------
      std::size_t GetMaxStackSize() const
      {
        return m_iMaxStackSize + 1;
      }

      //-------------------------------------------------------------------------------------------
      std::size_t GetSize() const
      {
        return m_vRPN.size();
      }

      //-------------------------------------------------------------------------------------------
      const token_type* GetBase() const
      {
        if (m_vRPN.size()==0)
          throw ParserError<TString>(ecINTERNAL_ERROR);
        else
          return &m_vRPN[0];
      }

      //-------------------------------------------------------------------------------------------
      EEngineCode GetEngineCode() const
      {
        return m_eEngineCode;
      }

      //-------------------------------------------------------------------------------------------
      void AsciiDump()
      {
        if (!m_vRPN.size()) 
        {
          _OUT << _SL("No bytecode available\n");
          return;
        }

        _OUT << _SL("Number of RPN tokens:") << (int)m_vRPN.size()-1 << _SL("\n");
        for (std::size_t i=0; i<m_vRPN.size() && m_vRPN[i].Cmd!=cmEND; ++i)
        {
          _OUT << std::dec << i << _SL(" : ") << m_vRPN[i].StackPos << _SL("\t");

          switch (m_vRPN[i].Cmd)
          {
          case  cmVAL_EX:
                _OUT << _SL("VAL_EX \t");
               
                if (m_vRPN[i].Val.ptr==&ParserBase<TValue, TString>::g_NullValue)
                {
	                _OUT << _SL("[ADDR: &ParserBase::g_NullValue]"); 
                }
                else
                {
	                _OUT << _SL("[ADDR: 0x") << std::hex << m_vRPN[i].Val.ptr << _SL("]"); 
                  _OUT << _SL("[IDENT:")   << m_vRPN[i].Ident << _SL("]"); 
                }

	              _OUT << _SL("[MUL: ") << m_vRPN[i].Val.mul   << _SL("]"); 
                _OUT << _SL("[ADD:")  << m_vRPN[i].Val.fixed << _SL("]\n");

                if (m_vRPN[i].Val.ptr2!=nullptr)
                {
                  _OUT << _SL("\tVAL_EX \t");

                  if (m_vRPN[i].Val.ptr2==&ParserBase<TValue, TString>::g_NullValue)
                  {
	                  _OUT << _SL("[ADDR:  &ParserBase::g_NullValue]"); 
                  }
                  else
                  {
	                  _OUT << _SL("[ADDR: 0x") << std::hex << m_vRPN[i].Val.ptr2 << _SL("]"); 
                    _OUT << _SL("[IDENT:")   << m_vRPN[i].Ident << _SL("]"); 
                  }

	                _OUT << _SL("[MUL: ") << m_vRPN[i].Val.mul2   << _SL("]"); 
                  _OUT << _SL("[ADD:")  << m_vRPN[i].Val.fixed2 << _SL("]\n");
                }
                break;

          case  cmVAL:   
                _OUT << _SL("VAL \t");
                _OUT << _SL("[") << m_vRPN[i].Val.fixed << _SL("]");
                _OUT << _SL("[IDENT:") << m_vRPN[i].Ident << _SL("]\n"); 
                break;

          case  cmVAR:
                _OUT << _SL("VAR \t");
                _OUT << _SL("[ADDR: 0x") << std::hex << m_vRPN[i].Val.ptr << _SL("]"); 
                _OUT << _SL("[IDENT:")   << m_vRPN[i].Ident << _SL("]\n"); 
                break;

          case  cmFUNC:
                _OUT << _SL("CALL\t");
                _OUT << _SL("[IDENT:")   << m_vRPN[i].Ident << _SL("]"); 
                _OUT << _SL("[ARG:")     << std::dec << m_vRPN[i].Fun.argc << _SL("]"); 
                _OUT << _SL("[ADDR: 0x") << std::hex << m_vRPN[i].Fun.ptr  << _SL("]"); 
                _OUT << _SL("\n");

                if (m_vRPN[i].Fun.ptr2!=nullptr)
                {
                  _OUT << _SL("\tCALL\t");
                  _OUT << _SL("[IDENT: \"\"]"); 
                  _OUT << _SL("[ARG:")     << std::dec << m_vRPN[i].Fun.argc2 << _SL("]"); 
                  _OUT << _SL("[ADDR: 0x") << std::hex << m_vRPN[i].Fun.ptr2  << _SL("]"); 
                  _OUT << _SL("\n");
                }

                if (m_vRPN[i].Fun.ptr3!=nullptr)
                {
                  _OUT << _SL("\tCALL\t");
                  _OUT << _SL("[IDENT: \"\"]"); 
                  _OUT << _SL("[ARG:")     << std::dec << m_vRPN[i].Fun.argc3 << _SL("]"); 
                  _OUT << _SL("[ADDR: 0x") << std::hex << m_vRPN[i].Fun.ptr3  << _SL("]"); 
                  _OUT << _SL("\n");
                }
                break;

          case  cmIF:    
                _OUT << _SL("IF\t");
                _OUT << _SL("[OFFSET:") << std::dec << m_vRPN[i].Oprt.offset << _SL("]\n");
                break;

          case  cmELSE:  
                _OUT << _SL("ELSE\t");
                _OUT << _SL("[OFFSET:") << std::dec << m_vRPN[i].Oprt.offset << _SL("]\n");
                break;

          case  cmENDIF: 
                _OUT << _SL("ENDIF\n"); 
                break;

          case  cmASSIGN: 
                _OUT << _SL("ASSIGN\t");
                _OUT << _SL("[ADDR: 0x") << m_vRPN[i].Oprt.ptr << _SL("]\n"); 
                break; 

          default:
                _OUT << _SL("(unknown code: ") << m_vRPN[i].Cmd << _SL(")\n"); 
                break;
          } // switch cmdCode
        } // while bytecode

        _OUT << _SL("END") << std::endl;
      }

  private:

      EEngineCode m_eEngineCode;  ///< A code identifying the precompiled parsing engine to use with this bytecode

      //-------------------------------------------------------------------------------------------
      bool TryOptimizeAddSub(token_type &tok)
      {
        std::size_t sz = m_vRPN.size();

        // 0.) Transform minus operations into an addition of a negative value. This will make 
        //     further optimization easier since i only have to deal with additions.
        if (sz>=1 && m_vRPN[sz-1].Cmd == cmVAL_EX && tok.Cmd == cmOPRT_BIN && tok.Ident==_SL("-") ) 
        {
          // change sign of the last value
          if (m_vRPN[sz-1].Val.mul!=0)
            m_vRPN[sz-1].Val.mul *= -1;
          
          if (m_vRPN[sz-1].Val.fixed!=0)
            m_vRPN[sz-1].Val.fixed *= -1;
          
          // transform the subtraction into an addition
          tok.Fun.ptr = MathImpl<TValue, TString>::Add;
          tok.Ident = _SL("+");

          // maybe there is another addition directly in front?
          if (sz>=2 && m_vRPN[sz-2].Cmd==cmFUNC && m_vRPN[sz-2].Ident==_SL("+") && m_vRPN[sz-1].Cmd==cmVAL_EX)
          {
            token_type t1 = m_vRPN[sz-1];
            token_type t2 = m_vRPN[sz-2];
            RemoveTok();
            RemoveTok();

            AddVal(t1);
            AddFun(t2);

            sz = m_vRPN.size(); // update size; AddFun may have been optimized away
          }
          else
          {
            // return false since tok must be pushed regardless of this optimization
            return false;
          }
        }

        // 1.) Test try to join values by partially calculating the result and storing it into a 
        //     val_ex
        //
        // Simple optimization based on pattern recognition for a shitload of different
        // bytecode combinations of addition/subtraction
        //
        // If possible Addition/Subtraction is applied immediately and the value tokens
        // are joined.
        if (sz>=2 && m_vRPN[sz-1].Cmd == cmVAL_EX && m_vRPN[sz-2].Cmd == cmVAL_EX) 
        {
          if ( (m_vRPN[sz-1].Val.mul==0 && m_vRPN[sz-2].Val.mul==0) || 
               (m_vRPN[sz-1].Val.mul==0 && m_vRPN[sz-2].Val.mul!=0) || 
               (m_vRPN[sz-1].Val.mul!=0 && m_vRPN[sz-2].Val.mul==0) || 
               (m_vRPN[sz-1].Val.ptr==m_vRPN[sz-2].Val.ptr) )
          {
            m_vRPN[sz-2].Val.ptr    = (m_vRPN[sz-1].Val.mul==0) ? m_vRPN[sz-2].Val.ptr : m_vRPN[sz-1].Val.ptr;
            m_vRPN[sz-2].Val.fixed += ((tok.Ident==_SL("-")) ? -1 : 1) * m_vRPN[sz-1].Val.fixed;
            m_vRPN[sz-2].Val.mul   += ((tok.Ident==_SL("-")) ? -1 : 1) * m_vRPN[sz-1].Val.mul;

            RemoveTok();

            if (m_vRPN.back().Val.mul==0)
              m_vRPN.back().ResetVariablePart();

            return true;
          } 
        }

        return false;
      }

      //-------------------------------------------------------------------------------------------
      bool TryOptimizeMul(const token_type & /*tok*/)
      {
        std::size_t sz = m_vRPN.size();
        if (sz<2 || m_vRPN[sz-1].Cmd != cmVAL_EX ||  m_vRPN[sz-2].Cmd != cmVAL_EX) 
          return false;

        // Value multiplied with a variable or vice versa
        if (m_vRPN[sz-1].Val.mul==0 && m_vRPN[sz-2].Val.mul!=0)
        {
          m_vRPN[sz-2].Cmd        = cmVAL_EX;
          m_vRPN[sz-2].Val.ptr    = m_vRPN[sz-2].Val.ptr;
          m_vRPN[sz-2].Val.mul   *= m_vRPN[sz-1].Val.fixed;
          m_vRPN[sz-2].Val.fixed *= m_vRPN[sz-1].Val.fixed;
          RemoveTok();
          return true;
        }
        else if (m_vRPN[sz-1].Val.mul!=0 && m_vRPN[sz-2].Val.mul==0)
        {
          m_vRPN[sz-2].Cmd       = cmVAL_EX;
          m_vRPN[sz-2].Val.ptr   = m_vRPN[sz-1].Val.ptr;
          m_vRPN[sz-2].Val.mul   = m_vRPN[sz-1].Val.mul   * m_vRPN[sz-2].Val.fixed;
          m_vRPN[sz-2].Val.fixed = m_vRPN[sz-1].Val.fixed * m_vRPN[sz-2].Val.fixed;
          RemoveTok();
          return true;
        }

        return false;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Tries to replace calls to pow with low integer exponents with 
                 faster versions. 
      */
      bool TryOptimizePow(const token_type &tok)
      {
        std::size_t sz = m_vRPN.size();
        if (sz<2)
          return false;

        token_type &top = m_vRPN[sz-1];
        if (top.Cmd != cmVAL_EX) 
          return false;

        token_type newTok(tok);
        int nPow = (int)(top.Val.fixed);
        if ( nPow==top.Val.fixed && top.Val.fixed>=2 && top.Val.fixed<=5)
        {
          RemoveTok();
          newTok.Cmd = cmFUNC;
          newTok.Fun.argc = 1;

          switch(nPow)
          {
          case 2:  newTok.Fun.ptr = FUN_P2; newTok.Ident = _SL("^2"); break;
          case 3:  newTok.Fun.ptr = FUN_P3; newTok.Ident = _SL("^3"); break;
          case 4:  newTok.Fun.ptr = FUN_P4; newTok.Ident = _SL("^4"); break;
          case 5:  newTok.Fun.ptr = FUN_P5; newTok.Ident = _SL("^5"); break;
          default: throw ParserError<TString>(ecINTERNAL_ERROR);            
          }

          AddTok(newTok);
          return true;
        }

        return false;
      }

      //---------------------------------------------------------------------------
      bool TryConstantFolding(const token_type &tok)
      {
        std::size_t sz = m_vRPN.size();
        if (sz<(std::size_t)tok.Fun.argc || tok.Fun.argc>=20)
          return false;

        if (tok.Fun.argc==0)
          return false;

        TValue buf[20];
        for (int i=0; i<tok.Fun.argc; ++i)
        {
          const token_type &t = m_vRPN[sz - tok.Fun.argc + i];
      
          if (t.Cmd!=cmVAL && t.Cmd!=cmVAR && t.Cmd!=cmVAL_EX)
            return false;

          // If there is a variable component no optimization is possible,
          // else collect the constant value for function application
          if (t.Val.mul!=0)
            return false;
          else
            buf[i] = t.Val.fixed;
        }

        // all parameters are constant, remove them from the stack and apply the function
        m_vRPN.erase(m_vRPN.end() - tok.Fun.argc + 1, m_vRPN.end());

        token_type &result = m_vRPN.back(); 
        result.ResetVariablePart();
        (*(tok.Fun).ptr)(buf, tok.Fun.argc);
        result.Val.fixed = buf[0];
        m_iStackPos = result.StackPos;
        return true;
      }

      //---------------------------------------------------------------------------------------------
      bool TrySubstitute(const TString &op1, const TString &op2, fun_type pFun, token_type &t1, token_type &t2) 
      {
        if (t1.Ident==op1 && t2.Ident==op2)
        {
          t2.Ident = op1 + op2;
          t2.Fun.ptr  = pFun;
          t2.Fun.argc = 3;
          return true;
        }

        return false;
      }

      //---------------------------------------------------------------------------------------------
      /** \brief Substitute Multiplication with a power of x with a single function call.
      */
      bool TrySubstitute(const TString &op1, 
                         fun_type pFunPow, 
                         fun_type pFun, 
                         token_type &t1, 
                         token_type &t2, 
                         const TString &sIdent) 
      {
        if (t1.Ident==op1 && t2.Fun.ptr==pFunPow)
        {
          t2.Ident = sIdent;
          t2.Fun.ptr  = pFun;
          t2.Fun.argc = 2;
          return true;
        }

        return false;
      }
      //-------------------------------------------------------------------------------------------
      void Substitute()
      {
        if (!m_bEnableOptimizer)
          return;

        rpn_type  newRPN;
        for (std::size_t i=0; i<m_vRPN.size(); ++i)
        {
          token_type &tokOrig = m_vRPN[i];

          if (!newRPN.size())
          {
            newRPN.push_back(tokOrig);
            continue;
          }

          token_type &tokNew = newRPN.back();
          switch(tokOrig.Cmd)
          {
          case  cmFUNC:
                // Herausfinden, ob zwei operatoren kombiniert werden können
                if (tokOrig.Cmd==cmFUNC && tokNew.Cmd==cmFUNC)
                {
                  if (TrySubstitute(_SL("+"), _SL("+"), FUN_AA, tokOrig, tokNew)) break;
                  if (TrySubstitute(_SL("*"), _SL("*"), FUN_MM, tokOrig, tokNew)) break;
                  if (TrySubstitute(_SL("+"), _SL("*"), FUN_MA, tokOrig, tokNew)) break;
                  if (TrySubstitute(_SL("*"), _SL("+"), FUN_AM, tokOrig, tokNew)) break;

                  if (TrySubstitute(_SL("/"), _SL("/"), FUN_DD, tokOrig, tokNew)) break;
                  if (TrySubstitute(_SL("*"), _SL("/"), FUN_DM, tokOrig, tokNew)) break;
                  if (TrySubstitute(_SL("/"), _SL("*"), FUN_MD, tokOrig, tokNew)) break;

                  if (TrySubstitute(_SL("+"), _SL("/"), FUN_DA, tokOrig, tokNew)) break;
                  if (TrySubstitute(_SL("/"), _SL("+"), FUN_AD, tokOrig, tokNew)) break;
                  if (TrySubstitute(_SL("-"), _SL("/"), FUN_DS, tokOrig, tokNew)) break;
                  if (TrySubstitute(_SL("/"), _SL("-"), FUN_SD, tokOrig, tokNew)) break;

                  if (TrySubstitute(_SL("*"), FUN_P2, FUN_P2M, tokOrig, tokNew, _SL("^2*"))) break;
                  if (TrySubstitute(_SL("*"), FUN_P3, FUN_P3M, tokOrig, tokNew, _SL("^3*"))) break;
                  if (TrySubstitute(_SL("*"), FUN_P4, FUN_P4M, tokOrig, tokNew, _SL("^4*"))) break;

                  if (TrySubstitute(_SL("+"), FUN_P2, FUN_P2A, tokOrig, tokNew, _SL("^2+"))) break;
                  if (TrySubstitute(_SL("+"), FUN_P3, FUN_P3A, tokOrig, tokNew, _SL("^3+"))) break;
                  if (TrySubstitute(_SL("+"), FUN_P4, FUN_P4A, tokOrig, tokNew, _SL("^4+"))) break;
                }

                // Function tokens can't be joined
                newRPN.push_back(tokOrig);
                continue;

          default:
                newRPN.push_back(tokOrig);
                continue;
          }
        }

        m_vRPN = newRPN;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Try to shorten the bytecode.
      
        This function will try shortening the bytecode by merging successive 
        entries into a single one. Successive arithmetic operations will
        be replaced with ternary function calls if applicable. 

        a+b+c -> add(a,b,c)
      */
      void Compress()
      {
        rpn_type  newRPN;

        if (m_bEnableOptimizer)
        {
          for (std::size_t i=0; i<m_vRPN.size(); ++i)
          {
            token_type &tokOrig = m_vRPN[i];

            if (!newRPN.size())
            {
              newRPN.push_back(tokOrig);
              continue;
            }

            token_type &tokNew = newRPN.back();
            switch(tokOrig.Cmd)
            {
            case  cmFUNC:
                  if (tokNew.Cmd==cmFUNC)
                  {
                    if (tokNew.Fun.ptr2==nullptr)
                    {
                      // Join the two function tokens
                      tokNew.Fun.ptr2  = tokOrig.Fun.ptr;
                      tokNew.Fun.argc2 = tokOrig.Fun.argc;
                      continue;
                    }
                    else if (tokNew.Fun.ptr3==nullptr)
                    {
                      // Joint with 3rd function
                      tokNew.Fun.ptr3  = tokOrig.Fun.ptr;
                      tokNew.Fun.argc3 = tokOrig.Fun.argc;
                      continue;
                    }
                  }

                  // Function tokens can't be joined
                  newRPN.push_back(tokOrig);
                  continue;

            case  cmVAL_EX:
                  if (tokNew.Cmd==cmVAL_EX && tokNew.Val.ptr2==nullptr)
                  {
                    // Join the two function tokens
                    tokNew.Val.ptr2   = tokOrig.Val.ptr;
                    tokNew.Val.fixed2 = tokOrig.Val.fixed;
                    tokNew.Val.mul2   = tokOrig.Val.mul;
                    continue;
                  }

                  // Function tokens can't be joined
                  newRPN.push_back(tokOrig);
                  continue;

            default:
                  newRPN.push_back(tokOrig);
                  continue;
            }
          }
        }

        m_vRPN = newRPN;
      }
  };
} // namespace mu

#endif



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

#ifndef MEC_TOKEN_READER_H
#define MEC_TOKEN_READER_H

#include <cassert>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <stack>
#include <string>

#include "mecDef.h"
#include "mecToken.h"

/** \file
    \brief This file contains the parser token reader definition.
*/


namespace mec
{
  // Forward declaration
  class ParserBase;

  /** \brief Token reader for the ParserBase class.

  */
  class TokenReader 
  {
  public:

      TokenReader(ParserBase *a_pParent);
      TokenReader* Clone(ParserBase *a_pParent) const;

      void AddValIdent(identfun_type a_pCallback);
      void SetVarCreator(facfun_type a_pFactory, void *pUserData);
      void SetExpr(const string_type &sExpr);
      void SetArgSep(char_type cArgSep);

      int GetPos() const;
      const string_type& GetFormula() const;
      const varmap_type& GetUsedVar() const;
      char_type GetArgSep() const;

      void IgnoreUndefVar(bool bIgnore);
      void ReInit();
      Token ReadNextToken();

  private:

      /** \brief Syntax codes. 
  	
	        The syntax codes control the syntax check done during the first time parsing of 
          the expression string. They are flags that indicate which tokens are allowed next
          if certain tokens are identified.
  	  */
      enum ESynCodes
      {
        noBO      = 1 << 0,  ///< to avoid i.e. "cos(7)(" 
        noBC      = 1 << 1,  ///< to avoid i.e. "sin)" or "()"
        noVAL     = 1 << 2,  ///< to avoid i.e. "tan 2" or "sin(8)3.14"
        noVAR     = 1 << 3,  ///< to avoid i.e. "sin a" or "sin(8)a"
        noARG_SEP = 1 << 4,  ///< to avoid i.e. ",," or "+," ...
        noFUN     = 1 << 5,  ///< to avoid i.e. "sqrt cos" or "(1)sin"	
        noOPT     = 1 << 6,  ///< to avoid i.e. "(+)"
        noPOSTOP  = 1 << 7,  ///< to avoid i.e. "(5!!)" "sin!"
	      noINFIXOP = 1 << 8,  ///< to avoid i.e. "++4" "!!4"
        noEND     = 1 << 9,  ///< to avoid unexpected end of formula
        noIF      = 1 << 10,
        noELSE    = 1 << 11,
        noANY     = ~0       ///< All of he above flags set
      };	

      TokenReader(const TokenReader &a_Reader);
      TokenReader& operator=(const TokenReader &a_Reader);
      void Assign(const TokenReader &a_Reader);

      void SetParent(ParserBase *a_pParent);
      int ExtractToken(const char_type *a_szCharSet, 
                       string_type &a_strTok, 
                       int a_iPos) const;
      int ExtractOperatorToken(string_type &a_sTok, int a_iPos) const;

      bool IsBuiltIn(Token &a_Tok);
      bool IsArgSep(Token &a_Tok);
      bool IsEOF(Token &a_Tok);
      bool IsInfixOpTok(Token &a_Tok);
      bool IsBuiltInFunTok(Token &a_Tok);
      bool IsFunTok(Token &a_Tok);
      bool IsPostOpTok(Token &a_Tok);
      bool IsOprt(Token &a_Tok);
      bool IsValTok(Token &a_Tok);
      bool IsVarTok(Token &a_Tok);
      bool IsUndefVarTok(Token &a_Tok);
      void Error(EErrorCodes a_iErrc, 
                 int a_iPos = -1, 
                 const string_type &a_sTok = string_type() ) const;

      Token& SaveBeforeReturn(const Token &tok);

      ParserBase *m_pParser;
      string_type m_sExpr;
      int  m_iPos;
      int  m_iSynFlags;
      bool m_bIgnoreUndefVar;

      const funmap_type *m_pFunDef;
      const funmap_type *m_pPostOprtDef;
      const funmap_type *m_pInfixOprtDef;
      const funmap_type *m_pOprtDef;
      const valmap_type *m_pConstDef;
      const strmap_type *m_pStrVarDef;
      varmap_type *m_pVarDef;  ///< The only non const pointer to parser internals
      facfun_type m_pFactory;
      void *m_pFactoryData;
      std::vector<identfun_type> m_vIdentFun; ///< Value token identification function
      varmap_type m_UsedVar;
      value_type m_fZero;      ///< Dummy value of zero, referenced by undefined variables
      int m_iBrackets;
      Token m_lastTok;
      char_type m_cArgSep;     ///< The character used for separating function arguments
  };
} // namespace mec

#endif
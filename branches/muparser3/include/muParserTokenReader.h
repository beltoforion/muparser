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
#ifndef MU_PARSER_TOKEN_READER_H
#define MU_PARSER_TOKEN_READER_H

//-------------------------------------------------------------------------------------------------
#include <list>
#include <map>
#include <string>
#include <stack>

//-------------------------------------------------------------------------------------------------
#include "muParserDef.h"
#include "muParserError.h"
#include "muParserToken.h"


MUP_NAMESPACE_START

  //-----------------------------------------------------------------------------------------------
  /** \brief Token reader implementation.
  */
  template<typename TValue , typename TString>
  class ParserTokenReader 
  {
  public:

      typedef Token<TValue, TString> token_type;
      typedef TValue* (*facfun_type)(const typename TString::value_type*, void*);
      typedef int (*identfun_type)(const typename TString::value_type *sExpr, int *nPos, TValue *fVal);

      //-------------------------------------------------------------------------------------------
      /** \brief Constructor. 
      
          Create a Token reader and bind it to a parser object. 

          \pre [assert] a_pParser may not be nullptr
          \post #m_pParser==a_pParser
          \param a_pParent Parent parser object of the token reader.
      */
      ParserTokenReader(ParserBase<TValue, TString> *a_pParent)
        :m_pParser(a_pParent)
        ,m_strFormula()
        ,m_iPos(0)
        ,m_iSynFlags(0)
        ,m_bIgnoreUndefVar(false)
        ,m_pFunDef(nullptr)
        ,m_pPostOprtDef(nullptr)
        ,m_pInfixOprtDef(nullptr)
        ,m_pOprtDef(nullptr)
        ,m_pConstDef(nullptr)
        ,m_pVarDef(nullptr)
        ,m_pFactory(nullptr)
        ,m_pFactoryData(nullptr)
        ,m_vIdentFun()
        ,m_UsedVar()
        ,m_fZero(0)
        ,m_iBrackets(0)
        ,m_lastTok()
        ,m_cArgSep(',')
      {
        assert(m_pParser);
        SetParent(m_pParser);
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Copy constructor.

          \sa Assign
          \throw nothrow
      */
      ParserTokenReader(const ParserTokenReader &a_Reader) 
      { 
        Assign(a_Reader);
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Create instance of a ParserTokenReader identical with this 
                  and return its pointer. 

          This is a factory method the calling function must take care of the object destruction.

          \return A new ParserTokenReader object.
          \throw nothrow
      */
      ParserTokenReader* Clone(ParserBase<TValue, TString> *a_pParent) const
      {
        std::unique_ptr<ParserTokenReader> ptr(new ParserTokenReader(*this));
        ptr->SetParent(a_pParent);
        return ptr.release();
      }

      //-------------------------------------------------------------------------------------------
      void AddValIdent(identfun_type a_pCallback)
      {
        m_vIdentFun.push_front(a_pCallback);
      }

      //-------------------------------------------------------------------------------------------
      void SetVarCreator(facfun_type a_pFactory, void *pUserData)
      {
        m_pFactory = a_pFactory;
        m_pFactoryData = pUserData;
      }

      //-------------------------------------------------------------------------------------------
      void SetFormula(const TString &a_strFormula)
      {
        m_strFormula = a_strFormula;
        ReInit();
      }

      //-------------------------------------------------------------------------------------------
      void SetArgSep(typename TString::value_type cArgSep)
      {
        m_cArgSep = cArgSep;
      }

      //-------------------------------------------------------------------------------------------
      int GetPos() const
      {
        return m_iPos;
      }

      //-------------------------------------------------------------------------------------------
      const TString& GetExpr() const
      {
        return m_strFormula;
      }

      //-------------------------------------------------------------------------------------------
      std::map<TString, TValue*>& GetUsedVar() 
      {
        return m_UsedVar;
      }

      //-------------------------------------------------------------------------------------------
      typename TString::value_type GetArgSep() const
      {
        return m_cArgSep;
      }

      //-------------------------------------------------------------------------------------------
      void IgnoreUndefVar(bool bIgnore)
      {
        m_bIgnoreUndefVar = bIgnore;
      }

      //-------------------------------------------------------------------------------------------
      void ReInit()
      {
        m_iPos = 0;
        m_iSynFlags = sfSTART_OF_LINE;
        m_iBrackets = 0;
        m_UsedVar.clear();
        m_lastTok = token_type();
      }

      //-------------------------------------------------------------------------------------------
      token_type ReadNextToken()
      {
        assert(m_pParser);

        std::stack<int> FunArgs;
        const TString::value_type *szFormula = m_strFormula.c_str();
        token_type tok;

        // Ignore all non printable characters when reading the expression
        while (szFormula[m_iPos]>0 && szFormula[m_iPos]<=0x20) 
          ++m_iPos;

        if ( IsEOF(tok) )        return SaveBeforeReturn(tok); // Check for end of formula
        if ( IsOprt(tok) )       return SaveBeforeReturn(tok); // Check for user defined binary operator
        if ( IsFunTok(tok) )     return SaveBeforeReturn(tok); // Check for function token
        if ( IsBuiltIn(tok) )    return SaveBeforeReturn(tok); // Check built in operators / tokens
        if ( IsArgSep(tok) )     return SaveBeforeReturn(tok); // Check for function argument separators
        if ( IsValTok(tok) )     return SaveBeforeReturn(tok); // Check for values / constant tokens
        if ( IsVarTok(tok) )     return SaveBeforeReturn(tok); // Check for variable tokens
        if ( IsInfixOpTok(tok) ) return SaveBeforeReturn(tok); // Check for unary operators
        if ( IsPostOpTok(tok) )  return SaveBeforeReturn(tok); // Check for unary operators

        // Check String for undefined variable token. Done only if a 
        // flag is set indicating to ignore undefined variables.
        // This is a way to conditionally avoid an error if 
        // undefined variables occur. 
        // (The GetUsedVar function must suppress the error for
        // undefined variables in order to collect all variable 
        // names including the undefined ones.)
        if ( (m_bIgnoreUndefVar || m_pFactory) && IsUndefVarTok(tok) )  
          return SaveBeforeReturn(tok);

        // Check for unknown token
        // 
        // !!! From this point on there is no exit without an exception possible...
        // 
        TString strTok;
        int iEnd = ExtractToken(m_pParser->c_sNameChars, strTok, m_iPos);
        if (iEnd!=m_iPos)
          Error(ecUNASSIGNABLE_TOKEN, m_iPos, strTok);

        Error(ecUNASSIGNABLE_TOKEN, m_iPos, m_strFormula.substr(m_iPos));
        return token_type(); // never reached
      }


  private:

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
        noASSIGN  = 1 << 10, ///< to block assignement to constant i.e. "4=7"
        noIF      = 1 << 11,
        noELSE    = 1 << 12,
        sfSTART_OF_LINE = noOPT | noBC | noPOSTOP | noASSIGN | noIF | noELSE | noARG_SEP,
        noANY     = ~0       ///< All of he above flags set
      };	

      //-------------------------------------------------------------------------------------------
      /** \brief Assignement operator.

          Self assignement will be suppressed otherwise #Assign is called.

          \param a_Reader Object to copy to this token reader.
          \throw nothrow
      */
      ParserTokenReader& operator=(const ParserTokenReader &a_Reader) 
      {
        if (&a_Reader!=this)
          Assign(a_Reader);

        return *this;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Assign state of a token reader to this token reader. 
      
          \param a_Reader Object from which the state should be copied.
          \throw nothrow
      */
      void Assign(const ParserTokenReader &a_Reader)
      {
        m_pParser = a_Reader.m_pParser;
        m_strFormula = a_Reader.m_strFormula;
        m_iPos = a_Reader.m_iPos;
        m_iSynFlags = a_Reader.m_iSynFlags;
    
        m_UsedVar         = a_Reader.m_UsedVar;
        m_pFunDef         = a_Reader.m_pFunDef;
        m_pConstDef       = a_Reader.m_pConstDef;
        m_pVarDef         = a_Reader.m_pVarDef;
        m_pPostOprtDef    = a_Reader.m_pPostOprtDef;
        m_pInfixOprtDef   = a_Reader.m_pInfixOprtDef;
        m_pOprtDef        = a_Reader.m_pOprtDef;
        m_bIgnoreUndefVar = a_Reader.m_bIgnoreUndefVar;
        m_vIdentFun       = a_Reader.m_vIdentFun;
        m_pFactory        = a_Reader.m_pFactory;
        m_pFactoryData    = a_Reader.m_pFactoryData;
        m_iBrackets       = a_Reader.m_iBrackets;
        m_cArgSep         = a_Reader.m_cArgSep;
      }

      //-------------------------------------------------------------------------------------------
      void SetParent(ParserBase<TValue, TString> *a_pParent)
      {
        m_pParser       = a_pParent; 
        m_pFunDef       = &a_pParent->m_FunDef;
        m_pOprtDef      = &a_pParent->m_OprtDef;
        m_pInfixOprtDef = &a_pParent->m_InfixOprtDef;
        m_pPostOprtDef  = &a_pParent->m_PostOprtDef;
        m_pVarDef       = &a_pParent->m_VarDef;
        m_pConstDef     = &a_pParent->m_ConstDef;
      }

      //-------------------------------------------------------------------------------------------
      int ExtractToken(const typename TString::value_type *a_szCharSet, TString &a_sTok, int a_iPos) const
      {
        int iEnd = (int)m_strFormula.find_first_not_of(a_szCharSet, a_iPos);

        if (iEnd==(int)TString::npos)
            iEnd = (int)m_strFormula.length();
    
        // Assign token string if there was something found
        if (a_iPos!=iEnd)
          a_sTok = TString( m_strFormula.begin()+a_iPos, m_strFormula.begin()+iEnd);

        return iEnd;
      }

      //-------------------------------------------------------------------------------------------
      int ExtractOperatorToken(TString &a_sTok, int a_iPos) const
      {
        int iEnd = (int)m_strFormula.find_first_not_of(m_pParser->c_sInfixOprtChars, a_iPos);
        if (iEnd==(int)TString::npos)
          iEnd = (int)m_strFormula.length();

        // Assign token string if there was something found
        if (a_iPos!=iEnd)
        {
          a_sTok = TString( m_strFormula.begin() + a_iPos, m_strFormula.begin() + iEnd);
          return iEnd;
        }
        else
        {
          // There is still the chance of having to deal with an operator consisting exclusively
          // of alphabetic characters.
          return ExtractToken(_SL("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), a_sTok, a_iPos);
        }
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Check if a built in operator or other token can be found
          \param a_Tok  [out] Operator token if one is found. This can either be a binary operator or an infix operator token.
          \return true if an operator token has been found.
      */
      bool IsBuiltIn(token_type &a_Tok)
      {
        const TString::value_type **const pOprtDef = m_pParser->c_DefaultOprt,
                                   *const szFormula = m_strFormula.c_str();

        // Compare token with function and operator strings
        // check string for operator/function
        for (int i=0; pOprtDef[i]; i++)
        {
          std::size_t len( std::char_traits<TString::value_type>::length(pOprtDef[i]) );
          if ( TString(pOprtDef[i]) == TString(szFormula + m_iPos, szFormula + m_iPos + len) )
          {
            switch(i)
            {
            case cmASSIGN:
                  // The assignement operator need special treatment
                  if (i==cmASSIGN && m_iSynFlags & noASSIGN)
                    Error(ecUNEXPECTED_OPERATOR, m_iPos, pOprtDef[i]);

                  if (m_iSynFlags & noOPT) 
                  {
                    // Maybe its an infix operator not an operator
                    // Both operator types can share characters in 
                    // their identifiers
                    if ( IsInfixOpTok(a_Tok) ) 
                      return true;

                    Error(ecUNEXPECTED_OPERATOR, m_iPos, pOprtDef[i]);
                  }

                  m_iSynFlags  = noBC | noOPT | noARG_SEP | noPOSTOP | noASSIGN | noIF | noELSE;
                  m_iSynFlags |= ( (i != cmEND) && ( i != cmBC) ) ? noEND : 0;
                  break;

		        case cmBO:
                  if (m_iSynFlags & noBO)
	                  Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef[i]);
              
                  if (m_lastTok.Cmd==cmFUNC)
                    m_iSynFlags = noOPT | noEND | noARG_SEP | noPOSTOP | noASSIGN | noIF | noELSE;
                  else
                    m_iSynFlags = noBC | noOPT | noEND | noARG_SEP | noPOSTOP | noASSIGN| noIF | noELSE;

                  ++m_iBrackets;
                  break;

		        case cmBC:
                  if (m_iSynFlags & noBC)
                    Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef[i]);

                  m_iSynFlags  = noBO | noVAR | noVAL | noFUN | noINFIXOP | noASSIGN;

                  if (--m_iBrackets<0)
                    Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef[i]);
                  break;

            case cmELSE:
                  if (m_iSynFlags & noELSE)
                    Error(ecUNEXPECTED_CONDITIONAL, m_iPos, pOprtDef[i]);

                  m_iSynFlags = noBC | noPOSTOP | noEND | noOPT | noIF | noELSE;
                  break;

            case cmIF:
                  if (m_iSynFlags & noIF)
                    Error(ecUNEXPECTED_CONDITIONAL, m_iPos, pOprtDef[i]);

                  m_iSynFlags = noBC | noPOSTOP | noEND | noOPT | noIF | noELSE;
                  break;

		        default:      // The operator is listed in c_DefaultOprt, but not here. This is a bad thing...
                  Error(ecINTERNAL_ERROR);
            } // switch operator id

            m_iPos += (int)len;
            a_Tok.Set((ECmdCode)i, pOprtDef[i]);
            return true;
	        } // if operator string found
        } // end of for all operator strings
  
        return false;
      }

      //-------------------------------------------------------------------------------------------
      bool IsArgSep(token_type &a_Tok)
      {
        const TString::value_type* szFormula = m_strFormula.c_str();

        if (szFormula[m_iPos]==m_cArgSep)
        {
          // copy the separator into null terminated string
          TString::value_type szSep[2];
          szSep[0] = m_cArgSep;
          szSep[1] = 0;

          if (m_iSynFlags & noARG_SEP)
            Error(ecUNEXPECTED_ARG_SEP, m_iPos, szSep);

          m_iSynFlags  = noBC | noOPT | noEND | noARG_SEP | noPOSTOP | noASSIGN;
          m_iPos++;

          a_Tok.Cmd = cmARG_SEP;
          a_Tok.Ident = szSep;
          return true;
        }

        return false;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Check for End of Formula.

          \return true if an end of formula is found false otherwise.
          \param a_Tok [out] If an eof is found the corresponding token will be stored there.
          \throw nothrow
          \sa IsOprt, IsFunTok, IsStrFunTok, IsValTok, IsVarTok, IsString, IsInfixOpTok, IsPostOpTok
      */
      bool IsEOF(token_type &a_Tok)
      {
        const TString::value_type* szFormula = m_strFormula.c_str();

        // check for EOF
        if ( !szFormula[m_iPos] /*|| szFormula[m_iPos] == '\n'*/)
        {
          if ( m_iSynFlags & noEND )
            Error(ecUNEXPECTED_EOF, m_iPos);

          if (m_iBrackets>0)
            Error(ecMISSING_PARENS, m_iPos, _SL(")"));

          m_iSynFlags = 0;
          a_Tok.Cmd = cmEND;
          return true;
        }

        return false;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Check if a string position contains a unary infix operator. 
          \return true if a function token has been found false otherwise.
      */
      bool IsInfixOpTok(token_type &a_Tok)
      {
        TString sTok;
        int iEnd = ExtractToken(m_pParser->c_sInfixOprtChars, sTok, m_iPos);
        if (iEnd==m_iPos)
          return false;

        // iteraterate over all postfix operator strings
        std::map<TString, token_type>::const_reverse_iterator it = m_pInfixOprtDef->rbegin();
        for ( ; it!=m_pInfixOprtDef->rend(); ++it)
        {
          if (sTok.find(it->first)!=0)
            continue;

          a_Tok = it->second; //.Set(it->second, it->first);
          m_iPos += (int)it->first.length();

          if (m_iSynFlags & noINFIXOP) 
            Error(ecUNEXPECTED_OPERATOR, m_iPos, a_Tok.Ident);

          m_iSynFlags = noPOSTOP | noINFIXOP | noOPT | noBC | noASSIGN;
          return true;
        }

        return false;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Check whether the token at a given position is a function token.
          \param a_Tok [out] If a value token is found it will be placed here.
          \throw ParserException if Syntaxflags do not allow a function at a_iPos
          \return true if a function token has been found false otherwise.
          \pre [assert] m_pParser!=0
      */
      bool IsFunTok(token_type &a_Tok)
      {
        TString strTok;
        int iEnd = ExtractToken(m_pParser->c_sNameChars, strTok, m_iPos);
        if (iEnd==m_iPos)
          return false;

        auto item = m_pFunDef->find(strTok);
        if (item==m_pFunDef->end())
          return false;

        // Check if the next sign is an opening bracket
        const TString::value_type *szFormula = m_strFormula.c_str();
        if (szFormula[iEnd]!='(')
          return false;

        a_Tok = item->second; 

        m_iPos = (int)iEnd;
        if (m_iSynFlags & noFUN)
          Error(ecUNEXPECTED_FUN, m_iPos-(int)a_Tok.Ident.length(), a_Tok.Ident);

        m_iSynFlags = noANY ^ noBO;
        return true;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Check if a string position contains a binary operator.
          \param a_Tok  [out] Operator token if one is found. This can either be a binary operator or an infix operator token.
          \return true if an operator token has been found.
      */
      bool IsOprt(token_type &a_Tok)
      {
        const TString::value_type *const szExpr = m_strFormula.c_str();
        TString strTok;

        int iEnd = ExtractOperatorToken(strTok, m_iPos);
        if (iEnd==m_iPos)
          return false;

        // Check if the operator is a built in operator, if so ignore it here
        const TString::value_type **const pOprtDef = m_pParser->c_DefaultOprt;
        for (int i=0; pOprtDef[i]; ++i)
        {
          if (TString(pOprtDef[i])==strTok)
            return false;
        }

        // Note:
        // All tokens in oprt_bin_maptype are have been sorted by their length
        // Long operators must come first! Otherwise short names (like: "add") that
        // are part of long token names (like: "add123") will be found instead 
        // of the long ones.
        // Length sorting is done with ascending length so we use a reverse iterator here.
        std::map<TString, token_type>::const_reverse_iterator it = m_pOprtDef->rbegin();
        for ( ; it!=m_pOprtDef->rend(); ++it)
        {
          const TString &sID = it->first;
          if ( sID == TString(szExpr + m_iPos, szExpr + m_iPos + sID.length()) )
          {
            a_Tok = it->second;
            //a_Tok.Set(it->second, strTok);

            // operator was found
            if (m_iSynFlags & noOPT) 
            {
              // An operator was found but is not expected to occur at
              // this position of the formula, maybe it is an infix 
              // operator, not a binary operator. Both operator types
              // can share characters in their identifiers.
              if ( IsInfixOpTok(a_Tok) ) 
                return true;
              else
              {
                // nope, no infix operator
                return false;
                //Error(ecUNEXPECTED_OPERATOR, m_iPos, a_Tok.GetAsString()); 
              }

            }

            m_iPos += (int)sID.length();
            m_iSynFlags  = noBC | noOPT | noARG_SEP | noPOSTOP | noEND | noBC | noASSIGN;
            return true;
          }
        }

        return false;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Check if a string position contains a unary post value operator. */
      bool IsPostOpTok(token_type &a_Tok)
      {
        // <ibg 20110629> Do not check for postfix operators if they are not allowed at
        //                the current expression index.
        //
        //  This will fix the bug reported here:  
        //
        //  http://sourceforge.net/tracker/index.php?func=detail&aid=3343891&group_id=137191&atid=737979
        //
        if (m_iSynFlags & noPOSTOP)
          return false;
        // </ibg>

        // Tricky problem with equations like "3m+5":
        //     m is a postfix operator, + is a valid sign for postfix operators and 
        //     for binary operators parser detects "m+" as operator string and 
        //     finds no matching postfix operator.
        // 
        // This is a special case so this routine slightly differs from the other
        // token readers.
    
        // Test if there could be a postfix operator
        TString sTok;
        int iEnd = ExtractToken(m_pParser->c_sOprtChars, sTok, m_iPos);
        if (iEnd==m_iPos)
          return false;

        // iteraterate over all postfix operator strings
        std::map<TString, token_type>::const_reverse_iterator it = m_pPostOprtDef->rbegin();
        for ( ; it!=m_pPostOprtDef->rend(); ++it)
        {
          if (sTok.find(it->first)!=0)
            continue;

          //a_Tok.Set(it->second, sTok);
          a_Tok = it->second;

  	      m_iPos += (int)it->first.length();

          m_iSynFlags = noVAL | noVAR | noFUN | noBO | noPOSTOP | noASSIGN;
          return true;
        }

        return false;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Check whether the token at a given position is a value token.

        Value tokens are either values or constants.

        \param a_Tok [out] If a value token is found it will be placed here.
        \return true if a value token has been found.
      */
      bool IsValTok(token_type &a_Tok)
      {
        assert(m_pConstDef);
        assert(m_pParser);

        TString strTok;
        TValue fVal(0);
        int iEnd(0);
    
        // 2.) Check for user defined constant
        // Read everything that could be a constant name
        iEnd = ExtractToken(m_pParser->c_sNameChars, strTok, m_iPos);
        if (iEnd!=m_iPos)
        {
          auto item = m_pConstDef->find(strTok);
          if (item!=m_pConstDef->end())
          {
            m_iPos = iEnd;
            a_Tok.SetVal(item->second, strTok);

            if (m_iSynFlags & noVAL)
              Error(ecUNEXPECTED_VAL, m_iPos - (int)strTok.length(), strTok);

            m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP | noASSIGN; 
            return true;
          }
        }

        // 3.call the value recognition functions provided by the user
        // Call user defined value recognition functions
        auto item = m_vIdentFun.begin();
        for (item = m_vIdentFun.begin(); item!=m_vIdentFun.end(); ++item)
        {
          int iStart = m_iPos;
          if ( (*item)(m_strFormula.c_str() + m_iPos, &m_iPos, &fVal)==1 )
          {
            strTok.assign(m_strFormula.c_str(), iStart, m_iPos);
            if (m_iSynFlags & noVAL)
              Error(ecUNEXPECTED_VAL, m_iPos - (int)strTok.length(), strTok);

            a_Tok.SetVal(fVal, strTok);
            m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP | noASSIGN;
            return true;
          }
        }

        return false;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Check wheter a token at a given position is a variable token. 
          \param a_Tok [out] If a variable token has been found it will be placed here.
	        \return true if a variable token has been found.
      */
      bool IsVarTok(token_type &a_Tok)
      {
        if (!m_pVarDef->size())
          return false;

        TString strTok;
        int iEnd = ExtractToken(m_pParser->c_sNameChars, strTok, m_iPos);
        if (iEnd==m_iPos)
          return false;

        auto item =  m_pVarDef->find(strTok);
        if (item==m_pVarDef->end())
          return false;

        if (m_iSynFlags & noVAR)
          Error(ecUNEXPECTED_VAR, m_iPos, strTok);

        m_iPos = iEnd;
        a_Tok.Cmd = cmVAR;
        a_Tok.Ident = strTok;
        a_Tok.Val.ptr = item->second;
        a_Tok.Val.mul = 1;
        a_Tok.Val.fixed = 0;
        m_UsedVar[item->first] = item->second;  // Add variable to used-var-list

        m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP;
        return true;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Check wheter a token at a given position is an undefined variable. 

          \param a_Tok [out] If a variable tom_pParser->m_vStringBufken has been found it will be placed here.
	        \return true if a variable token has been found.
          \throw nothrow
      */
      bool IsUndefVarTok(token_type &a_Tok)
      {
        TString strTok;
        int iEnd( ExtractToken(m_pParser->c_sNameChars, strTok, m_iPos) );
        if ( iEnd==m_iPos )
          return false;

        if (m_iSynFlags & noVAR)
          Error(ecUNEXPECTED_VAR, m_iPos - (int)a_Tok.Ident.length(), strTok);

        // If a factory is available implicitely create new variables
        if (m_pFactory)
        {
          TValue *pVar = m_pFactory(strTok.c_str(), m_pFactoryData);
          a_Tok.Cmd = cmVAR;
          a_Tok.Ident = strTok;
          a_Tok.Val.ptr = pVar;

          // Do not use m_pParser->DefineVar( strTok, fVar );
          // in order to define the new variable, it will clear the
          // m_UsedVar array which will kill previousely defined variables
          // from the list
          // This is safe because the new variable can never override an existing one
          // because they are checked first!
          (*m_pVarDef)[strTok] = pVar;
          m_UsedVar[strTok] = pVar;  // Add variable to used-var-list
        }
        else
        {
          a_Tok.Cmd = cmVAR;
          a_Tok.Ident = strTok;
          a_Tok.Val.ptr = (TValue*)&m_fZero;
          m_UsedVar[strTok] = 0;  // Add variable to used-var-list
        }

        m_iPos = iEnd;

        // Call the variable factory in order to let it define a new parser variable
        m_iSynFlags = noVAL | noVAR | noFUN | noBO | noPOSTOP | noINFIXOP;
        return true;
      }

      //-------------------------------------------------------------------------------------------
      /** \brief Create an error containing the parse error position.

        This function will create an Parser Exception object containing the error text and its position.

        \param a_iErrc [in] The error code of type #EErrorCodes.
        \param a_iPos [in] The position where the error was detected.
        \param a_strTok [in] The token string representation associated with the error.
        \throw ParserException always throws thats the only purpose of this function.
      */
      void  Error( EErrorCodes a_iErrc, 
                   int a_iPos = -1, 
                   const TString &a_sTok = TString()) const
      {
        m_pParser->Error(a_iErrc, a_iPos, a_sTok);
      }

      //---------------------------------------------------------------------------
      token_type& SaveBeforeReturn(const token_type &tok)
      {
        m_lastTok = tok;
        return m_lastTok;
      }

      ParserBase<TValue, TString> *m_pParser;
      TString m_strFormula;
      int  m_iPos;
      int  m_iSynFlags;
      bool m_bIgnoreUndefVar;

      const std::map<TString, token_type> *m_pFunDef;
      const std::map<TString, token_type> *m_pPostOprtDef;
      const std::map<TString, token_type> *m_pInfixOprtDef;
      const std::map<TString, token_type> *m_pOprtDef;
      const std::map<TString, TValue> *m_pConstDef;
      std::map<TString, TValue*> *m_pVarDef;  ///< The only non const pointer to parser internals
      facfun_type m_pFactory;
      void *m_pFactoryData;
      std::list<identfun_type> m_vIdentFun;   ///< Value token identification function
      std::map<TString, TValue*> m_UsedVar;   ///< A map with pointers to all variables used in the current expression
      TValue m_fZero;                         ///< Dummy value of zero, referenced by undefined variables
      int m_iBrackets;
      token_type m_lastTok;                   ///< A buffer for storing the last token read for reference in the next parsing step
      typename TString::value_type m_cArgSep;          ///< The character used for separating function arguments
  };
} // namespace mu

#endif



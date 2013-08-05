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
#include <cassert>
#include <cstdio>
#include <cstring>
#include <map>
#include <stack>
#include <string>

#include "mecTokenReader.h"
#include "mecParserBase.h"

/** \file
    \brief This file contains the parser token reader implementation.
*/


namespace mec
{

  // Forward declaration
  class ParserBase;

  //---------------------------------------------------------------------------
  /** \brief Copy constructor.

      \sa Assign
      \throw nothrow
  */
  TokenReader::TokenReader(const TokenReader &a_Reader) 
  { 
    Assign(a_Reader);
  }
    
  //---------------------------------------------------------------------------
  /** \brief Assignement operator.

      Self assignement will be suppressed otherwise #Assign is called.

      \param a_Reader Object to copy to this token reader.
      \throw nothrow
  */
  TokenReader& TokenReader::operator=(const TokenReader &a_Reader) 
  {
    if (&a_Reader!=this)
      Assign(a_Reader);

    return *this;
  }

  //---------------------------------------------------------------------------
  /** \brief Assign state of a token reader to this token reader. 
      
      \param a_Reader Object from which the state should be copied.
      \throw nothrow
  */
  void TokenReader::Assign(const TokenReader &a_Reader)
  {
    m_pParser = a_Reader.m_pParser;
    m_sExpr = a_Reader.m_sExpr;
    m_iPos = a_Reader.m_iPos;
    m_iSynFlags = a_Reader.m_iSynFlags;
    
    m_UsedVar         = a_Reader.m_UsedVar;
    m_pFunDef         = a_Reader.m_pFunDef;
    m_pConstDef       = a_Reader.m_pConstDef;
    m_pVarDef         = a_Reader.m_pVarDef;
    m_pStrVarDef      = a_Reader.m_pStrVarDef;
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

  //---------------------------------------------------------------------------
  /** \brief Constructor. 
      
      Create a Token reader and bind it to a parser object. 

      \pre [assert] a_pParser may not be NULL
      \post #m_pParser==a_pParser
      \param a_pParent Parent parser object of the token reader.
  */
  TokenReader::TokenReader(ParserBase *a_pParent)
    :m_pParser(a_pParent)
    ,m_sExpr()
    ,m_iPos(0)
    ,m_iSynFlags(0)
    ,m_bIgnoreUndefVar(false)
    ,m_pFunDef(NULL)
    ,m_pPostOprtDef(NULL)
    ,m_pInfixOprtDef(NULL)
    ,m_pOprtDef(NULL)
    ,m_pConstDef(NULL)
    ,m_pStrVarDef(NULL)
    ,m_pVarDef(NULL)
    ,m_pFactory(NULL)
    ,m_pFactoryData(NULL)
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
    
  //---------------------------------------------------------------------------
  /** \brief Create instance of a TokenReader identical with this 
              and return its pointer. 

      This is a factory method the calling function must take care of the object destruction.

      \return A new TokenReader object.
      \throw nothrow
  */
  TokenReader* TokenReader::Clone(ParserBase *a_pParent) const
  {
    std::auto_ptr<TokenReader> ptr(new TokenReader(*this));
    ptr->SetParent(a_pParent);
    return ptr.release();
  }

  //---------------------------------------------------------------------------
  Token& TokenReader::SaveBeforeReturn(const Token &tok)
  {
    m_lastTok = tok;
    return m_lastTok;
  }

  //---------------------------------------------------------------------------
  void TokenReader::AddValIdent(identfun_type a_pCallback)
  {
    m_vIdentFun.push_back(a_pCallback);
  }

  //---------------------------------------------------------------------------
  void TokenReader::SetVarCreator(facfun_type a_pFactory, void *pUserData)
  {
    m_pFactory = a_pFactory;
    m_pFactoryData = pUserData;
  }

  //---------------------------------------------------------------------------
  /** \brief Return the current position of the token reader in the formula string. 

      \return #m_iPos
      \throw nothrow
  */
  int TokenReader::GetPos() const
  {
    return m_iPos;
  }

  //---------------------------------------------------------------------------
  /** \brief Return a reference to the formula. 

      \return #m_sExpr
      \throw nothrow
  */
  const string_type& TokenReader::GetFormula() const
  {
    return m_sExpr;
  }

  //---------------------------------------------------------------------------
  /** \brief Return a map containing the used variables only. */
  const varmap_type& TokenReader::GetUsedVar() const
  {
    return m_UsedVar;
  }

  //---------------------------------------------------------------------------
  /** \brief Initialize the token Reader. 
  
      Sets the formula position index to zero and set Syntax flags to default for initial formula parsing.
      \pre [assert] triggered if a_szFormula==0
  */
  void TokenReader::SetExpr(const string_type &sExpr)
  {
    m_sExpr = sExpr;
    ReInit();
  }

  //---------------------------------------------------------------------------
  /** \brief Set Flag that contronls behaviour in case of undefined variables beeing found. 
  
    If true, the parser does not throw an exception if an undefined variable is found. 
    otherwise it does. This variable is used internally only!
    It supresses a "undefined variable" exception in GetUsedVar().  
    Those function should return a complete list of variables including 
    those the are not defined by the time of it's call.
  */
  void TokenReader::IgnoreUndefVar(bool bIgnore)
  {
    m_bIgnoreUndefVar = bIgnore;
  }

  //---------------------------------------------------------------------------
  /** \brief Reset the token reader to the start of the formula. 

      The syntax flags will be reset to a value appropriate for the 
      start of a formula.
      \post #m_iPos==0, #m_iSynFlags = noOPT | noBC | noPOSTOP | noSTR
      \throw nothrow
      \sa ESynCodes
  */
  void TokenReader::ReInit()
  {
    m_iPos = 0;
    m_iSynFlags = noOPT | noBC | noPOSTOP;
    m_iBrackets = 0;
    m_UsedVar.clear();
    m_lastTok = Token();
  }

  //---------------------------------------------------------------------------
  /** \brief Read the next token from the string. */ 
  Token TokenReader::ReadNextToken()
  {
    assert(m_pParser);

    std::stack<int> FunArgs; 
    const char_type *szFormula = m_sExpr.c_str();
    Token tok;

    while (szFormula[m_iPos]==' ') 
      ++m_iPos;

    if ( IsEOF(tok) )           return SaveBeforeReturn(tok);    // Check for end of formula
    if ( IsOprt(tok) )          return SaveBeforeReturn(tok);    // Check for user defined binary operator
    if ( IsBuiltInFunTok(tok) ) return SaveBeforeReturn(tok);    // Check for function token
    if ( IsFunTok(tok) )        return SaveBeforeReturn(tok);    // Check for function token
    if ( IsBuiltIn(tok) )       return SaveBeforeReturn(tok);    // Check built in operators / tokens
    if ( IsArgSep(tok) )        return SaveBeforeReturn(tok);    // Check for function argument separators
    if ( IsValTok(tok) )        return SaveBeforeReturn(tok);    // Check for values / constant tokens
    if ( IsVarTok(tok) )        return SaveBeforeReturn(tok);    // Check for variable tokens
    if ( IsInfixOpTok(tok) )    return SaveBeforeReturn(tok); // Check for unary operators
    if ( IsPostOpTok(tok) )     return SaveBeforeReturn(tok); // Check for unary operators

    // Check String for undefined variable token. Done only if a 
    // flag is set indicating to ignore undefined variables.
    // This is a way to conditionally avoid an error if 
    // undefined variables occur. 
    // The GetUsedVar function must supress the error for
    // undefined variables in order to collect all variable 
    // names including the undefined ones.
    if ( (m_bIgnoreUndefVar || m_pFactory) && IsUndefVarTok(tok) )  
      return SaveBeforeReturn(tok);

    // Check for unknown token
    // 
    // !!! From this point on there is no exit without an exception possible...
    // 
    string_type strTok;
    int iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd!=m_iPos)
      Error(ecUNASSIGNABLE_TOKEN, m_iPos, strTok);

    Error(ecUNASSIGNABLE_TOKEN, m_iPos, m_sExpr.substr(m_iPos));
    return Token(); // never reached
  }

  //---------------------------------------------------------------------------
  void TokenReader::SetParent(ParserBase *a_pParent)
  {
    m_pParser       = a_pParent; 
    m_pFunDef       = &a_pParent->m_FunDef;
    m_pOprtDef      = &a_pParent->m_OprtDef;
    m_pInfixOprtDef = &a_pParent->m_InfixOprtDef;
    m_pPostOprtDef  = &a_pParent->m_PostOprtDef;
    m_pVarDef       = &a_pParent->m_VarDef;
    m_pStrVarDef    = &a_pParent->m_StrVarDef;
    m_pConstDef     = &a_pParent->m_ConstDef;
  }

  //---------------------------------------------------------------------------
  /** \brief Extract all characters that belong to a certain charset.

    \param a_szCharSet [in] Const char array of the characters allowed in the token. 
    \param a_strTok [out]  The string that consists entirely of characters listed in a_szCharSet.
    \param a_iPos [in] Position in the string from where to start reading.
    \return The Position of the first character not listed in a_szCharSet.
    \throw nothrow
  */
  int TokenReader::ExtractToken(const char_type *a_szCharSet, 
                                      string_type &a_sTok, 
                                      int a_iPos) const
  {
    int iEnd = (int)m_sExpr.find_first_not_of(a_szCharSet, a_iPos);

    if (iEnd==(int)string_type::npos)
        iEnd = (int)m_sExpr.length();
    
    // Assign token string if there was something found
    if (a_iPos!=iEnd)
      a_sTok = string_type( m_sExpr.begin()+a_iPos, m_sExpr.begin()+iEnd);

    return iEnd;
  }

  //---------------------------------------------------------------------------
  /** \brief Check Expression for the presence of a binary operator token.
  
    Userdefined binary operator "++" gives inconsistent parsing result for
    the equations "a++b" and "a ++ b" if alphabetic characters are allowed
    in operator tokens. To avoid this this function checks specifically
    for operator tokens.
  */
  int TokenReader::ExtractOperatorToken(string_type &a_sTok, 
                                              int a_iPos) const
  {
    int iEnd = (int)m_sExpr.find_first_not_of(MEC_OPRT_CHARS, a_iPos);
    if (iEnd==(int)string_type::npos)
      iEnd = (int)m_sExpr.length();

    // Assign token string if there was something found
    if (a_iPos!=iEnd)
    {
      a_sTok = string_type( m_sExpr.begin() + a_iPos, m_sExpr.begin() + iEnd);
      return iEnd;
    }
    else
    {
      // There is still the chance of having to deal with an operator consisting exclusively
      // of alphabetic characters.
      return ExtractToken(MEC_CHARS, a_sTok, a_iPos);
    }
  }

  //---------------------------------------------------------------------------
  /** \brief Check if a built in operator or other token can be found
      \param a_Tok  [out] Operator token if one is found. This can either be a binary operator or an infix operator token.
      \return true if an operator token has been found.
  */
  bool TokenReader::IsBuiltIn(Token &a_Tok)
  {
    const char_type **const pOprtDef = m_pParser->GetOprtDef(),
                     *const szFormula = m_sExpr.c_str();

    // Compare token with function and operator strings
    // check string for operator/function
    for (int i=0; pOprtDef[i]; i++)
    {
      std::size_t len( std::char_traits<char_type>::length(pOprtDef[i]) );
      if ( string_type(pOprtDef[i]) == string_type(szFormula + m_iPos, szFormula + m_iPos + len) )
      {
        switch(i)
        {
        case cmMIN:
        case cmMAX:
        case cmLE:
        case cmGE:
        case cmNEQ:
        case cmEQ:
        case cmLT:
        case cmGT:
        case cmAND:
        case cmOR:
        case cmADD:
        case cmSUB:
        case cmMUL:
        case cmDIV:
//        case cmMOD:
              if (m_iSynFlags & noOPT) 
              {
                // Maybe its an infix operator not an operator
                // Both operator types can share characters in 
                // their identifiers
                if ( IsInfixOpTok(a_Tok) ) 
                  return true;

                Error(ecUNEXPECTED_OPERATOR, m_iPos, pOprtDef[i]);
              }

              m_iSynFlags  = noBC | noOPT | noARG_SEP | noPOSTOP | noIF | noELSE;
              m_iSynFlags |= ( (i != cmEND) && ( i != cmBC) ) ? noEND : 0;
              break;

		    case cmBO:
              if (m_iSynFlags & noBO)
	              Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef[i]);
              
              if (m_lastTok.IsFunction())
                m_iSynFlags =        noOPT | noEND | noARG_SEP | noPOSTOP | noIF | noELSE;
              else
                m_iSynFlags = noBC | noOPT | noEND | noARG_SEP | noPOSTOP | noIF | noELSE;

              ++m_iBrackets;
              break;

		    case cmBC:
              if (m_iSynFlags & noBC)
                Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef[i]);

              m_iSynFlags  = noBO | noVAR | noVAL | noFUN | noINFIXOP;

              if (--m_iBrackets<0)
                Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef[i]);
              break;
      	
        case cmELSE:
              m_iSynFlags = noBC | noPOSTOP | noEND | noOPT | noIF | noELSE;
              break;

        case cmIF:
              m_iSynFlags = noBC | noPOSTOP | noEND | noOPT | noIF | noELSE;
              break;

		    default:      // The operator is listed in c_DefaultOprt, but not here. This is a bad thing...
              Error(ecINTERNAL_ERROR);
        } // switch operator id

        m_iPos += (int)len;
        a_Tok.Set( (ECmdCode)i, pOprtDef[i] );
        return true;
	    } // if operator string found
    } // end of for all operator strings
  
    return false;
  }

  //---------------------------------------------------------------------------
  bool TokenReader::IsArgSep(Token &a_Tok)
  {
    const char_type* szFormula = m_sExpr.c_str();

    if (szFormula[m_iPos]==m_cArgSep)
    {
      // copy the separator into null terminated string
      char_type szSep[2];
      szSep[0] = m_cArgSep;
      szSep[1] = 0;

      if (m_iSynFlags & noARG_SEP)
        Error(ecUNEXPECTED_ARG_SEP, m_iPos, szSep);

      m_iSynFlags  = noBC | noOPT | noEND | noARG_SEP | noPOSTOP ;
      m_iPos++;
      a_Tok.Set(cmARG_SEP, szSep);
      return true;
    }

    return false;
  }

  //---------------------------------------------------------------------------
  /** \brief Check for End of Formula.

      \return true if an end of formula is found false otherwise.
      \param a_Tok [out] If an eof is found the corresponding token will be stored there.
      \throw nothrow
      \sa IsOprt, IsFunTok, IsStrFunTok, IsValTok, IsVarTok, IsString, IsInfixOpTok, IsPostOpTok
  */
  bool TokenReader::IsEOF(Token &a_Tok)
  {
    const char_type* szFormula = m_sExpr.c_str();

    // check for EOF
    if ( !szFormula[m_iPos] || szFormula[m_iPos] == '\n')
    {
      if ( m_iSynFlags & noEND )
        Error(ecUNEXPECTED_EOF, m_iPos);

      if (m_iBrackets>0)
        Error(ecMISSING_PARENS, m_iPos, _T(")"));

      m_iSynFlags = 0;
      a_Tok.Set(cmEND);
      return true;
    }

    return false;
  }

  //---------------------------------------------------------------------------
  /** \brief Check if a string position contains a unary infix operator. 
      \return true if a function token has been found false otherwise.
  */
  bool TokenReader::IsInfixOpTok(Token &a_Tok)
  {
    string_type sTok;
    int iEnd = ExtractToken(m_pParser->ValidInfixOprtChars(), sTok, m_iPos);
    if (iEnd==m_iPos)
      return false;

    funmap_type::const_iterator item = m_pInfixOprtDef->find(sTok);
    if (item==m_pInfixOprtDef->end())
      return false;

    a_Tok.Set(cmOPRT_INFIX, item->second, sTok);
    m_iPos = (int)iEnd;

    if (m_iSynFlags & noINFIXOP) 
      Error(ecUNEXPECTED_OPERATOR, m_iPos, a_Tok.GetAsString());

    m_iSynFlags = noPOSTOP | noINFIXOP | noOPT | noBC; 
    return true;
  }

  //---------------------------------------------------------------------------
  /** \brief Check whether the token at a given position is a function token.
      \param a_Tok [out] If a value token is found it will be placed here.
      \throw ParserException if Syntaxflags do not allow a function at a_iPos
      \return true if a function token has been found false otherwise.
      \pre [assert] m_pParser!=0
  */
  bool TokenReader::IsBuiltInFunTok(Token &a_Tok)
  {
    string_type strTok;
    int iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd==m_iPos)
      return false;

    const char_type **const pOprt = m_pParser->GetOprtDef();
    assert(pOprt);

    // Check for intrinsic functions
    ECmdCode eFunTok = cmUNKNOWN;
    for (int i=cmSIN; i<=cmSQRT; i++)
    {
      if (strTok==string_type(pOprt[i]))
      {
        eFunTok = (ECmdCode)i;
        break;
      }
    } // end of for all operator strings

    if (eFunTok==cmUNKNOWN)
      return false;

    // Check if the next sign is an opening bracket
    const char_type *szExpr = m_sExpr.c_str();
    if (szExpr[iEnd]!='(')
      return false;

    a_Tok.Set(eFunTok, strTok);

    m_iPos = (int)iEnd;
    if (m_iSynFlags & noFUN)
      Error(ecUNEXPECTED_FUN, m_iPos-(int)a_Tok.GetAsString().length(), a_Tok.GetAsString());

    m_iSynFlags = noANY ^ noBO;
    return true;
  }

  //---------------------------------------------------------------------------
  /** \brief Check whether the token at a given position is a function token.
      \param a_Tok [out] If a value token is found it will be placed here.
      \throw ParserException if Syntaxflags do not allow a function at a_iPos
      \return true if a function token has been found false otherwise.
      \pre [assert] m_pParser!=0
  */
  bool TokenReader::IsFunTok(Token &a_Tok)
  {
    string_type strTok;
    int iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd==m_iPos)
      return false;

    funmap_type::const_iterator item = m_pFunDef->find(strTok);
    if (item==m_pFunDef->end())
      return false;

    // Check if the next sign is an opening bracket
    const char_type *szExpr = m_sExpr.c_str();
    if (szExpr[iEnd]!='(')
      return false;

    a_Tok.Set(cmFUNC, item->second, strTok);

    m_iPos = (int)iEnd;
    if (m_iSynFlags & noFUN)
      Error(ecUNEXPECTED_FUN, m_iPos-(int)a_Tok.GetAsString().length(), a_Tok.GetAsString());

    m_iSynFlags = noANY ^ noBO;
    return true;
  }

  //---------------------------------------------------------------------------
  /** \brief Check if a string position contains a binary operator.
      \param a_Tok  [out] Operator token if one is found. This can either be a binary operator or an infix operator token.
      \return true if an operator token has been found.
  */
  bool TokenReader::IsOprt(Token &a_Tok)
  {
    const char_type *const szExpr = m_sExpr.c_str();
    string_type strTok;

    int iEnd = ExtractOperatorToken(strTok, m_iPos);
    if (iEnd==m_iPos)
      return false;

    // Check if the operator is a built in operator, if so ignore it here
    const char_type **const pOprtDef = m_pParser->GetOprtDef();
    for (int i=0; pOprtDef[i]; ++i)
    {
      if (string_type(pOprtDef[i])==strTok)
        return false;
    }

    // Note:
    // All tokens in oprt_bin_maptype are have been sorted by their length
    // Long operators must come first! Otherwise short names (like: "add") that
    // are part of long token names (like: "add123") will be found instead 
    // of the long ones.
    // Length sorting is done with ascending length so we use a reverse iterator here.
    funmap_type::const_reverse_iterator it = m_pOprtDef->rbegin();
    for ( ; it!=m_pOprtDef->rend(); ++it)
    {
      const string_type &sID = it->first;
      if ( sID == string_type(szExpr + m_iPos, szExpr + m_iPos + sID.length()) )
      {
        a_Tok.Set(cmOPRT_BIN, it->second, strTok);

        // operator was found
        if (m_iSynFlags & noOPT) 
        {
          // An operator was found but is not expected to occur at
          // this position of the formula, maybe it is an infix 
          // operator, not a binary operator. Both operator types
          // can share characters in their identifiers.
          if ( IsInfixOpTok(a_Tok) ) 
            return true;
          // nope, no infix operator
          Error(ecUNEXPECTED_OPERATOR, m_iPos, a_Tok.GetAsString()); 
        }

        m_iPos += (int)sID.length();
        m_iSynFlags  = noBC | noOPT | noARG_SEP | noPOSTOP | noEND | noBC;
        return true;
      }
    }

    return false;
  }

  //---------------------------------------------------------------------------
  /** \brief Check if a string position contains a unary post value operator. */
  bool TokenReader::IsPostOpTok(Token &a_Tok)
  {
    // Tricky problem with equations like "3m+5":
    //     m is a postfix operator, + is a valid sign for postfix operators and 
    //     for binary operators parser detects "m+" as operator string and 
    //     finds no matching postfix operator.
    // 
    // This is a special case so this routine slightly differs from the other
    // token readers.
    
    // Test if there could be a postfix operator
    string_type sTok;
    int iEnd = ExtractToken(m_pParser->ValidOprtChars(), sTok, m_iPos);
    if (iEnd==m_iPos)
      return false;

    // iteraterate over all postfix operator strings
    funmap_type::const_iterator item = m_pPostOprtDef->begin();
    for (item=m_pPostOprtDef->begin(); item!=m_pPostOprtDef->end(); ++item)
    {
      if (sTok.find(item->first)!=0)
        continue;

      a_Tok.Set(cmOPRT_POSTFIX, item->second, sTok);
  	  m_iPos += (int)item->first.length();

      if (m_iSynFlags & noPOSTOP)
        Error(ecUNEXPECTED_OPERATOR, m_iPos-(int)item->first.length(), item->first);

      m_iSynFlags = noVAL | noVAR | noFUN | noBO | noPOSTOP;
      return true;
    }

    return false;
  }

  //---------------------------------------------------------------------------
  /** \brief Check whether the token at a given position is a value token.

    Value tokens are either values or constants.

    \param a_Tok [out] If a value token is found it will be placed here.
    \return true if a value token has been found.
  */
  bool TokenReader::IsValTok(Token &a_Tok)
  {
    assert(m_pConstDef);
    assert(m_pParser);

    #if defined(_MSC_VER)
      #pragma warning( disable : 4244 )
    #endif

    string_type strTok;
    value_type fVal(0);
    int iEnd(0);
    
    // 2.) Check for user defined constant
    // Read everything that could be a constant name
    iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd!=m_iPos)
    {
      valmap_type::const_iterator item = m_pConstDef->find(strTok);
      if (item!=m_pConstDef->end())
      {
        m_iPos = iEnd;
        a_Tok.SetVal(item->second, strTok);

        if (m_iSynFlags & noVAL)
          Error(ecUNEXPECTED_VAL, m_iPos - (int)strTok.length(), strTok);

        m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP; 
        return true;
      }
    }

    // 3.call the value recognition functions provided by the user
    // Call user defined value recognition functions
    std::vector<identfun_type>::const_iterator item = m_vIdentFun.begin();
    for (item = m_vIdentFun.begin(); item!=m_vIdentFun.end(); ++item)
    {
      int iStart = m_iPos;
      if ( (*item)(m_sExpr.c_str() + m_iPos, &m_iPos, &fVal)==1 )
      {
        strTok.assign(m_sExpr.c_str(), iStart, m_iPos);
        if (m_iSynFlags & noVAL)
          Error(ecUNEXPECTED_VAL, m_iPos - (int)strTok.length(), strTok);

        a_Tok.SetVal(fVal, strTok);
        m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP;
        return true;
      }
    }

    return false;

    #if defined(_MSC_VER)
      #pragma warning( default : 4244 )
    #endif
  }

  //---------------------------------------------------------------------------
  /** \brief Check wheter a token at a given position is a variable token. 
      \param a_Tok [out] If a variable token has been found it will be placed here.
	    \return true if a variable token has been found.
  */
  bool TokenReader::IsVarTok(Token &a_Tok)
  {
    if (!m_pVarDef->size())
      return false;

    string_type strTok;
    int iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd==m_iPos)
      return false;

    varmap_type::const_iterator item =  m_pVarDef->find(strTok);
    if (item==m_pVarDef->end())
      return false;

    if (m_iSynFlags & noVAR)
      Error(ecUNEXPECTED_VAR, m_iPos, strTok);

    m_iPos = iEnd;
    a_Tok.SetVar(item->second, strTok);
    m_UsedVar[item->first] = item->second;  // Add variable to used-var-list

    m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP;

//  Zur Info hier die SynFlags von IsVal():
//    m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP | noSTR | noASSIGN; 
    return true;
  }

  //---------------------------------------------------------------------------
  /** \brief Check wheter a token at a given position is an undefined variable. 

      \param a_Tok [out] If a variable tom_pParser->m_vStringBufken has been found it will be placed here.
	    \return true if a variable token has been found.
      \throw nothrow
  */
  bool TokenReader::IsUndefVarTok(Token &a_Tok)
  {
    string_type strTok;
    int iEnd( ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos) );
    if ( iEnd==m_iPos )
      return false;

    if (m_iSynFlags & noVAR)
    {
      // <ibg/> 20061021 added token string strTok instead of a_Tok.GetAsString() as the 
      //                 token identifier. 
      // related bug report:
      // http://sourceforge.net/tracker/index.php?func=detail&aid=1578779&group_id=137191&atid=737979
      Error(ecUNEXPECTED_VAR, m_iPos - (int)a_Tok.GetAsString().length(), strTok);
    }

    // If a factory is available implicitely create new variables
    if (m_pFactory)
    {
      value_type *fVar = m_pFactory(strTok.c_str(), m_pFactoryData);
      a_Tok.SetVar(fVar, strTok );

      // Do not use m_pParser->DefineVar( strTok, fVar );
      // in order to define the new variable, it will clear the
      // m_UsedVar array which will kill previousely defined variables
      // from the list
      // This is safe because the new variable can never override an existing one
      // because they are checked first!
      (*m_pVarDef)[strTok] = fVar;
      m_UsedVar[strTok] = fVar;  // Add variable to used-var-list
    }
    else
    {
      a_Tok.SetVar((value_type*)&m_fZero, strTok);
      m_UsedVar[strTok] = 0;  // Add variable to used-var-list
    }

    m_iPos = iEnd;

    // Call the variable factory in order to let it define a new parser variable
    m_iSynFlags = noVAL | noVAR | noFUN | noBO | noPOSTOP | noINFIXOP;
    return true;
  }

  //---------------------------------------------------------------------------
  /** \brief Create an error containing the parse error position.

    This function will create an Parser Exception object containing the error text and its position.

    \param a_iErrc [in] The error code of type #EErrorCodes.
    \param a_iPos [in] The position where the error was detected.
    \param a_strTok [in] The token string representation associated with the error.
    \throw ParserException always throws thats the only purpose of this function.
  */
  void  TokenReader::Error( EErrorCodes a_iErrc, 
                                  int a_iPos, 
                                  const string_type &a_sTok) const
  {
    m_pParser->Error(a_iErrc, a_iPos, a_sTok);
  }

  //---------------------------------------------------------------------------
  void TokenReader::SetArgSep(char_type cArgSep)
  {
    m_cArgSep = cArgSep;
  }

  //---------------------------------------------------------------------------
  char_type TokenReader::GetArgSep() const
  {
    return m_cArgSep;
  }
} // namespace mec


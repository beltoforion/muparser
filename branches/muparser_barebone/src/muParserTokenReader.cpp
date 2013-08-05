/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
  Copyright (C) 2013 Ingo Berg

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

#include "muParserTokenReader.h"
#include "muParserBase.h"

/** \file
    \brief This file contains the parser token reader implementation.
*/


namespace mu
{

  // Forward declaration
  class ParserBase;

  //---------------------------------------------------------------------------
  /** \brief Copy constructor.

      \sa Assign
      \throw nothrow
  */
  ParserTokenReader::ParserTokenReader(const ParserTokenReader &a_Reader) 
  { 
    Assign(a_Reader);
  }
    
  //---------------------------------------------------------------------------
  /** \brief Assignement operator.

      Self assignement will be suppressed otherwise #Assign is called.

      \param a_Reader Object to copy to this token reader.
      \throw nothrow
  */
  ParserTokenReader& ParserTokenReader::operator=(const ParserTokenReader &a_Reader) 
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
  void ParserTokenReader::Assign(const ParserTokenReader &a_Reader)
  {
    m_pParser = a_Reader.m_pParser;
    m_sExpr = a_Reader.m_sExpr;
    m_iPos = a_Reader.m_iPos;
    m_iSynFlags = a_Reader.m_iSynFlags;
    
    m_pFunDef         = a_Reader.m_pFunDef;
    m_pConstDef       = a_Reader.m_pConstDef;
    m_pVarDef         = a_Reader.m_pVarDef;
    m_pInfixOprtDef   = a_Reader.m_pInfixOprtDef;
    m_vIdentFun       = a_Reader.m_vIdentFun;
    m_iBrackets       = a_Reader.m_iBrackets;
  }

  //---------------------------------------------------------------------------
  /** \brief Constructor. 
      
      Create a Token reader and bind it to a parser object. 

      \pre [assert] a_pParser may not be nullptr
      \param a_pParent Parent parser object of the token reader.
  */
  ParserTokenReader::ParserTokenReader(ParserBase *a_pParent)
    :m_pParser(a_pParent)
    ,m_sExpr()
    ,m_iPos(0)
    ,m_iSynFlags(0)
    ,m_pFunDef(nullptr)
    ,m_pInfixOprtDef(nullptr)
    ,m_pConstDef(nullptr)
    ,m_pVarDef(nullptr)
    ,m_vIdentFun()
    ,m_fZero(0)
    ,m_iBrackets(0)
    ,m_lastTok()
  {
    assert(m_pParser);
    SetParent(m_pParser);
  }
    
  //---------------------------------------------------------------------------
  /** \brief Create instance of a ParserTokenReader identical with this 
              and return its pointer. 

      This is a factory method the calling function must take care of the object destruction.

      \return A new ParserTokenReader object.
      \throw nothrow
  */
  ParserTokenReader* ParserTokenReader::Clone(ParserBase *a_pParent) const
  {
    std::auto_ptr<ParserTokenReader> ptr(new ParserTokenReader(*this));
    ptr->SetParent(a_pParent);
    return ptr.release();
  }

  //---------------------------------------------------------------------------
  ParserTokenReader::token_type& ParserTokenReader::SaveBeforeReturn(const token_type &tok)
  {
    m_lastTok = tok;
    return m_lastTok;
  }

  //---------------------------------------------------------------------------
  void ParserTokenReader::AddValIdent(identfun_type a_pCallback)
  {
    // Use push_front is used to give user defined callbacks a higher priority than
    // the built in ones. Otherwise reading hex numbers would not work
    // since the "0" in "0xff" would always be read first making parsing of 
    // the rest impossible.
    // reference:
    // http://sourceforge.net/projects/muparser/forums/forum/462843/topic/4824956
    m_vIdentFun.push_front(a_pCallback);
  }

  //---------------------------------------------------------------------------
  /** \brief Return the current position of the token reader in the formula string. 

      \return #m_iPos
      \throw nothrow
  */
  int ParserTokenReader::GetPos() const
  {
    return m_iPos;
  }

  //---------------------------------------------------------------------------
  /** \brief Return a reference to the formula. 

      \return #m_sExpr
      \throw nothrow
  */
  const string_type& ParserTokenReader::GetExpr() const
  {
    return m_sExpr;
  }

  //---------------------------------------------------------------------------
  /** \brief Initialize the token Reader. 
  
      Sets the formula position index to zero and set Syntax flags to default for initial formula parsing.
      \pre [assert] triggered if a_szFormula==0
  */
  void ParserTokenReader::SetFormula(const string_type &a_sExpr)
  {
    m_sExpr = a_sExpr;
    ReInit();
  }

  //---------------------------------------------------------------------------
  /** \brief Reset the token reader to the start of the formula. 

      The syntax flags will be reset to a value appropriate for the 
      start of a formula.
      \post #m_iPos==0, #m_iSynFlags = noOPT | noBC | noPOSTOP | noSTR
      \throw nothrow
      \sa ESynCodes
  */
  void ParserTokenReader::ReInit()
  {
    m_iPos = 0;
    m_iSynFlags = sfSTART_OF_LINE;
    m_iBrackets = 0;
    m_lastTok = token_type();
  }

  //---------------------------------------------------------------------------
  /** \brief Read the next token from the string. */ 
  ParserTokenReader::token_type ParserTokenReader::ReadNextToken()
  {
    assert(m_pParser);

    std::stack<int> FunArgs;
    const char_type *szFormula = m_sExpr.c_str();
    token_type tok;

    // Ignore all non printable characters when reading the expression
    while (szFormula[m_iPos]>0 && szFormula[m_iPos]<=0x20) 
      ++m_iPos;

    if ( IsEOF(tok) )        return SaveBeforeReturn(tok); // Check for end of formula
    if ( IsFunTok(tok) )     return SaveBeforeReturn(tok); // Check for function token
    if ( IsBuiltIn(tok) )    return SaveBeforeReturn(tok); // Check built in operators / tokens
    if ( IsArgSep(tok) )     return SaveBeforeReturn(tok); // Check for function argument separators
    if ( IsValTok(tok) )     return SaveBeforeReturn(tok); // Check for values / constant tokens
    if ( IsVarTok(tok) )     return SaveBeforeReturn(tok); // Check for variable tokens
    if ( IsInfixOpTok(tok) ) return SaveBeforeReturn(tok); // Check for unary operators

    // Check for unknown token
    // 
    // !!! From this point on there is no exit without an exception possible...
    // 
    string_type strTok;
    int iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd!=m_iPos)
      Error(ecUNASSIGNABLE_TOKEN, m_iPos, strTok);

    Error(ecUNASSIGNABLE_TOKEN, m_iPos, m_sExpr.substr(m_iPos));
    return token_type(); // never reached
  }

  //---------------------------------------------------------------------------
  void ParserTokenReader::SetParent(ParserBase *a_pParent)
  {
    m_pParser       = a_pParent; 
    m_pFunDef       = &a_pParent->m_FunDef;
    m_pInfixOprtDef = &a_pParent->m_InfixOprtDef;
    m_pVarDef       = &a_pParent->m_VarDef;
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
  int ParserTokenReader::ExtractToken(const char_type *a_szCharSet, 
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
  int ParserTokenReader::ExtractOperatorToken(string_type &a_sTok, 
                                              int a_iPos) const
  {
    int iEnd = (int)m_sExpr.find_first_not_of(m_pParser->ValidInfixOprtChars(), a_iPos);
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
      return ExtractToken(MUP_CHARS, a_sTok, a_iPos);
    }
  }

  //---------------------------------------------------------------------------
  /** \brief Check if a built in operator or other token can be found
      \param a_Tok  [out] Operator token if one is found. This can either be a binary operator or an infix operator token.
      \return true if an operator token has been found.
  */
  bool ParserTokenReader::IsBuiltIn(token_type &a_Tok)
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
        case cmLAND:
        case cmLOR:
        case cmLT:
        case cmGT:
        case cmLE:
        case cmGE:
        case cmNEQ:  
        case cmEQ:
        case cmADD:
        case cmSUB:
        case cmMUL:
        case cmDIV:
        case cmPOW:
              if (m_iSynFlags & noOPT) 
              {
                // Maybe its an infix operator not an operator
                // Both operator types can share characters in 
                // their identifiers
                if ( IsInfixOpTok(a_Tok) ) 
                  return true;

                Error(ecUNEXPECTED_OPERATOR, m_iPos, pOprtDef[i]);
              }

              m_iSynFlags  = noBC | noOPT | noARG_SEP;
              m_iSynFlags |= ( (i != cmEND) && ( i != cmBC) ) ? noEND : 0;
              break;

		    case cmBO:
              if (m_iSynFlags & noBO)
	              Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef[i]);
              
              if (m_lastTok.GetCode()==cmFUNC)
                m_iSynFlags = noOPT | noEND | noARG_SEP;
              else
                m_iSynFlags = noBC | noOPT | noEND | noARG_SEP;

              ++m_iBrackets;
              break;

		    case cmBC:
              if (m_iSynFlags & noBC)
                Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef[i]);

              m_iSynFlags  = noBO | noVAR | noVAL | noFUN | noINFIXOP;

              if (--m_iBrackets<0)
                Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef[i]);
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
  bool ParserTokenReader::IsArgSep(token_type &a_Tok)
  {
    const char_type* szFormula = m_sExpr.c_str();

    if (szFormula[m_iPos]==',')
    {
      // copy the separator into nullptr terminated string
      char_type szSep[2];
      szSep[0] = ',';
      szSep[1] = 0;

      if (m_iSynFlags & noARG_SEP)
        Error(ecUNEXPECTED_ARG_SEP, m_iPos, szSep);

      m_iSynFlags  = noBC | noOPT | noEND | noARG_SEP;
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
      \sa IsOprt, IsFunTok, IsValTok, IsVarTok, IsInfixOpTok
  */
  bool ParserTokenReader::IsEOF(token_type &a_Tok)
  {
    const char_type* szFormula = m_sExpr.c_str();

    // check for EOF
    if ( !szFormula[m_iPos])
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
  bool ParserTokenReader::IsInfixOpTok(token_type &a_Tok)
  {
    string_type sTok;
    int iEnd = ExtractToken(m_pParser->ValidInfixOprtChars(), sTok, m_iPos);
    if (iEnd==m_iPos)
      return false;

    // iteraterate over all postfix operator strings
    funmap_type::const_reverse_iterator it = m_pInfixOprtDef->rbegin();
    for ( ; it!=m_pInfixOprtDef->rend(); ++it)
    {
      if (sTok.find(it->first)!=0)
        continue;

      a_Tok.Set(it->second, it->first);
      m_iPos += (int)it->first.length();

      if (m_iSynFlags & noINFIXOP) 
        Error(ecUNEXPECTED_OPERATOR, m_iPos, a_Tok.GetAsString());

      m_iSynFlags = noINFIXOP | noOPT | noBC;
      return true;
    }

    return false;
  }

  //---------------------------------------------------------------------------
  /** \brief Check whether the token at a given position is a function token.
      \param a_Tok [out] If a value token is found it will be placed here.
      \throw ParserException if Syntaxflags do not allow a function at a_iPos
      \return true if a function token has been found false otherwise.
      \pre [assert] m_pParser!=0
  */
  bool ParserTokenReader::IsFunTok(token_type &a_Tok)
  {
    string_type strTok;
    int iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd==m_iPos)
      return false;

    funmap_type::const_iterator item = m_pFunDef->find(strTok);
    if (item==m_pFunDef->end())
      return false;

    // Check if the next sign is an opening bracket
    const char_type *szFormula = m_sExpr.c_str();
    if (szFormula[iEnd]!='(')
      return false;

    a_Tok.Set(item->second, strTok);

    m_iPos = (int)iEnd;
    if (m_iSynFlags & noFUN)
      Error(ecUNEXPECTED_FUN, m_iPos-(int)a_Tok.GetAsString().length(), a_Tok.GetAsString());

    m_iSynFlags = noANY ^ noBO;
    return true;
  }

  //---------------------------------------------------------------------------
  /** \brief Check whether the token at a given position is a value token.

    Value tokens are either values or constants.

    \param a_Tok [out] If a value token is found it will be placed here.
    \return true if a value token has been found.
  */
  bool ParserTokenReader::IsValTok(token_type &a_Tok)
  {
    assert(m_pConstDef);
    assert(m_pParser);

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
    std::list<identfun_type>::const_iterator item = m_vIdentFun.begin();
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
  }

  //---------------------------------------------------------------------------
  /** \brief Check wheter a token at a given position is a variable token. 
      \param a_Tok [out] If a variable token has been found it will be placed here.
	    \return true if a variable token has been found.
  */
  bool ParserTokenReader::IsVarTok(token_type &a_Tok)
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

    m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP;

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
  void  ParserTokenReader::Error( EErrorCodes a_iErrc, 
                                  int a_iPos, 
                                  const string_type &a_sTok) const
  {
    m_pParser->Error(a_iErrc, a_iPos, a_sTok);
  }
} // namespace mu


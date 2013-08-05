#ifndef MEC_TOKEN_H
#define MEC_TOKEN_H

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
#include <string>
#include <stack>
#include <vector>
#include <memory>

#include "mecError.h"
#include "mecCallback.h"

/** \file
    \brief This file contains the parser token definition.
*/

namespace mec
{
  #pragma pack(push, 1) 
  struct SPackedToken
  {
    index_type  m_nStackPos;
    index_type  m_eCode;
    
    union
    {
      value_type  m_fVal;
      value_type *m_pVar;
      
      struct SCallback
      {
        index_type m_nArgc;
        union
        {
          void       *m_pFun;
          fun_type0   m_pFun0;
          fun_type1   m_pFun1;
          fun_type2   m_pFun2;
          fun_type3   m_pFun3;
          fun_type4   m_pFun4;
          fun_type5   m_pFun5;
          fun_type6   m_pFun6;
          fun_type7   m_pFun7;
          fun_type8   m_pFun8;
          fun_type9   m_pFun9;
          fun_type10  m_pFun10;
        };
      } Fun; // Union SCallback

      struct SJump
      {
        index_type offset;
      } Jmp;
    }; // anonymous union
  }; // struct SToken
  #pragma pack(pop)


  /** \brief Encapsulation of the data for a single formula token. 

    Formula token implementation. Part of the Math Parser Package.
    Formula tokens can be either one of the following:
    <ul>
      <li>value</li>
      <li>variable</li>
      <li>function with numerical arguments</li>
      <li>functions with a string as argument</li>
      <li>prefix operators</li>
      <li>infix operators</li>
	    <li>binary operator</li>
    </ul>

   \author (C) 2011 Ingo Berg 
  */
  class Token
  {
  private:

      ECmdCode    m_eCode;    ///< Type of the token; The token type is a constant of type #ECmdCode.

      value_type *m_pVar;     ///< Stores variable pointers
      value_type  m_fVal;     ///< Stores values directly

      int  m_iFlags;          ///< Additional flags for the token.
      std::auto_ptr<Callback> m_pCallback;
      string_type m_sTok;   ///< Token string

  public:

      Token();
      Token(const Token &a_Tok);
      Token& operator=(const Token &a_Tok);

      void Assign(const Token &a_Tok);
      void AddFlags(int a_iFlags);
      bool IsFlagSet(int a_iFlags) const;

      Token& Set(ECmdCode eCode, const string_type &a_strTok=string_type());
      Token& Set(ECmdCode eCode, const Callback &a_pCallback, const string_type &a_sTok);
      Token& SetVal(value_type  a_fVal, const string_type &a_strTok=string_type());
      Token& SetVar(value_type *a_pVar, const string_type &a_strTok);

      int GetArgCount() const;
      bool IsFunction() const;

      ECmdCode  GetCode() const;
      Callback* GetCallback() const;
      value_type  GetVal() const;
      value_type* GetVar() const;
      const string_type& GetAsString() const;
  };
} // namespace mec

#endif

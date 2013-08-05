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

#ifndef MEC_CALLBACK_H
#define MEC_CALLBACK_H

#include "mecDef.h"

/** \file
    \brief Definition of the parser callback class.
*/

namespace mec
{
  /** \brief Encapsulates data of a parser callback.
  */
  struct Callback
  {
    Callback(fun_type0 a_pFun, int a_nFlags = 0);
    Callback(fun_type1 a_pFun, int a_nPrec = -1, int flags = 0);
    Callback(fun_type2 a_pFun, int a_nPrec, EOprtAssociativity a_eAssociativity, int a_nFlags = 0);
    Callback(fun_type2 a_pFun, int a_nFlags = 0);
    Callback(fun_type3 a_pFun, int a_nFlags = 0);
    Callback(fun_type4 a_pFun, int a_nFlags = 0);
    Callback(fun_type5 a_pFun, int a_nFlags = 0);
    Callback(fun_type6 a_pFun, int a_nFlags = 0);
    Callback(fun_type7 a_pFun, int a_nFlags = 0);
    Callback(fun_type8 a_pFun, int a_nFlags = 0);
    Callback(fun_type9 a_pFun, int a_nFlags = 0);
    Callback(fun_type10 a_pFun, int a_nFlags = 0);
    Callback();

    union
    {
      void      *m_pFun;
      fun_type0  m_pFun0;
      fun_type1  m_pFun1;
      fun_type2  m_pFun2;
      fun_type3  m_pFun3;
      fun_type4  m_pFun4;
      fun_type5  m_pFun5;
    };

    int   m_nArgc;                  ///< Number of function arguments   
    int   m_nPrec;                  ///< Valid only for binary and infix operators; Operator precedence.
    int   m_nFlags;                 ///< Reserved for future use
    EOprtAssociativity m_eOprtAsct; ///< Operator associativity; Valid only for binary operators 
  };

//------------------------------------------------------------------------------
/** \brief Container for Callback objects. */
typedef std::map<string_type, Callback> funmap_type; 

} // namespace mec

#endif


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

#include "mecCallback.h"

/** \file
    \brief Implementation of the parser callback class.
*/
namespace mec
{
  //---------------------------------------------------------------------------
  Callback::Callback(fun_type0 a_pFun, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(0)
    ,m_nPrec(-1)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}

  //---------------------------------------------------------------------------
  Callback::Callback(fun_type1 a_pFun, int a_iPrec, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(1)
    ,m_nPrec(a_iPrec)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}


  //---------------------------------------------------------------------------
  /** \brief Constructor for constructing funcstion callbacks taking two arguments. 
      \throw nothrow
  */
  Callback::Callback(fun_type2 a_pFun, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(2)
    ,m_nPrec(-1)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}

  //---------------------------------------------------------------------------
  /** \brief Constructor for constructing binary operator callbacks. 
      \param a_pFun Pointer to a static function taking two arguments
      \param a_bAllowOpti A flag indicating this funcation can be optimized
      \param a_iPrec The operator precedence
      \param a_eOprtAsct The operators associativity
      \throw nothrow
  */
  Callback::Callback(fun_type2 a_pFun, 
                     int a_iPrec, 
                     EOprtAssociativity a_eOprtAsct, 
                     int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(2)
    ,m_nPrec(a_iPrec)
    ,m_nFlags(flags)
    ,m_eOprtAsct(a_eOprtAsct)
  {}

  //---------------------------------------------------------------------------
  Callback::Callback(fun_type3 a_pFun, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(3)
    ,m_nPrec(-1)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}

  //---------------------------------------------------------------------------
  Callback::Callback(fun_type4 a_pFun, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(4)
    ,m_nPrec(-1)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}

  //---------------------------------------------------------------------------
  Callback::Callback(fun_type5 a_pFun, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(5)
    ,m_nPrec(-1)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}

  //---------------------------------------------------------------------------
  Callback::Callback(fun_type6 a_pFun, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(6)
    ,m_nPrec(-1)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}

  //---------------------------------------------------------------------------
  Callback::Callback(fun_type7 a_pFun, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(7)
    ,m_nPrec(-1)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}

  //---------------------------------------------------------------------------
  Callback::Callback(fun_type8 a_pFun, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(8)
    ,m_nPrec(-1)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}

  //---------------------------------------------------------------------------
  Callback::Callback(fun_type9 a_pFun, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(9)
    ,m_nPrec(-1)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}

  //---------------------------------------------------------------------------
  Callback::Callback(fun_type10 a_pFun, int flags)
    :m_pFun((void*)a_pFun)
    ,m_nArgc(10)
    ,m_nPrec(-1)
    ,m_nFlags(flags)
    ,m_eOprtAsct(oaNONE)
  {}

  //---------------------------------------------------------------------------
  /** \brief Default constructor. 
      \throw nothrow
  */
  Callback::Callback()
    :m_pFun(0)
    ,m_nArgc(0)
    ,m_nPrec(-1)
    ,m_nFlags(0)
    ,m_eOprtAsct(oaNONE)
  {}
} // namespace mec

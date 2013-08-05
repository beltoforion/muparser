/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
  Copyright (C) 2004-2011 Ingo Berg

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

#include "muParserCallback.h"

/** \file
    \brief Implementation of the parser callback class.
*/


namespace mu
{
  //---------------------------------------------------------------------------
  ParserCallback::ParserCallback(fun_type0 a_pFun, bool a_bAllowOpti)
    :m_pFun((void*)a_pFun)
    ,m_iArgc(0)
    ,m_iPri(-1)
    ,m_eOprtAsct(oaNONE)
    ,m_iCode(cmFUNC)
  {}

  //---------------------------------------------------------------------------
  ParserCallback::ParserCallback(fun_type1 a_pFun, bool a_bAllowOpti, int a_iPrec, ECmdCode a_iCode)
    :m_pFun((void*)a_pFun)
    ,m_iArgc(1)
    ,m_iPri(a_iPrec)
    ,m_eOprtAsct(oaNONE)
    ,m_iCode(a_iCode)
  {}


  //---------------------------------------------------------------------------
  /** \brief Constructor for constructing funcstion callbacks taking two arguments. 
      \throw nothrow
  */
  ParserCallback::ParserCallback(fun_type2 a_pFun, bool a_bAllowOpti)
    :m_pFun((void*)a_pFun)
    ,m_iArgc(2)
    ,m_iPri(-1)
    ,m_eOprtAsct(oaNONE)
    ,m_iCode(cmFUNC)
  {}

  //---------------------------------------------------------------------------
  ParserCallback::ParserCallback(fun_type3 a_pFun, bool a_bAllowOpti)
    :m_pFun((void*)a_pFun)
    ,m_iArgc(3)
    ,m_iPri(-1)
    ,m_eOprtAsct(oaNONE)
    ,m_iCode(cmFUNC)
  {}


  //---------------------------------------------------------------------------
  ParserCallback::ParserCallback(bulkfun_type0 a_pFun, bool a_bAllowOpti)
    :m_pFun((void*)a_pFun)
    ,m_iArgc(0)
    ,m_iPri(-1)
    ,m_eOprtAsct(oaNONE)
    ,m_iCode(cmFUNC_BULK)
  {}

  //---------------------------------------------------------------------------
  ParserCallback::ParserCallback(bulkfun_type1 a_pFun, bool a_bAllowOpti)
    :m_pFun((void*)a_pFun)
    ,m_iArgc(1)
    ,m_iPri(-1)
    ,m_eOprtAsct(oaNONE)
    ,m_iCode(cmFUNC_BULK)
  {}


  //---------------------------------------------------------------------------
  /** \brief Constructor for constructing funcstion callbacks taking two arguments. 
      \throw nothrow
  */
  ParserCallback::ParserCallback(bulkfun_type2 a_pFun, bool a_bAllowOpti)
    :m_pFun((void*)a_pFun)
    ,m_iArgc(2)
    ,m_iPri(-1)
    ,m_eOprtAsct(oaNONE)
    ,m_iCode(cmFUNC_BULK)
  {}

  //---------------------------------------------------------------------------
  ParserCallback::ParserCallback(bulkfun_type3 a_pFun, bool a_bAllowOpti)
    :m_pFun((void*)a_pFun)
    ,m_iArgc(3)
    ,m_iPri(-1)
    ,m_eOprtAsct(oaNONE)
    ,m_iCode(cmFUNC_BULK)
  {}

  //---------------------------------------------------------------------------
  /** \brief Default constructor. 
      \throw nothrow
  */
  ParserCallback::ParserCallback()
    :m_pFun(0)
    ,m_iArgc(0)
    ,m_iPri(-1)
    ,m_eOprtAsct(oaNONE)
    ,m_iCode(cmUNKNOWN)
  {}


  //---------------------------------------------------------------------------
  /** \brief Copy constructor. 
      \throw nothrow
  */
  ParserCallback::ParserCallback(const ParserCallback &ref)
  {
    m_pFun       = ref.m_pFun;
    m_iArgc      = ref.m_iArgc;
    m_iCode      = ref.m_iCode;
    m_iPri       = ref.m_iPri;
    m_eOprtAsct  = ref.m_eOprtAsct;
  }

  //---------------------------------------------------------------------------
  /** \brief Clone this instance and return a pointer to the new instance. */
  ParserCallback* ParserCallback::Clone() const
  {
    return new ParserCallback(*this);
  }

  //---------------------------------------------------------------------------
  /** \brief Get the callback address for the parser function. 
  
      The type of the address is void. It needs to be recasted according to the
      argument number to the right type.

      \throw nothrow
      \return #pFun
  */
  void* ParserCallback::GetAddr() const 
  { 
    return m_pFun;  
  }

  //---------------------------------------------------------------------------
  /** \brief Return the callback code. */
  ECmdCode  ParserCallback::GetCode() const 
  { 
    return m_iCode; 
  }
  
  //---------------------------------------------------------------------------
  /** \brief Return the operator precedence. 
      \throw nothrown

     Only valid if the callback token is an operator token (binary or infix).
  */
  int ParserCallback::GetPri()  const 
  { 
    return m_iPri;  
  }

  //---------------------------------------------------------------------------
  /** \brief Return the operators associativity. 
      \throw nothrown

     Only valid if the callback token is a binary operator token.
  */
  EOprtAssociativity ParserCallback::GetAssociativity() const
  {
    return m_eOprtAsct;
  }

  //---------------------------------------------------------------------------
  /** \brief Returns the number of function Arguments. */
  int ParserCallback::GetArgc() const 
  { 
    return m_iArgc; 
  }
} // namespace mu

/*
                 __________
    _____   __ __\______   \_____  _______  ______  ____ _______
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|
        \/                       \/            \/      \/
  Copyright (C) 2020 Ingo Berg

    Redistribution and use in source and binary forms, with or without modification, are permitted
    provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, this list of
        conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, this list of
        conditions and the following disclaimer in the documentation and/or other materials provided
        with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
    FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
    IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MU_PARSER_STACK_H
#define MU_PARSER_STACK_H

#include <cassert>
#include <string>
#include <stack>
#include <vector>

#include "muParserError.h"
#include "muParserToken.h"

/** \file 
    \brief This file defines the stack used by muparser.
*/

namespace mu
{

  /** \brief Parser stack implementation. 

      Stack implementation based on a std::stack. The behaviour of pop() had been
      slightly changed in order to get an error code if the stack is empty.
      The stack is used within the Parser both as a value stack and as an operator stack.

      \author (C) 2004-2011 Ingo Berg 
  */
  template <typename TValueType>
  class ParserStack 
  {
    private:

      /** \brief Type of the underlying stack implementation. */
      typedef std::stack<TValueType, std::vector<TValueType> > impl_type;
      
      impl_type m_Stack;  ///< This is the actual stack.

    public:	
  	 
      //---------------------------------------------------------------------------
      ParserStack()
        :m_Stack()
      {}

      //---------------------------------------------------------------------------
      virtual ~ParserStack()
      {}

      //---------------------------------------------------------------------------
      /** \brief Pop a value from the stack.
       
        Unlike the standard implementation this function will return the value that
        is going to be taken from the stack.

        \throw ParserException in case the stack is empty.
        \sa pop(int &a_iErrc)
      */
	    TValueType pop()
      {
        if (empty())
          throw ParserError( _T("stack is empty.") );

        TValueType el = top();
        m_Stack.pop();
        return el;
      }

      /** \brief Push an object into the stack. 

          \param a_Val object to push into the stack.
          \throw nothrow
      */
      void push(const TValueType& a_Val) 
      { 
        m_Stack.push(a_Val); 
      }

      /** \brief Return the number of stored elements. */
      unsigned size() const
      { 
        return (unsigned)m_Stack.size(); 
      }

      /** \brief Returns true if stack is empty false otherwise. */
      bool empty() const
      {
        return m_Stack.empty(); 
      }
       
      /** \brief Return reference to the top object in the stack. 
       
          The top object is the one pushed most recently.
      */
      TValueType& top() 
      { 
        return m_Stack.top(); 
      }
  };
} // namespace MathUtils

#endif

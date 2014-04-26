/***************************************************************************************************
 **
 **  Original work Copyright (C) 2013 Ingo Berg
 **  Modified work Copyright 2014 Roman Telezhinsky <dismine@gmail.com>
 **
 **  Permission is hereby granted, free of charge, to any person obtaining a copy of this
 **  software and associated documentation files (the "Software"), to deal in the Software
 **  without restriction, including without limitation the rights to use, copy, modify,
 **  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 **  permit persons to whom the Software is furnished to do so, subject to the following conditions:
 **
 **  The above copyright notice and this permission notice shall be included in all copies or
 **  substantial portions of the Software.
 **
 **  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 **  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 **  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 **  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 **  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **
 ******************************************************************************************************/

#include "qmuparserbytecode.h"

#include <cassert>
#include <string>
#include <stack>
#include <vector>
#include <iostream>

#include "qmuparserdef.h"
#include "qmuparsererror.h"
#include "qmuparsertoken.h"
#include "qmuparserstack.h"
#include "qmuparsertemplatemagic.h"


namespace qmu
{
  //---------------------------------------------------------------------------
  /** \brief Bytecode default constructor. */
  QmuParserByteCode::QmuParserByteCode()
    :m_iStackPos(0)
    ,m_iMaxStackSize(0)
    ,m_vRPN()
    ,m_bEnableOptimizer(true)
  {
    m_vRPN.reserve(50);
  }

  //---------------------------------------------------------------------------
  /** \brief Copy constructor. 
    
      Implemented in Terms of Assign(const QParserByteCode &a_ByteCode)
  */
  QmuParserByteCode::QmuParserByteCode(const QmuParserByteCode &a_ByteCode)
  {
    Assign(a_ByteCode);
  }

  //---------------------------------------------------------------------------
  /** \brief Assignment operator.
    
      Implemented in Terms of Assign(const QParserByteCode &a_ByteCode)
  */
  QmuParserByteCode& QmuParserByteCode::operator=(const QmuParserByteCode &a_ByteCode)
  {
    Assign(a_ByteCode);
    return *this;
  }

  //---------------------------------------------------------------------------
  void QmuParserByteCode::EnableOptimizer(bool bStat)
  {
    m_bEnableOptimizer = bStat;
  }

  //---------------------------------------------------------------------------
  /** \brief Copy state of another object to this. 
    
      \throw nowthrow
  */
  void QmuParserByteCode::Assign(const QmuParserByteCode &a_ByteCode)
  {
    if (this==&a_ByteCode)    
      return;

    m_iStackPos = a_ByteCode.m_iStackPos;
    m_vRPN = a_ByteCode.m_vRPN;
    m_iMaxStackSize = a_ByteCode.m_iMaxStackSize;
  }

  //---------------------------------------------------------------------------
  /** \brief Add a Variable pointer to bytecode. 
      \param a_pVar Pointer to be added.
      \throw nothrow
  */
  void QmuParserByteCode::AddVar(qreal *a_pVar)
  {
    ++m_iStackPos;
    m_iMaxStackSize = std::max(m_iMaxStackSize, (size_t)m_iStackPos);

    // optimization does not apply
    SToken tok;
    tok.Cmd       = cmVAR;
    tok.Val.ptr   = a_pVar;
    tok.Val.data  = 1;
    tok.Val.data2 = 0;
    m_vRPN.push_back(tok);
  }

  //---------------------------------------------------------------------------
  /** \brief Add a Variable pointer to bytecode. 

      Value entries in byte code consist of:
      <ul>
        <li>value array position of the value</li>
        <li>the operator code according to ParserToken::cmVAL</li>
        <li>the value stored in #mc_iSizeVal number of bytecode entries.</li>
      </ul>

      \param a_pVal Value to be added.
      \throw nothrow
  */
  void QmuParserByteCode::AddVal(qreal a_fVal)
  {
    ++m_iStackPos;
    m_iMaxStackSize = std::max(m_iMaxStackSize, (size_t)m_iStackPos);

    // If optimization does not apply
    SToken tok;
    tok.Cmd = cmVAL;
    tok.Val.ptr   = NULL;
    tok.Val.data  = 0;
    tok.Val.data2 = a_fVal;
    m_vRPN.push_back(tok);
  }

  //---------------------------------------------------------------------------
  void QmuParserByteCode::ConstantFolding(ECmdCode a_Oprt)
  {
    std::size_t sz = m_vRPN.size();
    qreal &x = m_vRPN[sz-2].Val.data2,
               &y = m_vRPN[sz-1].Val.data2;
    switch (a_Oprt)
    {
    case cmLAND: x = (int)x && (int)y; m_vRPN.pop_back(); break;
    case cmLOR:  x = (int)x || (int)y; m_vRPN.pop_back(); break;
    case cmLT:   x = x < y;  m_vRPN.pop_back();  break;
    case cmGT:   x = x > y;  m_vRPN.pop_back();  break;
    case cmLE:   x = x <= y; m_vRPN.pop_back();  break;
    case cmGE:   x = x >= y; m_vRPN.pop_back();  break;
    case cmNEQ:  x = x != y; m_vRPN.pop_back();  break;
    case cmEQ:   x = x == y; m_vRPN.pop_back();  break;
    case cmADD:  x = x + y;  m_vRPN.pop_back();  break;
    case cmSUB:  x = x - y;  m_vRPN.pop_back();  break;
    case cmMUL:  x = x * y;  m_vRPN.pop_back();  break;
    case cmDIV: 

#if defined(MUP_MATH_EXCEPTIONS)
        if (y==0)
          throw ParserError(ecDIV_BY_ZERO, "0");
#endif

        x = x / y;   
        m_vRPN.pop_back();
        break;

    case cmPOW: x = MathImpl<qreal>::Pow(x, y);
                m_vRPN.pop_back();
                break;

    default:
        break;
    } // switch opcode
  }

  //---------------------------------------------------------------------------
  /** \brief Add an operator identifier to bytecode. 
    
      Operator entries in byte code consist of:
      <ul>
        <li>value array position of the result</li>
        <li>the operator code according to ParserToken::ECmdCode</li>
      </ul>

      \sa  ParserToken::ECmdCode
  */
  void QmuParserByteCode::AddOp(ECmdCode a_Oprt)
  {
    bool bOptimized = false;

    if (m_bEnableOptimizer)
    {
      std::size_t sz = m_vRPN.size();

      // Check for foldable constants like:
      //   cmVAL cmVAL cmADD 
      // where cmADD can stand fopr any binary operator applied to
      // two constant values.
      if (sz>=2 && m_vRPN[sz-2].Cmd == cmVAL && m_vRPN[sz-1].Cmd == cmVAL)
      {
        ConstantFolding(a_Oprt);
        bOptimized = true;
      }
      else
      {
        switch(a_Oprt)
        {
        case  cmPOW:
              // Optimization for ploynomials of low order
              if (m_vRPN[sz-2].Cmd == cmVAR && m_vRPN[sz-1].Cmd == cmVAL)
              {
                if (m_vRPN[sz-1].Val.data2==2)
                  m_vRPN[sz-2].Cmd = cmVARPOW2;
                else if (m_vRPN[sz-1].Val.data2==3)
                  m_vRPN[sz-2].Cmd = cmVARPOW3;
                else if (m_vRPN[sz-1].Val.data2==4)
                  m_vRPN[sz-2].Cmd = cmVARPOW4;
                else
                  break;

                m_vRPN.pop_back();
                bOptimized = true;
              }
              break;

        case  cmSUB:
        case  cmADD:
              // Simple optimization based on pattern recognition for a shitload of different
              // bytecode combinations of addition/subtraction
              if ( (m_vRPN[sz-1].Cmd == cmVAR    && m_vRPN[sz-2].Cmd == cmVAL)    ||
                   (m_vRPN[sz-1].Cmd == cmVAL    && m_vRPN[sz-2].Cmd == cmVAR)    || 
                   (m_vRPN[sz-1].Cmd == cmVAL    && m_vRPN[sz-2].Cmd == cmVARMUL) ||
                   (m_vRPN[sz-1].Cmd == cmVARMUL && m_vRPN[sz-2].Cmd == cmVAL)    ||
                   (m_vRPN[sz-1].Cmd == cmVAR    && m_vRPN[sz-2].Cmd == cmVAR    && m_vRPN[sz-2].Val.ptr == m_vRPN[sz-1].Val.ptr) ||
                   (m_vRPN[sz-1].Cmd == cmVAR    && m_vRPN[sz-2].Cmd == cmVARMUL && m_vRPN[sz-2].Val.ptr == m_vRPN[sz-1].Val.ptr) ||
                   (m_vRPN[sz-1].Cmd == cmVARMUL && m_vRPN[sz-2].Cmd == cmVAR    && m_vRPN[sz-2].Val.ptr == m_vRPN[sz-1].Val.ptr) ||
                   (m_vRPN[sz-1].Cmd == cmVARMUL && m_vRPN[sz-2].Cmd == cmVARMUL && m_vRPN[sz-2].Val.ptr == m_vRPN[sz-1].Val.ptr) )
              {
                assert( (m_vRPN[sz-2].Val.ptr==NULL && m_vRPN[sz-1].Val.ptr!=NULL) ||
                        (m_vRPN[sz-2].Val.ptr!=NULL && m_vRPN[sz-1].Val.ptr==NULL) || 
                        (m_vRPN[sz-2].Val.ptr == m_vRPN[sz-1].Val.ptr) );

                m_vRPN[sz-2].Cmd = cmVARMUL;
                m_vRPN[sz-2].Val.ptr    = (qreal*)((long long)(m_vRPN[sz-2].Val.ptr) | (long long)(m_vRPN[sz-1].Val.ptr));    // variable
                m_vRPN[sz-2].Val.data2 += ((a_Oprt==cmSUB) ? -1 : 1) * m_vRPN[sz-1].Val.data2;  // offset
                m_vRPN[sz-2].Val.data  += ((a_Oprt==cmSUB) ? -1 : 1) * m_vRPN[sz-1].Val.data;   // multiplikatior
                m_vRPN.pop_back();
                bOptimized = true;
              } 
              break;

        case  cmMUL:
              if ( (m_vRPN[sz-1].Cmd == cmVAR && m_vRPN[sz-2].Cmd == cmVAL) ||
                   (m_vRPN[sz-1].Cmd == cmVAL && m_vRPN[sz-2].Cmd == cmVAR) ) 
              {
                m_vRPN[sz-2].Cmd        = cmVARMUL;
                m_vRPN[sz-2].Val.ptr    = (qreal*)((long long)(m_vRPN[sz-2].Val.ptr) | (long long)(m_vRPN[sz-1].Val.ptr));
                m_vRPN[sz-2].Val.data   = m_vRPN[sz-2].Val.data2 + m_vRPN[sz-1].Val.data2;
                m_vRPN[sz-2].Val.data2  = 0;
                m_vRPN.pop_back();
                bOptimized = true;
              } 
              else if ( (m_vRPN[sz-1].Cmd == cmVAL    && m_vRPN[sz-2].Cmd == cmVARMUL) ||
                        (m_vRPN[sz-1].Cmd == cmVARMUL && m_vRPN[sz-2].Cmd == cmVAL) )
              {
                // Optimization: 2*(3*b+1) or (3*b+1)*2 -> 6*b+2
                m_vRPN[sz-2].Cmd     = cmVARMUL;
                m_vRPN[sz-2].Val.ptr = (qreal*)((long long)(m_vRPN[sz-2].Val.ptr) | (long long)(m_vRPN[sz-1].Val.ptr));
                if (m_vRPN[sz-1].Cmd == cmVAL)
                {
                  m_vRPN[sz-2].Val.data  *= m_vRPN[sz-1].Val.data2;
                  m_vRPN[sz-2].Val.data2 *= m_vRPN[sz-1].Val.data2;
                }
                else
                {
                  m_vRPN[sz-2].Val.data  = m_vRPN[sz-1].Val.data  * m_vRPN[sz-2].Val.data2;
                  m_vRPN[sz-2].Val.data2 = m_vRPN[sz-1].Val.data2 * m_vRPN[sz-2].Val.data2;
                }
                m_vRPN.pop_back();
                bOptimized = true;
              }
              else if (m_vRPN[sz-1].Cmd == cmVAR && m_vRPN[sz-2].Cmd == cmVAR &&
                       m_vRPN[sz-1].Val.ptr == m_vRPN[sz-2].Val.ptr)
              {
                // Optimization: a*a -> a^2
                m_vRPN[sz-2].Cmd = cmVARPOW2;
                m_vRPN.pop_back();
                bOptimized = true;
              }
              break;

        case cmDIV:
              if (m_vRPN[sz-1].Cmd == cmVAL && m_vRPN[sz-2].Cmd == cmVARMUL && m_vRPN[sz-1].Val.data2!=0)
              {
                // Optimization: 4*a/2 -> 2*a
                m_vRPN[sz-2].Val.data  /= m_vRPN[sz-1].Val.data2;
                m_vRPN[sz-2].Val.data2 /= m_vRPN[sz-1].Val.data2;
                m_vRPN.pop_back();
                bOptimized = true;
              }
              break;
              
        } // switch a_Oprt
      }
    }

    // If optimization can't be applied just write the value
    if (!bOptimized)
    {
      --m_iStackPos;
      SToken tok;
      tok.Cmd = a_Oprt;
      m_vRPN.push_back(tok);
    }
  }

  //---------------------------------------------------------------------------
  void QmuParserByteCode::AddIfElse(ECmdCode a_Oprt)
  {
    SToken tok;
    tok.Cmd = a_Oprt;
    m_vRPN.push_back(tok);
  }

  //---------------------------------------------------------------------------
  /** \brief Add an assignement operator
    
      Operator entries in byte code consist of:
      <ul>
        <li>cmASSIGN code</li>
        <li>the pointer of the destination variable</li>
      </ul>

      \sa  ParserToken::ECmdCode
  */
  void QmuParserByteCode::AddAssignOp(qreal *a_pVar)
  {
    --m_iStackPos;

    SToken tok;
    tok.Cmd = cmASSIGN;
    tok.Val.ptr = a_pVar;
    m_vRPN.push_back(tok);
  }

  //---------------------------------------------------------------------------
  /** \brief Add function to bytecode. 

      \param a_iArgc Number of arguments, negative numbers indicate multiarg functions.
      \param a_pFun Pointer to function callback.
  */
  void QmuParserByteCode::AddFun(generic_fun_type a_pFun, int a_iArgc)
  {
    if (a_iArgc>=0)
    {
      m_iStackPos = m_iStackPos - a_iArgc + 1; 
    }
    else
    {
      // function with unlimited number of arguments
      m_iStackPos = m_iStackPos + a_iArgc + 1; 
    }
    m_iMaxStackSize = std::max(m_iMaxStackSize, (size_t)m_iStackPos);

    SToken tok;
    tok.Cmd = cmFUNC;
    tok.Fun.argc = a_iArgc;
    tok.Fun.ptr = a_pFun;
    m_vRPN.push_back(tok);
  }

  //---------------------------------------------------------------------------
  /** \brief Add a bulk function to bytecode. 

      \param a_iArgc Number of arguments, negative numbers indicate multiarg functions.
      \param a_pFun Pointer to function callback.
  */
  void QmuParserByteCode::AddBulkFun(generic_fun_type a_pFun, int a_iArgc)
  {
    m_iStackPos = m_iStackPos - a_iArgc + 1; 
    m_iMaxStackSize = std::max(m_iMaxStackSize, (size_t)m_iStackPos);

    SToken tok;
    tok.Cmd = cmFUNC_BULK;
    tok.Fun.argc = a_iArgc;
    tok.Fun.ptr = a_pFun;
    m_vRPN.push_back(tok);
  }

  //---------------------------------------------------------------------------
  /** \brief Add Strung function entry to the parser bytecode. 
      \throw nothrow

      A string function entry consists of the stack position of the return value,
      followed by a cmSTRFUNC code, the function pointer and an index into the 
      string buffer maintained by the parser.
  */
  void QmuParserByteCode::AddStrFun(generic_fun_type a_pFun, int a_iArgc, int a_iIdx)
  {
    m_iStackPos = m_iStackPos - a_iArgc + 1;

    SToken tok;
    tok.Cmd = cmFUNC_STR;
    tok.Fun.argc = a_iArgc;
    tok.Fun.idx = a_iIdx;
    tok.Fun.ptr = a_pFun;
    m_vRPN.push_back(tok);

    m_iMaxStackSize = std::max(m_iMaxStackSize, (size_t)m_iStackPos);
  }

  //---------------------------------------------------------------------------
  /** \brief Add end marker to bytecode.
      
      \throw nothrow 
  */
  void QmuParserByteCode::Finalize()
  {
    SToken tok;
    tok.Cmd = cmEND;
    m_vRPN.push_back(tok);
    rpn_type(m_vRPN).swap(m_vRPN);     // shrink bytecode vector to fit

    // Determine the if-then-else jump offsets
    QStack<int> stIf, stElse;
    int idx;
    for (int i=0; i<(int)m_vRPN.size(); ++i)
    {
      switch(m_vRPN[i].Cmd)
      {
      case cmIF:
            stIf.push(i);
            break;

      case  cmELSE:
            stElse.push(i);
            idx = stIf.pop();
            m_vRPN[idx].Oprt.offset = i - idx;
            break;
      
      case cmENDIF:
            idx = stElse.pop();
            m_vRPN[idx].Oprt.offset = i - idx;
            break;

      default:
            break;
      }
    }
  }

  //---------------------------------------------------------------------------
  const SToken* QmuParserByteCode::GetBase() const
  {
    if (m_vRPN.size()==0)
      throw QmuParserError(ecINTERNAL_ERROR);
    else
      return &m_vRPN[0];
  }

  //---------------------------------------------------------------------------
  std::size_t QmuParserByteCode::GetMaxStackSize() const
  {
    return m_iMaxStackSize+1;
  }

  //---------------------------------------------------------------------------
  /** \brief Returns the number of entries in the bytecode. */
  std::size_t QmuParserByteCode::GetSize() const
  {
    return m_vRPN.size();
  }

  //---------------------------------------------------------------------------
  /** \brief Delete the bytecode. 
  
      \throw nothrow

      The name of this function is a violation of my own coding guidelines
      but this way it's more in line with the STL functions thus more 
      intuitive.
  */
  void QmuParserByteCode::clear()
  {
    m_vRPN.clear();
    m_iStackPos = 0;
    m_iMaxStackSize = 0;
  }

  //---------------------------------------------------------------------------
  /** \brief Dump bytecode (for debugging only!). */
  void QmuParserByteCode::AsciiDump()
  {
    if (!m_vRPN.size()) 
    {
      mu::console() << "No bytecode available\n";
      return;
    }

    mu::console() << "Number of RPN tokens:" << (int)m_vRPN.size() << "\n";
    for (std::size_t i=0; i<m_vRPN.size() && m_vRPN[i].Cmd!=cmEND; ++i)
    {
      mu::console() << std::dec << i << " : \t";
      switch (m_vRPN[i].Cmd)
      {
      case cmVAL:   mu::console() << "VAL \t";
                    mu::console() << "[" << m_vRPN[i].Val.data2 << "]\n";
                    break;

      case cmVAR:   mu::console() << "VAR \t";
                      mu::console() << "[ADDR: 0x" << std::hex << m_vRPN[i].Val.ptr << "]\n";
                    break;

      case cmVARPOW2: mu::console() << "VARPOW2 \t";
                        mu::console() << "[ADDR: 0x" << std::hex << m_vRPN[i].Val.ptr << "]\n";
                      break;

      case cmVARPOW3: mu::console() << "VARPOW3 \t";
                        mu::console() << "[ADDR: 0x" << std::hex << m_vRPN[i].Val.ptr << "]\n";
                      break;

      case cmVARPOW4: mu::console() << "VARPOW4 \t";
                        mu::console() << "[ADDR: 0x" << std::hex << m_vRPN[i].Val.ptr << "]\n";
                      break;

      case cmVARMUL:  mu::console() << "VARMUL \t";
                        mu::console() << "[ADDR: 0x" << std::hex << m_vRPN[i].Val.ptr << "]";
                      mu::console() << " * [" << m_vRPN[i].Val.data << "]";
                      mu::console() << " + [" << m_vRPN[i].Val.data2 << "]\n";
                      break;

      case cmFUNC:  mu::console() << "CALL\t";
                    mu::console() << "[ARG:" << std::dec << m_vRPN[i].Fun.argc << "]";
                    mu::console() << "[ADDR: 0x" << std::hex << m_vRPN[i].Fun.ptr << "]";
                    mu::console() << "\n";
                    break;

      case cmFUNC_STR:
                    mu::console() << "CALL STRFUNC\t";
                    mu::console() << "[ARG:" << std::dec << m_vRPN[i].Fun.argc << "]";
                    mu::console() << "[IDX:" << std::dec << m_vRPN[i].Fun.idx << "]";
                    mu::console() << "[ADDR: 0x" << m_vRPN[i].Fun.ptr << "]\n";
                    break;

      case cmLT:    mu::console() << "LT\n";  break;
      case cmGT:    mu::console() << "GT\n";  break;
      case cmLE:    mu::console() << "LE\n";  break;
      case cmGE:    mu::console() << "GE\n";  break;
      case cmEQ:    mu::console() << "EQ\n";  break;
      case cmNEQ:   mu::console() << "NEQ\n"; break;
      case cmADD:   mu::console() << "ADD\n"; break;
      case cmLAND:  mu::console() << "&&\n"; break;
      case cmLOR:   mu::console() << "||\n"; break;
      case cmSUB:   mu::console() << "SUB\n"; break;
      case cmMUL:   mu::console() << "MUL\n"; break;
      case cmDIV:   mu::console() << "DIV\n"; break;
      case cmPOW:   mu::console() << "POW\n"; break;

      case cmIF:    mu::console() << "IF\t";
                    mu::console() << "[OFFSET:" << std::dec << m_vRPN[i].Oprt.offset << "]\n";
                    break;

      case cmELSE:  mu::console() << "ELSE\t";
                    mu::console() << "[OFFSET:" << std::dec << m_vRPN[i].Oprt.offset << "]\n";
                    break;

      case cmENDIF: mu::console() << "ENDIF\n"; break;

      case cmASSIGN: 
                    mu::console() << "ASSIGN\t";
                    mu::console() << "[ADDR: 0x" << m_vRPN[i].Oprt.ptr << "]\n";
                    break; 

      default:      mu::console() << "(unknown code: " << m_vRPN[i].Cmd << ")\n";
                    break;
      } // switch cmdCode
    } // while bytecode

    mu::console() << "END" << std::endl;
  }
} // namespace qmu

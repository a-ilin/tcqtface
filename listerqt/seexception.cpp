#include "seexception.h"

#include <common.h>

SeException::SeException(UINT nSeCode, _EXCEPTION_POINTERS* pExcPointers) :
  m_code(nSeCode),
  m_pExcPointers(pExcPointers)
{
}

QString SeException::msg() const
{
  return QString("Structured Exception. Hex code: 0x") + QString::number(m_code, 16);
}



void SeTranslator(UINT nSeCode, _EXCEPTION_POINTERS* pExcPointers)
{
  throw new SeException(nSeCode, pExcPointers);
}

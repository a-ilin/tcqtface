#ifndef SEEXCEPTION_H
#define SEEXCEPTION_H

#include <QString>

typedef unsigned int UINT;
struct _EXCEPTION_POINTERS;

class SeException
{
public:
  SeException(UINT nSeCode, _EXCEPTION_POINTERS* pExcPointers);

  UINT code() const { return m_code; }

  QString msg() const;

private:
  UINT m_code;
  _EXCEPTION_POINTERS* m_pExcPointers;
};

void SeTranslator(UINT nSeCode, _EXCEPTION_POINTERS* pExcPointers);

#endif // SEEXCEPTION_H

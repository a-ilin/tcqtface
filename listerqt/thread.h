#ifndef THREAD_H
#define THREAD_H

#include "common.h"

class Thread
{
public:
  Thread()
    : m_handle(NULL) {}

  ~Thread() { wait(); }


  void start(LPTHREAD_START_ROUTINE pFunc, LPVOID arg = NULL)
  {
    _assert( ! m_handle );
    m_handle = CreateThread(NULL, 0, pFunc, arg, 0, NULL);
    _assert(m_handle);
  }

  DWORD wait(DWORD dwMilliseconds = INFINITE)
  {
    DWORD res = WAIT_OBJECT_0;

    if (m_handle)
    {
      res = WaitForSingleObject(m_handle, dwMilliseconds);
      if (WAIT_OBJECT_0 == res)
      {
        CloseHandle(m_handle);
        m_handle = NULL;
      }
    }

    return res;
  }

private:
  Thread(const Thread&) = delete;
  Thread& operator= (const Thread&) = delete;

private:
  HANDLE m_handle;
};

#endif // THREAD_H

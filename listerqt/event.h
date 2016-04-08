#ifndef EVENT_H
#define EVENT_H

#include  "common.h"

class Event
{
public:
  Event(BOOL bManualReset = FALSE, BOOL bInitialSignaled = FALSE)
    : m_handle(CreateEvent(NULL, bManualReset, bInitialSignaled, NULL))
  {
    _assert(m_handle);
  }

  ~Event()
  {
    CloseHandle(m_handle);
  }

  void set()
  {
    SetEvent(m_handle);
  }

  void reset()
  {
    ResetEvent(m_handle);
  }

  DWORD wait(DWORD dwMilliseconds = INFINITE)
  {
    return WaitForSingleObject(m_handle, dwMilliseconds);
  }

private:
  Event(const Event&) = delete;
  Event& operator= (const Event&) = delete;

private:
  HANDLE m_handle;
};

#endif // EVENT_H

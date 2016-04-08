#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "common.h"

class Semaphore
{
public:
  Semaphore(LONG initial = 1, LONG maximum = 1)
    : m_handle(CreateSemaphore(NULL, initial, maximum, NULL))
  {
    _assert(m_handle);
  }

  ~Semaphore()
  {
    CloseHandle(m_handle);
  }

  DWORD lock(DWORD dwMilliseconds = INFINITE)
  {
    return WaitForSingleObject(m_handle, dwMilliseconds);
  }

  LONG unlock()
  {
    LONG prev = 0;
    BOOL res = ReleaseSemaphore(m_handle, 1, &prev);
    _assert(res);
    return prev;
  }

private:
  Semaphore(const Semaphore&) = delete;
  Semaphore& operator = (const Semaphore&) = delete;

private:
  HANDLE m_handle;
};

class SemaphoreLocker
{
public:
  SemaphoreLocker(Semaphore* sem)
    : m_sem(sem)
  {
    _assert(m_sem);
    m_sem->lock();
  }

  ~SemaphoreLocker()
  {
    m_sem->unlock();
  }

private:
  SemaphoreLocker(const SemaphoreLocker&) = delete;
  SemaphoreLocker& operator = (const SemaphoreLocker&) = delete;

private:
  Semaphore* m_sem;
};

#endif // SEMAPHORE_H

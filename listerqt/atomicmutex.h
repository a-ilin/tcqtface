#ifndef ATOMICMUTEX_H
#define ATOMICMUTEX_H

#include <QAtomicInt>

class AtomicMutex
{
public:
  AtomicMutex()
    : m_data(0) {}

  void lock()
  {
    while ( ! m_data.testAndSetAcquire(0, 1) );
  }

  void unlock()
  {
    m_data.storeRelease(0);
  }

  bool tryLock()
  {
    return m_data.testAndSetAcquire(0, 1);
  }

  bool isLocked()
  {
    return (1 == m_data);
  }

private:
  QAtomicInt m_data;
};

class AtomicLocker
{
public:
  AtomicLocker(AtomicMutex* mutex)
    : m_mutex(mutex)
  {
    m_mutex->lock();
  }

  ~AtomicLocker()
  {
    m_mutex->unlock();
  }

private:
  AtomicMutex* m_mutex;
};

#endif // ATOMICMUTEX_H

#include "atomicmutex.h"

AtomicMutex::AtomicMutex() :
  m_data(0)
{
}

void AtomicMutex::lock()
{
  while ( ! m_data.testAndSetAcquire(0, 1) );
}

void AtomicMutex::unlock()
{
  m_data.storeRelease(0);
}

bool AtomicMutex::tryLock()
{
  return m_data.testAndSetAcquire(0, 1);
}

bool AtomicMutex::isLocked()
{
  return (1 == m_data);
}



AtomicLocker::AtomicLocker(AtomicMutex* mutex) :
  m_mutex(mutex)
{
  m_mutex->lock();
}

AtomicLocker::~AtomicLocker()
{
  m_mutex->unlock();
}

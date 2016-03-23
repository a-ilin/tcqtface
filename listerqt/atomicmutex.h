#ifndef ATOMICMUTEX_H
#define ATOMICMUTEX_H

#include <QAtomicInt>

class AtomicMutex
{
public:
  AtomicMutex();

  void lock();
  void unlock();

  bool tryLock();

  bool isLocked();

private:
  QAtomicInt m_data;
};

class AtomicLocker
{
public:
  AtomicLocker(AtomicMutex* mutex);
  ~AtomicLocker();
private:
  AtomicMutex* m_mutex;
};

#endif // ATOMICMUTEX_H

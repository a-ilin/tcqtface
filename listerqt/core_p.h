#ifndef CORE_P_H
#define CORE_P_H

#include <memory>

#include "core.h"

#include <QEvent>
#include <QObject>
#include <QSet>

// processes Core payloads inside QApplication thread
class CoreAgent : public QObject
{
public:
  CoreAgent() :
    QObject() {}

protected:
  bool event(QEvent *e);
};

struct CoreData
{
  std::unique_ptr<CoreAgent> pAgent;
  HANDLE hSemApp;   // sync app create/destroy
  QSet<AtomicMutex*> pendingMutexes; // pending mutexes
  int iRecursionLevel;
  int iWinCount; // counter of active windows
};

class RecursionHolder
{
public:
  RecursionHolder(CoreData* p)
    : m_p(p) { ++m_p->iRecursionLevel; }
  ~RecursionHolder() { --m_p->iRecursionLevel; }
private:
  CoreData* m_p;
};

extern QEvent::Type EventCoreType;
class CoreEvent : public QEvent
{
public:
  CoreEvent(CorePayload* payload)
    : QEvent(EventCoreType)
    , m_pMutex(NULL)
    , m_payload(payload) {}

  void setMutex(AtomicMutex* pMutex) { m_pMutex = pMutex; }
  AtomicMutex* mutex() const { return m_pMutex; }

  CorePayload* payload() const { return m_payload; }

private:
  AtomicMutex* m_pMutex;
  CorePayload* m_payload;
};

#endif // CORE_P_H

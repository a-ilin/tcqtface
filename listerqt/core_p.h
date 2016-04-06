#ifndef CORE_P_H
#define CORE_P_H

#include <memory>

#include "core.h"

#include <QEvent>
#include <QObject>

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
  QAtomicInt iWinCount; // counter of active windows
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

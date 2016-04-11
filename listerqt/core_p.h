#ifndef CORE_P_H
#define CORE_P_H

#include <memory>

#include "core.h"
#include "event.h"
#include "semaphore.h"
#include "thread.h"

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
  // sync app create/destroy
  Event appStartEvent;
  // application thread
  Thread appThread;
  // counter of active windows
  QAtomicInt winCount;
};

extern QEvent::Type EventCoreType;
class CoreEvent : public QEvent
{
public:
  CoreEvent(CorePayload* payload)
    : QEvent(EventCoreType)
    , m_pEvent(NULL)
    , m_payload(payload) {}

  void setEvent(Event* pEvent) { m_pEvent = pEvent; }
  Event* event() const { return m_pEvent; }

  CorePayload* payload() const { return m_payload; }

private:
  Event* m_pEvent;
  CorePayload* m_payload;
};

#endif // CORE_P_H

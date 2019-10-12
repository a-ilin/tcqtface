/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 by Aleksei Ilin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef CORE_P_H
#define CORE_P_H

#include <memory>

#include "core.h"
#include "event.h"
#include "semaphore.h"
#include "thread.h"
#include "wlxcore.h"

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
  std::unique_ptr<IWlxCore> pWlxCore;
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

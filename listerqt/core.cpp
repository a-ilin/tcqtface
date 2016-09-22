/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 by Aleksei Ilin
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

#include "core.h"
#include "core_p.h"

#include "application.h"
#include "atomicmutex.h"
#include "common.h"
#include "libraryloader.h"
#include "seexception.h"

#include <QCoreApplication>
#include <QPixmapCache>
#include <QThread>

// global objects
static AtomicMutex g_coreMutex;
static std::shared_ptr<Core> g_pCore;

QEvent::Type EventCoreType = (QEvent::Type)QEvent::registerEventType();

static DWORD WINAPI qtAppProc(CONST LPVOID lpParam)
{
  _set_se_translator(SeTranslator);

  CoreData* d = reinterpret_cast<CoreData*>(lpParam);
  DWORD code = 0;

  {
    Application app;
    _log(QString("qApp thread id: %1").arg(GetCurrentThreadId()));

    _assert( ! d->pAgent );
    d->pAgent.reset(new CoreAgent());

    d->appStartEvent.set();

    _log("Enter qApp EventLoop");
    code = app.exec();
    _log("Leave qApp EventLoop");

    // TC can keep plugins loaded so need to clean Qt caches
    QPixmapCache::clear();

    // deinitialize
    d->pAgent.reset();
  }

  _log(QString("Result code: %1").arg(code));
  return code;
}

CoreEvent* CorePayload::createEvent()
{
  return new CoreEvent(this);
}

void Core::processPayload_helper(CoreEvent* event)
{
  _log(QString("Thread ID: 0x%1").arg(QString::number((UINT)GetCurrentThreadId(), 16)));
  _log("Sending payload");
  _assert(event);

  Event e(FALSE, TRUE);
  event->setEvent(&e);

  _assert(qApp);
  if (qApp)
  {
    e.reset();
    _assert(d->pAgent);
    QCoreApplication::postEvent(d->pAgent.get(), event);
  }

  // process incoming messages during wait
  while ( e.wait(3) == WAIT_TIMEOUT )
  {
    dispatchMessages();
  }

  _log("Payload is processed");
}

void Core::dispatchMessages()
{
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

Core::Core()
  : d(new CoreData)
{
  _assert(g_coreMutex.isLocked());
  _assert( ! g_pCore );
  g_pCore.reset(this, [](Core* pCore)
  {
    delete pCore;
  });

  d->winCount = 0;

  d->pWlxCore.reset(new WlxCore());

#ifndef STATIC_BUILD
  // enable loading Qt plugins
  QCoreApplication::setLibraryPaths(QStringList() << Loader::dirByPath(Loader::pathThis()));
#endif

  _log("Core created");
}

Core::~Core()
{
  _assert(g_coreMutex.isLocked());
  stopApplication();

#ifndef STATIC_BUILD
  QCoreApplication::setLibraryPaths(QStringList());
#endif

  _assert( ! d->pAgent );
  _log("Core destroyed");
}

std::shared_ptr<Core> Core::i()
{
  AtomicLocker locker(&g_coreMutex);
  if ( ! g_pCore )
  {
    new Core();
  }

  _assert(g_pCore);
  return g_pCore;
}

bool Core::isExists()
{
  _assert(g_coreMutex.isLocked());
  return static_cast<bool>(g_pCore);
}

bool Core::destroy()
{
  _assert(g_coreMutex.isLocked());
  _assert(g_pCore);

  if ((g_pCore.use_count() == 1) && !g_pCore->winCounter())
  {
    g_pCore.reset();
    return true;
  }

  return false;
}

void Core::lock()
{
  g_coreMutex.lock();
}

void Core::unlock()
{
  _assert(g_coreMutex.isLocked());
  g_coreMutex.unlock();
}

void Core::processPayload(CorePayload& payload, bool processEvents)
{
  if (startApplication() && payload.preprocess())
  {
    // main event
    processPayload_helper(payload.createEvent());

    // process pending events
    if (processEvents)
    {
      auto eventPoster = createCorePayload([]()
      {
        QCoreApplication::processEvents();
        QCoreApplication::sendPostedEvents();
      });

      processPayload_helper(eventPoster.createEvent());
    }

    payload.postprocess();
  }
}

bool Core::startApplication()
{
  CoreLocker locker;

  // make Qt mark TOTALCMD thread as QCoreApplicationPrivate::theMainThread,
  // QApplication will post warnings to the console but at least there will be no crash
  // at plugin unloading, and no leaking threads
  QObject obj;

  if (qApp)
  {
    return true;
  }

  _log("Starting qApp...");

  d->appThread.start(&qtAppProc, d.get());

  // wait until qApp will be initialized
  d->appStartEvent.wait();

  _assert(qApp);
  _log(qApp ? "qApp is succesfully started" : "qApp was not started");

  return qApp != NULL;
}

void Core::stopApplication()
{
  _assert(g_coreMutex.isLocked());

  if ( ! qApp )
  {
    return;
  }

  _assert( ! d->winCount );
  _log("Stopping qApp...");

  // stop the app
  auto payloadStop = createCorePayload([&]()
  {
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
  });
  processPayload_helper(payloadStop.createEvent());

  while (d->appThread.wait(3) == WAIT_TIMEOUT)
  {
    dispatchMessages();
  }

  // FIXME: there is a racing condition here with qt_adopted_thread_watcher

  _log("qApp is stopped");
}

void Core::increaseWinCounter()
{
  ++d->winCount;
  _log(QString("Total window count: ") + QString::number(d->winCount));
}

void Core::decreaseWinCounter()
{
  --d->winCount;
  _log(QString("Total window count: ") + QString::number(d->winCount));
}

int Core::winCounter() const
{
  _assert(g_coreMutex.isLocked());
  return d->winCount;
}

IWlxCore* Core::wlxCore() const
{
  return d->pWlxCore.get();
}

bool CoreAgent::event(QEvent* e)
{
  _assert(QThread::currentThread() == qApp->thread());

  if ( e->type() == EventCoreType )
  {
    CoreEvent* ce = static_cast<CoreEvent*>(e);

    try
    {
      ce->payload()->process();
    }
    catch(...)
    {
      ce->event()->set();
      throw;
    }

    ce->accept();
    ce->event()->set();
    return true;
  }

  return QObject::event(e);
}

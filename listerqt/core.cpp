#include "core.h"
#include "core_p.h"

#include "application.h"
#include "atomicmutex.h"
#include "common.h"
#include "libraryloader.h"
#include "seexception.h"

#include <QCoreApplication>
#include <QThread>

// global objects
AtomicMutex g_coreMutex;
std::shared_ptr<Core> g_pCore;

QEvent::Type EventCoreType          = (QEvent::Type)QEvent::registerEventType();
QEvent::Type EventPostEventsType    = (QEvent::Type)QEvent::registerEventType();

CoreEvent* CorePayload::createEvent()
{
  return new CoreEvent(this);
}

void Core::processPayload_helper(CoreEvent* event)
{
  _log(QString("Thread ID: 0x%1").arg(QString::number((UINT)GetCurrentThreadId(), 16)));
  _log("Sending payload");

  AtomicMutex mutex;
  _assert(event);
  event->setMutex(&mutex);

  _assert(qApp);
  if (qApp)
  {
    mutex.lock();

    _assert(d->pAgent);
    QCoreApplication::postEvent(d->pAgent.get(), event);
  }

  // process incoming messages during wait
  while ( ! mutex.tryLock() )
  {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Sleep(3);
  }
  _log("Payload is processed");
}

DWORD WINAPI qtAppProc(CONST LPVOID lpParam)
{
  _set_se_translator(SeTranslator);

  CoreData* d = reinterpret_cast<CoreData*>(lpParam);
  DWORD code = 0;

  {
    Application app;
    _log(QString("qApp thread id: %1").arg(GetCurrentThreadId()));

    _assert( ! d->pAgent );
    d->pAgent.reset(new CoreAgent());

    UnlockSemaphore(d->hSemApp);

    _log("Enter qApp EventLoop");
    code = app.exec();
    _log("Leave qApp EventLoop");

    // deinitialize
    LockSemaphore(d->hSemApp);

    d->pAgent.reset();
  }

  _log(QString("Result code: %1").arg(code));

  UnlockSemaphore(d->hSemApp);

  return code;
}

void Core::dispatchMessages(HANDLE hSem)
{ // process incoming messages during wait
  while (LockSemaphoreEx(hSem, 3) == WAIT_TIMEOUT)
  {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
  }

  UnlockSemaphore(hSem);
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

  d->iWinCount = 0;
  d->hSemApp = CreateSemaphore(NULL, 1, 1, NULL);
  _assert(d->hSemApp);

  // enable loading Qt plugins
  QCoreApplication::setLibraryPaths(QStringList() << Loader::dirByPath(Loader::pathThis()));

  _log("Core created");
}

Core::~Core()
{
  _assert(g_coreMutex.isLocked());
  stopApplication();

  CloseHandle(d->hSemApp);

  QCoreApplication::setLibraryPaths(QStringList());

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

void Core::processPayload(CorePayload& payload)
{
  if (startApplication() && payload.preprocess())
  {
    // main event
    processPayload_helper(payload.createEvent());

    // process pending events
    auto eventPoster = createCorePayload([&]()
    {
      QCoreApplication::processEvents();
      QCoreApplication::sendPostedEvents();
    });

    processPayload_helper(eventPoster.createEvent());

    payload.postprocess();
  }
}

bool Core::startApplication()
{
  CoreLocker locker;

  if (qApp)
  {
    return true;
  }

  _log("Starting qApp...");

  LockSemaphore(d->hSemApp);

  // App thread
  HANDLE hQtThread = CreateThread(NULL, 0, &qtAppProc, d.get(), 0, NULL);
  _assert(hQtThread);
  if ( hQtThread )
  {
    CloseHandle(hQtThread);
    // wait until qApp will be initialized
    WaitForSemaphore(d->hSemApp);
  }

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

  _assert( ! d->iWinCount );
  _log("Stopping qApp...");

  QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);

  while (qApp)
  {
    dispatchMessages(d->hSemApp);
    Sleep(3);
  }

  // be sure that qApp thread is finished
  dispatchMessages(d->hSemApp);

  _log("qApp is stopped");
}

void Core::increaseWinCounter()
{
  ++d->iWinCount;
  _log(QString("Total window count: ") + QString::number(d->iWinCount));
}

void Core::decreaseWinCounter()
{
  --d->iWinCount;
  _log(QString("Total window count: ") + QString::number(d->iWinCount));
}

int Core::winCounter() const
{
  _assert(g_coreMutex.isLocked());
  return d->iWinCount;
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
      ce->mutex()->unlock();
      throw;
    }

    ce->accept();
    ce->mutex()->unlock();
    return true;
  }

  return QObject::event(e);
}

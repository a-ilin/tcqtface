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
std::unique_ptr<Core> g_pCore;

QEvent::Type EventCoreType          = (QEvent::Type)QEvent::registerEventType();
QEvent::Type EventPostEventsType    = (QEvent::Type)QEvent::registerEventType();

static void EmergencyUnlock(CoreData* d)
{
  GlobalError = true;
  for (AtomicMutex* m : d->pendingMutexes)
  {
    m->unlock();
  }
  d->pendingMutexes.clear();
}

CoreEvent* CorePayload::createEvent()
{
  return new CoreEvent(this);
}

void Core::processPayload_helper(CoreEvent* event)
{
  _assert(d->iRecursionLevel > 0);

  _log(QString("Thread ID: 0x%1").arg(QString::number((UINT)GetCurrentThreadId(), 16)));

  AtomicMutex mutex;
  d->pendingMutexes.insert(&mutex);

  bool exc = false;
  QString errMsg("Main (TC) Thread: ");

  try
  {
    _assert(d->pAgent);
    _assert(event);
    _assert(qApp);

    event->setMutex(&mutex);

    if (qApp)
    {
      mutex.lock();
      QCoreApplication::postEvent(d->pAgent.get(), event);
    }
  }
  catch(SeException* ex)
  {
    errMsg += ex->msg();
    exc = true;
  }
  catch(...)
  {
    errMsg += QString("Unknown C++ Exception");
    exc = true;
  }

  if ( exc )
  { // an exception occured
    EmergencyUnlock(d.get());
    _assert_ex(false, errMsg);
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

    Sleep(10);
  }

  d->pendingMutexes.remove(&mutex);
}

DWORD WINAPI InitializeQtAppProc(CONST LPVOID lpParam)
{
  CoreData* d = reinterpret_cast<CoreData*>(lpParam);
  int code = 0;

  {
    // initialize
    Application app;
    _assert(qApp);
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

  _assert( ! qApp );
  _log(QString("Result code: %1").arg(code));

  UnlockSemaphore(d->hSemApp);

  return code;
}

DWORD WINAPI ThreadWrapper(CONST LPVOID lpParam)
{
  CoreData* d = reinterpret_cast<CoreData*>(lpParam);
  int code = 0;

  _set_se_translator(SeTranslator);

  bool exc = false;
  QString errMsg("QApplication Thread: ");

  try
  {
    code = InitializeQtAppProc(lpParam);
  }
  catch(SeException* ex)
  {
    errMsg += ex->msg();
    exc = true;
  }
  catch(...)
  {
    errMsg += QString("Unknown C++ Exception");
    exc = true;
  }

  if (exc)
  {
    EmergencyUnlock(d);

    _assert_ex(false, errMsg);

    UnlockSemaphore(d->hSemApp);
    ExitThread(-1);
  }

  ExitThread(code);
  return code;
}

void Core::dispatchMessages(HANDLE hSem)
{ // process incoming messages during wait
  while (LockSemaphoreEx(hSem, 10) == WAIT_TIMEOUT)
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
  d->iWinCount = 0;
  d->iRecursionLevel = 0;
  d->hSemApp = CreateSemaphore(NULL, 1, 1, NULL);
  _assert(d->hSemApp);

  // enable loading Qt plugins
  QCoreApplication::setLibraryPaths(QStringList() << LibraryLoader::dirByPath(LibraryLoader::pathThis()));

  _assert( ! g_pCore );
  g_pCore.reset(this);

  _log("Core created");
}

Core::~Core()
{
  g_pCore.release();

  stopApplication();

  CloseHandle(d->hSemApp);

  QCoreApplication::setLibraryPaths(QStringList());

  _assert( ! d->pAgent );
  _log("Core destroyed");
}

Core& Core::i()
{
  if ( ! g_pCore )
  {
    new Core();
  }

  return *g_pCore.get();
}

bool Core::isExists()
{
  return static_cast<bool>(g_pCore);
}

void Core::processPayload(CorePayload& payload)
{
  RecursionHolder rholder(d.get());

  if ( payload.preprocess() )
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
  }

  payload.postprocess();
}

bool Core::startApplication()
{
  if (qApp)
  {
    return true;
  }

  _log("Starting qApp...");

  LockSemaphore(d->hSemApp);

  // App thread
  HANDLE hQtThread = CreateThread(NULL, 0, &ThreadWrapper, d.get(), 0, NULL);
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
    Sleep(10);
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

bool Core::isUnusable() const
{
  return  ! ( d->iRecursionLevel || d->iWinCount );
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

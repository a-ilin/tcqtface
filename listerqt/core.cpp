#include "core.h"
#include "core_p.h"

#include "application.h"
#include "atomicmutex.h"
#include "listplug_qt_iface.h"
#include "seexception.h"
#include "tcmdparentwindow.h"

#include <QCoreApplication>
#include <QThread>

QEvent::Type EventWindowCreateType  = (QEvent::Type)QEvent::registerEventType();
QEvent::Type EventWindowDestroyType = (QEvent::Type)QEvent::registerEventType();
QEvent::Type EventLogThreadIdType   = (QEvent::Type)QEvent::registerEventType();
QEvent::Type EventPostEventsType    = (QEvent::Type)QEvent::registerEventType();

// custom window commands
QEvent::Type EventCustomLoadFileType     = (QEvent::Type)QEvent::registerEventType();
QEvent::Type EventCustomSearchDialogType = (QEvent::Type)QEvent::registerEventType();
QEvent::Type EventCustomPrintType        = (QEvent::Type)QEvent::registerEventType();
QEvent::Type EventCustomSendCommandType  = (QEvent::Type)QEvent::registerEventType();
QEvent::Type EventCustomSearchTextType   = (QEvent::Type)QEvent::registerEventType();

static void SetupGlobalError(CorePrivate* d)
{
  GlobalError = true;
  if (d)
  {
    foreach (AtomicMutex* m, d->m_pendingMutexes)
    {
      m->unlock();
    }
    d->m_pendingMutexes.clear();
  }
}


CorePrivate::CorePrivate() :
  m_iRecursionLevel(0)
{
  m_hSemApp   = CreateSemaphore(NULL, 1, 1, NULL);
  _assert(m_hSemApp);
}

CorePrivate::~CorePrivate()
{
  if (qApp)
  {
    shutdownApp();
  }

  CloseHandle(m_hSemApp);

  _assert( ! m_pWinManager );
}

void CorePrivate::processEvent_helper(Event* event)
{
  _assert(m_iRecursionLevel > 0);

  _log(QString("Thread ID: 0x%1").arg(QString::number((UINT)GetCurrentThreadId(), 16)));

  AtomicMutex mutex;
  m_pendingMutexes.insert(&mutex);

  bool exc = false;
  QString errMsg("Main (TC) Thread: ");

  try
  {
    _assert(m_pWinManager);
    _assert(event);
    _assert(qApp);

    event->pMutex = &mutex;

    if (qApp)
    {
      mutex.lock();
      QCoreApplication::postEvent(m_pWinManager.data(), event);
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
    SetupGlobalError(this);
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

  m_pendingMutexes.remove(&mutex);
}

void CorePrivate::processEvent(Event* event)
{
  // main event
  processEvent_helper(event);
  // process pending events
  processEvent_helper(new EventPostEvents());
}

DWORD WINAPI InitializeQtAppProc(CONST LPVOID lpParam)
{
  CorePrivate* d = reinterpret_cast<CorePrivate*>(lpParam);
  int code = 0;

  {
    // initialize
    Application app;
    _assert(qApp);
    _log(QString("qApp thread id: %1").arg(GetCurrentThreadId()));

    _assert( ! d->m_pWinManager );
    d->m_pWinManager.reset(new WinManager());

    UnlockSemaphore(d->m_hSemApp);

    _log("Enter qApp EventLoop");
    code = app.exec();
    _log("Leave qApp EventLoop");

    // deinitialize
    LockSemaphore(d->m_hSemApp);

    d->m_pWinManager.reset();
  }

  _assert( ! qApp );
  _log(QString("Result code: %1").arg(code));

  UnlockSemaphore(d->m_hSemApp);

  return code;
}

DWORD WINAPI ThreadWrapper(CONST LPVOID lpParam)
{
  CorePrivate* d = reinterpret_cast<CorePrivate*>(lpParam);
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
    SetupGlobalError(d);

    _assert_ex(false, errMsg);

    UnlockSemaphore(d->m_hSemApp);
    ExitThread(-1);
  }

  ExitThread(code);
  return code;
}

void CorePrivate::startupApp()
{
  if (qApp)
  {
    return;
  }

  _log("qApp is not yet initialized. Creating...");

  LockSemaphore(m_hSemApp);

  // App thread
  HANDLE hQtThread = CreateThread(NULL, 0, &ThreadWrapper, this, 0, NULL);
  _assert(hQtThread);
  if ( ! hQtThread )
  {
    return;
  }

  CloseHandle(hQtThread);
  // wait until qApp will be initialized
  WaitForSemaphore(m_hSemApp);

  _assert(qApp);
  _log("qApp is created.");
}

void CorePrivate::shutdownApp()
{
  _assert(qApp);
  _assert( ! m_winSet.size() );

  _log("Start qApp shutdown.");

  // shutdown qApp
  QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);

  while (qApp)
  {
    dispatchMessages(m_hSemApp);
    Sleep(10);
  }

  // be sure that qApp thread is finished
  dispatchMessages(m_hSemApp);

  _log("qApp shutted down");
}

HWND CorePrivate::createWindow(const InterfaceKeeper& keeper,
                               HWND hParentWin,
                               const QString& sFilePath,
                               int iShowFlags)
{
  ListPlugQtIface* iface = keeper.iface();
  _assert(iface);
  if ( ! iface )
  {
    return NULL;
  }

  if ( ! iface->isFileAcceptable(sFilePath) )
  {
    _log(QString("File not acceptable: ") + sFilePath);
    return NULL;
  }

  // initialize QApplication if needed
  startupApp();
  if ( ! qApp )
  {
    return NULL;
  }

  HWND hChildWin = NULL;
  TCmdParentWindow* pChildWin = NULL;

  processEvent(new EventWindowCreate(keeper, hParentWin, sFilePath,
                                     iShowFlags, &hChildWin, &pChildWin));

  if (hChildWin)
  {
    _assert(pChildWin);
    _assert( ! m_winSet.contains(pChildWin) );
    m_winSet.insert(pChildWin);
  }

  _log(QString("Total window count: ") + QString::number(m_winSet.size()));
  return hChildWin;
}

void CorePrivate::destroyWindow(const InterfaceKeeper& keeper, HWND hWin)
{
  TCmdParentWindow* pChildwin = NULL;
  processEvent(new EventWindowDestroy(keeper, hWin, &pChildwin));

  _assert(pChildwin);
  m_winSet.remove(pChildwin);

  _log(QString("Total window count: ") + QString::number(m_winSet.size()));
}

int CorePrivate::customWindowCommand(EventWindowCustomCommand* event)
{
  int resultCode;
  event->pResultCode = &resultCode;
  processEvent(event);
  return resultCode;
}

void CorePrivate::dispatchMessages(HANDLE hSem)
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

void CorePrivate::tryDtor()
{
  _assert(Core::isExists());

  if ( ( ! m_iRecursionLevel ) && // all recursion calls are finished
       ( ! m_winSet.size() ) )    // there aren't active windows
  { // destruct core
    delete &Core::i();
  }
}


// global objects
QScopedPointer<Core> g_pCore;
AtomicMutex g_coreMutex;

Core::Core()
{
  _assert(g_coreMutex.isLocked());

  _assert( ! g_pCore );
  g_pCore.reset(this);

  d_ptr = new CorePrivate();

  _log("Core created");
}

Core::~Core()
{
  AtomicLocker locker(&g_coreMutex);
  g_pCore.take();

  delete d_ptr;

  _log("Core destroyed");
}

Core& Core::i()
{
  AtomicLocker locker(&g_coreMutex);

  if ( ! g_pCore )
  {
    new Core();
  }

  return *g_pCore.data();
}

bool Core::isExists()
{
  AtomicLocker locker(&g_coreMutex);
  return ! g_pCore.isNull();
}

HWND Core::createWindow(const InterfaceKeeper& keeper, HWND hParentWin,
                        const QString& sFilePath, int iShowFlags)
{
  Q_D(Core);
  HWND hWin = d->callRecursively<HWND>(&CorePrivate::createWindow,
                                        keeper, hParentWin, sFilePath, iShowFlags);

  d->tryDtor();

  return hWin;
}

int Core::loadFile(const InterfaceKeeper& keeper, HWND hParentWin, HWND hChildWin,
                   const QString& sFilePath, int iShowFlags)
{
  Q_UNUSED(hParentWin);
  Q_D(Core);
  return d->callRecursively<int>(&CorePrivate::customWindowCommand,
                                 new EventCustomLoadFile(keeper, hChildWin,
                                                         sFilePath, iShowFlags));
}

void Core::destroyWindow(const InterfaceKeeper& keeper, HWND hWin)
{
  Q_D(Core);
  d->callRecursively(&CorePrivate::destroyWindow, keeper, hWin);

  d->tryDtor();
}

int Core::searchDialog(const InterfaceKeeper& keeper, HWND hWin, int iFindNext)
{
  Q_D(Core);
  return d->callRecursively<int>(&CorePrivate::customWindowCommand,
                                 new EventCustomSearchDialog(keeper, hWin, iFindNext));
}

int Core::print(const InterfaceKeeper& keeper, HWND hWin,
                const QString& sFileToPrint, const QString& sDefPrinter,
                int iPrintFlags, RECT* pMargins)
{
  Q_D(Core);
  QMargins margins(pMargins->left, pMargins->top, pMargins->right, pMargins->bottom);
  return d->callRecursively<int>(&CorePrivate::customWindowCommand,
                                 new EventCustomPrint(keeper, hWin, sFileToPrint, sDefPrinter,
                                                      iPrintFlags, margins));
}

int Core::sendCommand(const InterfaceKeeper& keeper, HWND hWin, int iCommand, int iParameter)
{
  Q_D(Core);
  return d->callRecursively<int>(&CorePrivate::customWindowCommand,
                                 new EventCustomSendCommand(keeper, hWin, iCommand, iParameter));
}

int Core::searchText(const InterfaceKeeper& keeper, HWND hWin,
                     const QString& sSearchString, int iSearchParameter)
{
  Q_D(Core);
  return d->callRecursively<int>(&CorePrivate::customWindowCommand,
                                 new EventCustomSearchText(keeper, hWin,
                                                           sSearchString, iSearchParameter));
}

bool WinManager::event(QEvent* e)
{
  _assert(QThread::currentThread() == qApp->thread());

  bool isManagedEvent = false;

  try
  {
    isManagedEvent = eventWrapper(e);
  }
  catch(...)
  { // unlock mutex
    Event* ewm = static_cast<Event*> (e);
    ewm->pMutex->unlock();
    throw;
  }

  if (isManagedEvent)
  {
    e->accept();
    Event* ewm = static_cast<Event*> (e);
    ewm->pMutex->unlock();
    return true;
  }

  return QObject::event(e);
}

bool WinManager::eventWrapper(QEvent* e)
{
  if ( e->type() == EventWindowCreateType )
  {
    createEvent(e);
    return true;
  }
  else if (e->type() == EventWindowDestroyType)
  {
    destroyEvent(e);
    return true;
  }
  else if (e->type() == EventPostEventsType)
  {
    postEvents(e);
    return true;
  }
  // custom commands
  else if (e->type() == EventCustomLoadFileType)
  {
    customWindowEvent(e, &loadFile);
    return true;
  }
  else if (e->type() == EventCustomSearchDialogType)
  {
    customWindowEvent(e, &searchDialog);
    return true;
  }
  else if (e->type() == EventCustomPrintType)
  {
    customWindowEvent(e, &print);
    return true;
  }
  else if (e->type() == EventCustomSendCommandType)
  {
    customWindowEvent(e, &sendCommand);
    return true;
  }
  else if (e->type() == EventCustomSearchTextType)
  {
    customWindowEvent(e, &searchText);
    return true;
  }

  return false;
}

void WinManager::createEvent(QEvent* e)
{
  _assert(e->type() == EventWindowCreateType);
  EventWindowCreate* ewm = static_cast<EventWindowCreate*> (e);

  InterfaceKeeper& keeper = ewm->keeper;
  ListPlugQtIface* iface = keeper.iface();
  _assert(iface);
  if ( ! iface )
  {
    return;
  }

  // create new window
  TCmdParentWindow* pTcmdParentWnd = new TCmdParentWindow(keeper, (WId)ewm->hParentWin);
  TCmdChildWindow* pTcmdChildWnd = iface->createChildWindow(pTcmdParentWnd);
  _assert(pTcmdChildWnd && pTcmdChildWnd->widget());

  if (pTcmdChildWnd &&
      pTcmdChildWnd->widget() &&
      (pTcmdChildWnd->loadFile(ewm->sFilePath, ewm->iShowFlags) == LISTPLUGIN_OK))
  {
    *(ewm->hChildWin) = (HWND)pTcmdParentWnd->winId();
    *(ewm->pChildwin) = pTcmdParentWnd;
    pTcmdParentWnd->setChildWindow(pTcmdChildWnd);
    pTcmdParentWnd->show();

    _log(QString("Window created. Parent: 0x") + QString::number((quint64)pTcmdParentWnd, 16)
         + QString(", HWND: 0x") + QString::number((quint64)pTcmdParentWnd->winId(), 16));
  }
  else
  {
    _log("Window was NOT created!");

    delete pTcmdChildWnd;
    delete pTcmdParentWnd;
  }
}

void WinManager::destroyEvent(QEvent* e)
{
  _assert(e->type() == EventWindowDestroyType);
  EventWindowDestroy* ewm = static_cast<EventWindowDestroy*> (e);

  _assert(ewm->hDestroyWin != NULL);
  if (ewm->hDestroyWin != NULL)
  { // destroy existing window synchronously
    TCmdParentWindow* pTcmdParentWnd = TCmdParentWindow::getByHandle(ewm->hDestroyWin);
    _assert(pTcmdParentWnd);
    if (pTcmdParentWnd)
    {
      _log(QString("Window destroyed. Parent: 0x") + QString::number((quint64)pTcmdParentWnd, 16)
           + QString(", HWND: 0x") + QString::number((quint64)pTcmdParentWnd->winId(), 16));

      pTcmdParentWnd->close();

      *(ewm->pChildwin) = pTcmdParentWnd;
    }
  }
}

void WinManager::postEvents(QEvent* e)
{
  _assert(e->type() == EventPostEventsType);
  QCoreApplication::processEvents();
  QCoreApplication::sendPostedEvents();
}

void WinManager::customWindowEvent(QEvent* e, int(*f)(QEvent*, TCmdParentWindow*))
{
  EventWindowCustomCommand* ewm = static_cast<EventWindowCustomCommand*> (e);

  _assert(ewm->hWin != NULL);
  if (ewm->hWin != NULL)
  {
    TCmdParentWindow* pTcmdParentWnd = TCmdParentWindow::getByHandle(ewm->hWin);
    _assert(pTcmdParentWnd);
    if (pTcmdParentWnd)
    {
      *ewm->pResultCode = (*f)(e, pTcmdParentWnd);
    }
  }
}

int WinManager::loadFile(QEvent* e, TCmdParentWindow* parent)
{
  _assert(e->type() == EventCustomLoadFileType);
  EventCustomLoadFile* ewm = static_cast<EventCustomLoadFile*> (e);
  return parent->childWindow()->loadFile(ewm->sFilePath, ewm->iShowFlags);
}

int WinManager::searchDialog(QEvent* e, TCmdParentWindow* parent)
{
  _assert(e->type() == EventCustomSearchDialogType);
  EventCustomSearchDialog* ewm = static_cast<EventCustomSearchDialog*> (e);
  return parent->childWindow()->searchDialog(ewm->iFindNext);
}

int WinManager::print(QEvent* e, TCmdParentWindow* parent)
{
  _assert(e->type() == EventCustomPrintType);
  EventCustomPrint* ewm = static_cast<EventCustomPrint*> (e);
  return parent->childWindow()->print(ewm->sFileToPrint, ewm->sDefPrinter,
                                      ewm->iPrintFlags, ewm->mMargins);
}

int WinManager::sendCommand(QEvent* e, TCmdParentWindow* parent)
{
  _assert(e->type() == EventCustomSendCommandType);
  EventCustomSendCommand* ewm = static_cast<EventCustomSendCommand*> (e);
  return parent->childWindow()->sendCommand(ewm->iCommand, ewm->iParameter);
}

int WinManager::searchText(QEvent* e, TCmdParentWindow* parent)
{
  _assert(e->type() == EventCustomSearchTextType);
  EventCustomSearchText* ewm = static_cast<EventCustomSearchText*> (e);
  return parent->childWindow()->searchText(ewm->sSearchString, ewm->iSearchParameter);
}


EventWindowCreate::EventWindowCreate(const InterfaceKeeper& _keeper,
                                     HWND _hParentWin,
                                     const QString& _sFilePath,
                                     int _iShowFlags,
                                     HWND* _hChildWin,
                                     TCmdParentWindow** _pChildWin) :
  Event(EventWindowCreateType),
  keeper(_keeper),
  hParentWin(_hParentWin),
  sFilePath(_sFilePath),
  iShowFlags(_iShowFlags),
  hChildWin(_hChildWin),
  pChildwin(_pChildWin)
{}

EventWindowDestroy::EventWindowDestroy(const InterfaceKeeper& _keeper,
                                       HWND _hDestroyWin,
                                       TCmdParentWindow** _pChildwin) :
  Event(EventWindowDestroyType),
  keeper(_keeper),
  hDestroyWin(_hDestroyWin),
  pChildwin(_pChildwin)
{}


EventPostEvents::EventPostEvents() :
  Event(EventPostEventsType)
{}


EventWindowCustomCommand::EventWindowCustomCommand(QEvent::Type type,
                                                   const InterfaceKeeper& _keeper,
                                                   HWND _hWin) :
  Event(type),
  keeper(_keeper),
  hWin(_hWin),
  pResultCode(NULL)
{}


EventCustomLoadFile::EventCustomLoadFile(const InterfaceKeeper& _keeper,
                                         HWND _hWin,
                                         const QString& _sFilePath,
                                         int _iShowFlags) :
  EventWindowCustomCommand(EventCustomLoadFileType,
                           _keeper,
                           _hWin),
  sFilePath(_sFilePath),
  iShowFlags(_iShowFlags)
{}


EventCustomSearchDialog::EventCustomSearchDialog(const InterfaceKeeper& _keeper,
                                                 HWND _hWin,
                                                 int _iFindNext) :
  EventWindowCustomCommand(EventCustomSearchDialogType,
                           _keeper,
                           _hWin),
  iFindNext(_iFindNext)
{}


EventCustomPrint::EventCustomPrint(const InterfaceKeeper& _keeper,
                                   HWND _hWin,
                                   const QString& _sFileToPrint,
                                   const QString& _sDefPrinter,
                                   int _iPrintFlags,
                                   const QMargins& _mMargins) :
  EventWindowCustomCommand(EventCustomPrintType,
                           _keeper,
                           _hWin),
  sFileToPrint(_sFileToPrint),
  sDefPrinter(_sDefPrinter),
  iPrintFlags(_iPrintFlags),
  mMargins(_mMargins)
{}


EventCustomSendCommand::EventCustomSendCommand(const InterfaceKeeper& _keeper,
                                               HWND _hWin,
                                               int _iCommand,
                                               int _iParameter) :
  EventWindowCustomCommand(EventCustomSendCommandType,
                           _keeper,
                           _hWin),
  iCommand(_iCommand),
  iParameter(_iParameter)
{}


EventCustomSearchText::EventCustomSearchText(const InterfaceKeeper& _keeper,
                                             HWND _hWin,
                                             const QString& _sSearchString,
                                             int _iSearchParameter) :
  EventWindowCustomCommand(EventCustomSearchTextType,
                           _keeper,
                           _hWin),
  sSearchString(_sSearchString),
  iSearchParameter(_iSearchParameter)
{}

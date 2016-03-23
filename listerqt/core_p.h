#ifndef CORE_P_H
#define CORE_P_H

#include "core.h"
#include "common.h"
#include "libraryloader.h"

#include <QEvent>
#include <QMargins>
#include <QObject>
#include <QScopedPointer>
#include <QSet>

class AtomicMutex;
class Event;
class EventWindowCustomCommand;

class QMargins;

class CorePrivate
{
public:
  CorePrivate();
  ~CorePrivate();

  // synchronously process event, returns false on Exception
  void processEvent_helper(Event* event);
  void processEvent(Event* event);
  // creates App if needed
  void startupApp();
  void shutdownApp();

  // window manage
  HWND createWindow(const InterfaceKeeper& keeper,
                    HWND hParentWin,
                    const QString& sFilePath,
                    int iShowFlags);

  void destroyWindow(const InterfaceKeeper& keeper, HWND hWin);

  // send custom window command
  int customWindowCommand(EventWindowCustomCommand* event);

  // dispatches system messages until hSem is available
  static void dispatchMessages(HANDLE hSem);

  // non-recursive callers
  template <typename Ret, typename Func, typename... Args>
  Ret callRecursively(Func func, Args... args);
  template <typename Func, typename... Args>
  void callRecursively(Func func, Args... args);

  // try to self-destruct
  void tryDtor();

public:
  QScopedPointer<QObject> m_pWinManager;
  HANDLE m_hSemApp;   // sync app create/destroy
  QSet<TCmdParentWindow*> m_winSet; // all active windows

  QSet<AtomicMutex*> m_pendingMutexes; // pending mutexes
  int m_iRecursionLevel;
};

template <typename Ret, typename Func, typename... Args>
Ret CorePrivate::callRecursively(Func func, Args... args)
{
  _assert(m_iRecursionLevel >= 0);
  ++m_iRecursionLevel;
  Ret ret = (this->*func)(args...);
  --m_iRecursionLevel;
  _assert(m_iRecursionLevel >= 0);
  return ret;
}

template <typename Func, typename... Args>
void CorePrivate::callRecursively(Func func, Args... args)
{
  _assert(m_iRecursionLevel >= 0);
  ++m_iRecursionLevel;
  (this->*func)(args...);
  --m_iRecursionLevel;
  _assert(m_iRecursionLevel >= 0);
}


extern QEvent::Type EventWindowCreateType;
extern QEvent::Type EventWindowDestroyType;
extern QEvent::Type EventLogThreadIdType;
extern QEvent::Type EventPostEventsType;


// creates windows on request, lives in qApp thread
class WinManager : public QObject
{
public:
  WinManager() :
    QObject() {}

protected:
  bool event(QEvent *e);
  // returns true if managed event
  bool eventWrapper(QEvent *e);

  static void createEvent(QEvent* e);
  static void destroyEvent(QEvent* e);
  static void postEvents(QEvent* e);

  static void customWindowEvent(QEvent* e, int(*f)(QEvent*, TCmdParentWindow*));

  // custom events
  static int loadFile(QEvent* e, TCmdParentWindow* parent);
  static int searchDialog(QEvent* e, TCmdParentWindow* parent);
  static int print(QEvent* e, TCmdParentWindow* parent);
  static int sendCommand(QEvent* e, TCmdParentWindow* parent);
  static int searchText(QEvent* e, TCmdParentWindow* parent);
};

class Event : public QEvent
{
public:
  Event(QEvent::Type type):
    QEvent(type) {}

  AtomicMutex* pMutex;
};

// Qt Event used to create windows
class EventWindowCreate : public Event
{
public:
  EventWindowCreate(const InterfaceKeeper& _keeper,
                    HWND _hParentWin,
                    const QString& _sFilePath,
                    int _iShowFlags,
                    HWND* _hChildWin,
                    TCmdParentWindow** _pChildWin);

  InterfaceKeeper keeper;
  HWND hParentWin;
  QString sFilePath;
  int iShowFlags;
  HWND* hChildWin;
  TCmdParentWindow** pChildwin;
};

// Qt Event used to create windows
class EventWindowDestroy : public Event
{
public:
  EventWindowDestroy(const InterfaceKeeper& _keeper,
                     HWND _hDestroyWin,
                     TCmdParentWindow** _pChildwin);

  InterfaceKeeper keeper;
  HWND hDestroyWin;
  TCmdParentWindow** pChildwin;
};

// Qt Event used to process main QEventLoop
class EventPostEvents : public Event
{
public:
  EventPostEvents();
};

// Qt Event used to deliver custom window commands
class EventWindowCustomCommand : public Event
{
public:
  EventWindowCustomCommand(QEvent::Type type,
                           const InterfaceKeeper& _keeper,
                           HWND _hWin);

  InterfaceKeeper keeper;
  HWND hWin;

  int* pResultCode;
};

// Qt Event used to load file
class EventCustomLoadFile : public EventWindowCustomCommand
{
public:
  EventCustomLoadFile(const InterfaceKeeper& _keeper,
                      HWND _hWin,
                      const QString& _sFilePath,
                      int _iShowFlags);
  QString sFilePath;
  int iShowFlags;
};

// Qt Event used to open search dialog
class EventCustomSearchDialog : public EventWindowCustomCommand
{
public:
  EventCustomSearchDialog(const InterfaceKeeper& _keeper,
                          HWND _hWin,
                          int _iFindNext);
  int iFindNext;
};

// Qt Event used to print
class EventCustomPrint : public EventWindowCustomCommand
{
public:
  EventCustomPrint(const InterfaceKeeper& _keeper,
                   HWND _hWin,
                   const QString& _sFileToPrint,
                   const QString& _sDefPrinter,
                   int _iPrintFlags,
                   const QMargins& _mMargins);

  QString sFileToPrint;
  QString sDefPrinter;
  int iPrintFlags;
  QMargins mMargins;
};

// Qt Event used to send command to window
class EventCustomSendCommand : public EventWindowCustomCommand
{
public:
  EventCustomSendCommand(const InterfaceKeeper& _keeper,
                         HWND _hWin,
                         int _iCommand,
                         int _iParameter);

  int iCommand;
  int iParameter;
};

// Qt Event used to search text
class EventCustomSearchText : public EventWindowCustomCommand
{
public:
  EventCustomSearchText(const InterfaceKeeper& _keeper,
                        HWND _hWin,
                        const QString& _sSearchString,
                        int _iSearchParameter);

  QString sSearchString;
  int iSearchParameter;
};

#endif // CORE_P_H

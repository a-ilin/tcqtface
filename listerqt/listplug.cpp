#include "common.h"
#include "listplug.h"
#include "wlx_interfaces.h"

#include "core.h"
#include "libraryloader.h"
#include "manager.h"
#include "parentwlxwindow.h"

#include <stdio.h>
#include <intrin.h>
#include <memory>

#include <QtWin>

#pragma intrinsic(_ReturnAddress)

#define MANAGER Manager(_ReturnAddress())


HWND createWindow(Manager& manager, HWND hLister, const QString& fileName, int flags)
{
  HWND hWnd = NULL;
  ParentWlxWindow* pWnd = NULL;

  auto payload = createCorePayloadEx(
  []() -> bool {return true;},
  [&]()
  {
    Interface iface = manager.iface();
    _assert(iface);
    if ( iface && iface->isFileAcceptable(fileName) )
    {
      std::unique_ptr<ParentWlxWindow> parent(new ParentWlxWindow(iface, (WId)hLister));
      // it's better to not use unique_ptr because of ownership is transferred to parent
      IAbstractWlxWindow* iwlx = iface->createWindow(parent.get());

      if (iwlx && iwlx->widget())
      {
        parent->setChildWindow(iwlx);

        if (iwlx->loadFile(fileName, flags) == LISTPLUGIN_OK)
        {
          parent->show();

          _log(QString("Window created. Parent: 0x") + QString::number((quint64)parent.get(), 16)
               + QString(", HWND: 0x") + QString::number((quint64)parent->winId(), 16));

          hWnd = (HWND)parent->winId();
          pWnd = parent.release();
        }
      }
      else
      {
        _assert_ex(false, "Cannot cast IAbstractWlxWindow* to QWidget*");
        delete iwlx;
      }
    }
  },
  [&]()
  {
    if (hWnd)
    {
      manager.core()->increaseWinCounter();
    }
  });

  manager.core()->processPayload(payload);

  if ( ! hWnd )
  {
    _log("Window was NOT created!");
  }
  return hWnd;
}

HWND CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListLoad)(HWND ParentWin, char* FileToLoad, int ShowFlags)
{
  _log_line;
  return createWindow(MANAGER, ParentWin, _toString(FileToLoad), ShowFlags);
}

HWND CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListLoadW)(HWND ParentWin, WCHAR* FileToLoad, int ShowFlags)
{
  _log_line;
  return createWindow(MANAGER, ParentWin, _toString(FileToLoad), ShowFlags);
}

static int listLoadNext(Manager& manager, HWND PluginWin, const QString& FileToLoad, int ShowFlags)
{
  int result = LISTPLUGIN_ERROR;

  auto payload = createCorePayload([&]()
  {
    ParentWlxWindow* parent = ParentWlxWindow::getByHandle(PluginWin);
    _assert(parent);
    if (parent)
    {
      result = parent->childWindow()->loadFile(FileToLoad, ShowFlags);
    }
  });

  manager.core()->processPayload(payload);
  return result;
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListLoadNext)(HWND ParentWin, HWND PluginWin,
                                               char* FileToLoad, int ShowFlags)
{
  Q_UNUSED(ParentWin);

  _log_line;
  Manager manager = MANAGER;
  return listLoadNext(manager, PluginWin, _toString(FileToLoad), ShowFlags);
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListLoadNextW)(HWND ParentWin, HWND PluginWin,
                                                WCHAR* FileToLoad, int ShowFlags)
{
  Q_UNUSED(ParentWin);

  _log_line;
  Manager manager = MANAGER;
  return listLoadNext(manager, PluginWin, _toString(FileToLoad), ShowFlags);
}

void CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListCloseWindow)(HWND hWnd)
{
  _log_line;
  Manager manager = MANAGER;

  _assert(hWnd);

  if (hWnd)
  { // if not exists then nothing to do
    ParentWlxWindow* pDeleted = NULL;

    auto payload = createCorePayloadEx(
    []() -> bool { return true; },
    [&]()
    {
      ParentWlxWindow* parent = ParentWlxWindow::getByHandle(hWnd);
      _assert(parent);
      if (parent)
      {
        _log(QString("Window destroyed. Parent: 0x") + QString::number((quint64)parent, 16)
             + QString(", HWND: 0x") + QString::number((quint64)parent->winId(), 16));

        parent->close();

        pDeleted = parent;
      }
    },
    [&]()
    {
      _assert(pDeleted);
      if (pDeleted)
      {
        manager.core()->decreaseWinCounter();
      }
    });

    manager.core()->processPayload(payload);
  }
}

// return file detectable string
void CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListGetDetectString)(char* DetectString, int maxlen)
{
  _log_line;
  Manager manager = MANAGER;

  auto payload = createCorePayload([&]()
  {
    Interface iface = manager.iface();
    if (iface)
    {
      QString detect = iface->getDetectString();

      _assert(detect.size() + 1 < maxlen);
      if (detect.size() + 1 < maxlen)
      {
        QByteArray loc8bit = detect.toLocal8Bit();
        strcpy_s(DetectString, maxlen, loc8bit.constData());
      }
    }
  });

  manager.core()->processPayload(payload);
}

static int listSearchText(Manager& manager, HWND ListWin, const QString& SearchString, int SearchParameter)
{
  int result = LISTPLUGIN_ERROR;

  auto payload = createCorePayload([&]()
  {
    ParentWlxWindow* parent = ParentWlxWindow::getByHandle(ListWin);
    _assert(parent);
    if (parent)
    {
      result = parent->childWindow()->searchText(SearchString, SearchParameter);
    }
  });

  manager.core()->processPayload(payload);
  return result;
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListSearchText) (HWND ListWin, char* SearchString, int SearchParameter)
{
  _log_line;
  Manager manager = MANAGER;
  return listSearchText(manager, ListWin, _toString(SearchString), SearchParameter);
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListSearchTextW) (HWND ListWin, WCHAR* SearchString, int SearchParameter)
{
  _log_line;
  Manager manager = MANAGER;
  return listSearchText(manager, ListWin, _toString(SearchString), SearchParameter);
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListSearchDialog) (HWND ListWin, int FindNext)
{
  _log_line;
  Manager manager = MANAGER;
  int result = LISTPLUGIN_ERROR;

  auto payload = createCorePayload([&]()
  {
    ParentWlxWindow* parent = ParentWlxWindow::getByHandle(ListWin);
    _assert(parent);
    if (parent)
    {
      result = parent->childWindow()->searchDialog(FindNext);
    }
  });

  manager.core()->processPayload(payload);
  return result;
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListSendCommand) (HWND ListWin, int Command, int Parameter)
{
  _log_line;
  Manager manager = MANAGER;
  int result = LISTPLUGIN_ERROR;

  auto payload = createCorePayload([&]()
  {
    ParentWlxWindow* parent = ParentWlxWindow::getByHandle(ListWin);
    _assert(parent);
    if (parent)
    {
      result = parent->childWindow()->sendCommand(Command, Parameter);
    }
  });

  manager.core()->processPayload(payload);
  return result;
}

int listPrint(Manager& manager, HWND ListWin, const QString& FileToPrint, const QString& DefPrinter, int PrintFlags, RECT* Margins)
{
  int result = LISTPLUGIN_ERROR;

  auto payload = createCorePayload([&]()
  {
    ParentWlxWindow* parent = ParentWlxWindow::getByHandle(ListWin);
    _assert(parent);
    if (parent)
    {
      QMarginsF margins(Margins->left, Margins->top, Margins->right, Margins->bottom);
      result = parent->childWindow()->print(FileToPrint, DefPrinter, PrintFlags, margins);
    }
  });

  manager.core()->processPayload(payload);
  return result;
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListPrint) (HWND ListWin, char* FileToPrint, char* DefPrinter,
                                             int PrintFlags, RECT* Margins)
{
  _log_line;
  Manager manager = MANAGER;
  return listPrint(manager, ListWin, _toString(FileToPrint), _toString(DefPrinter), PrintFlags, Margins);
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListPrintW) (HWND ListWin, WCHAR* FileToPrint, WCHAR* DefPrinter,
                                              int PrintFlags, RECT* Margins)
{
  _log_line;
  Manager manager = MANAGER;
  return listPrint(manager, ListWin, _toString(FileToPrint), _toString(DefPrinter), PrintFlags, Margins);
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListNotificationReceived)(HWND ListWin, int Message, WPARAM wParam, LPARAM lParam)
{
  _log_line;
  Manager manager = MANAGER;

  _log(QString("ListWin: 0x%1, Message: 0x%2, wParam: 0x%3, lParam: 0x%4")
       .arg(QString::number((qint64)ListWin, 16))
       .arg(QString::number((qint64)Message, 16))
       .arg(QString::number((qint64)wParam, 16))
       .arg(QString::number((qint64)lParam, 16)));

  return 0;
}

// default params setter
void _set_default_params(ListDefaultParamStruct* dps);

void CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListSetDefaultParams)(ListDefaultParamStruct* dps)
{
  _log_line;
  Manager manager = MANAGER;

  _set_default_params(dps);

  auto payload = createCorePayload([&]()
  {
    Interface iface = manager.iface();
    _assert(iface);
    if (iface)
    {
      iface->setDefaultParams(dps);
    }
  });

  manager.core()->processPayload(payload);
}

static HBITMAP listGetPreviewBitmap(Manager& manager, const QString& FileToLoad,
                                    int width, int height, char *contentbuf, int contentbuflen)
{
  HBITMAP hBitMap = NULL;

  auto payload = createCorePayload([&]()
  {
    Interface iface = manager.iface();
    _assert(iface);
    if (iface)
    {
      QPixmap pixmap = iface->previewBitmap(FileToLoad, QSize(width, height), QByteArray::fromRawData(contentbuf, contentbuflen));
      if ( ! pixmap.isNull() )
      {
        hBitMap = QtWin::toHBITMAP(pixmap, QtWin::HBitmapAlpha);
      }
    }
  });

  manager.core()->processPayload(payload);
  return hBitMap;
}

HBITMAP CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListGetPreviewBitmap)(char* FileToLoad,int width,int height,
                                                           char* contentbuf,int contentbuflen)
{
  _log_line;
  return listGetPreviewBitmap(MANAGER, _toString(FileToLoad), width, height, contentbuf, contentbuflen);
}

HBITMAP CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListGetPreviewBitmapW)(WCHAR* FileToLoad,int width,int height,
                                                            char* contentbuf,int contentbuflen)
{
  _log_line;
  return listGetPreviewBitmap(MANAGER, _toString(FileToLoad), width, height, contentbuf, contentbuflen);
}

// can WLX plugin be unloaded
int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(GetUnloadableStatus)()
{
  LoaderLocker locker;
  if ( Loader::isExists() )
  {
    return Loader::i()->containsLibrary(_ReturnAddress()) ? 0 : 1;
  }
  else
  {
    return 1;
  }
}

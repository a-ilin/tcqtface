#include "common.h"
#include "listplug.h"
#include "wlx_interfaces.h"

#include "core.h"
#include "libraryloader.h"
#include "parentwlxwindow.h"

#include <stdio.h>
#include <intrin.h>
#include <memory>

#include <QtWin>

#pragma intrinsic(_ReturnAddress)

#define KEEPER LibraryLoader::i().keeper(_ReturnAddress())

class LibraryReleaser
{
public:
  ~LibraryReleaser()
  {
    if (LibraryLoader::isExists())
    {
      LibraryLoader* lib = &LibraryLoader::i();
      if (lib->isEmpty())
      {
        delete lib;
      }
    }
  }
};

HWND createWindow(const InterfaceKeeper& keeper, HWND hLister, const QString& fileName, int flags)
{
  IAbstractWlxPlugin* iface = keeper.iface();
  _assert(iface);
  if ( ! iface )
  {
    return NULL;
  }

  if ( ! iface->isFileAcceptable(fileName) )
  {
    _log(QString("File not acceptable: ") + fileName);
    return NULL;
  }

  Core* core = &Core::i();

  HWND hWnd = NULL;
  ParentWlxWindow* pWnd = NULL;

  auto payload = createCorePayloadEx(
  [&]() -> bool
  { // initialize QApplication if needed
    return core->startApplication();
  },
  [&]()
  {
    std::unique_ptr<ParentWlxWindow> parent(new ParentWlxWindow(keeper, (WId)hLister));
    std::unique_ptr<IAbstractWlxWindow> iwlx(iface->createWindow(parent.get()));
    _assert(iwlx && iwlx->widget());

    if (iwlx && iwlx->widget() && (iwlx->loadFile(fileName, flags) == LISTPLUGIN_OK))
    {
      parent->setChildWindow(iwlx.get());
      parent->show();

      _log(QString("Window created. Parent: 0x") + QString::number((quint64)parent.get(), 16)
           + QString(", HWND: 0x") + QString::number((quint64)parent->winId(), 16));

      hWnd = (HWND)parent->winId();
      pWnd = parent.release();
      iwlx.release();
    }
    else
    {
      _log("Window was NOT created!");
    }
  },
  [&]()
  {
    if (hWnd)
    {
      core->increaseWinCounter();
    }
  });

  core->processPayload(payload);

  if (core->isUnusable())
  {
    delete core;
  }

  return hWnd;
}

HWND CALLTYPE FUNC_WRAPPER_EXPORT(ListLoad)(HWND ParentWin, char* FileToLoad, int ShowFlags)
{
  _log_line;
  CHECK_GLOBAL_ERROR(NULL);

  LibraryReleaser libReleaser;

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return createWindow(keeper, ParentWin, _toString(FileToLoad), ShowFlags);
  }

  return NULL;
}

HWND CALLTYPE FUNC_WRAPPER_EXPORT(ListLoadW)(HWND ParentWin, WCHAR* FileToLoad, int ShowFlags)
{
  _log_line;
  CHECK_GLOBAL_ERROR(NULL);

  LibraryReleaser libReleaser;

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return createWindow(KEEPER, ParentWin, _toString(FileToLoad), ShowFlags);
  }

  return NULL;
}

static int listLoadNext(HWND PluginWin, const QString& FileToLoad, int ShowFlags)
{
  _assert(Core::isExists());
  int result = LISTPLUGIN_ERROR;

  auto payload = createCorePayload([&]()
  {
    ParentWlxWindow* parent = ParentWlxWindow::getByHandle(PluginWin);
    result = parent->childWindow()->loadFile(FileToLoad, ShowFlags);
  });

  Core::i().processPayload(payload);

  return result;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListLoadNext)(HWND ParentWin, HWND PluginWin,
                                               char* FileToLoad, int ShowFlags)
{
  Q_UNUSED(ParentWin);

  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  LibraryReleaser libReleaser;

  _assert(Core::isExists());
  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listLoadNext(PluginWin, _toString(FileToLoad), ShowFlags);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListLoadNextW)(HWND ParentWin, HWND PluginWin,
                                                WCHAR* FileToLoad, int ShowFlags)
{
  Q_UNUSED(ParentWin);

  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  LibraryReleaser libReleaser;

  _assert(Core::isExists());
  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listLoadNext(PluginWin, _toString(FileToLoad), ShowFlags);
  }

  return LISTPLUGIN_ERROR;
}

void CALLTYPE FUNC_WRAPPER_EXPORT(ListCloseWindow)(HWND hWnd)
{
  _log_line;
  CHECK_GLOBAL_ERROR();

  LibraryReleaser libReleaser;

  _assert(hWnd);
  _assert(Core::isExists());
  InterfaceKeeper keeper = KEEPER;
  if (hWnd && Core::isExists() && !keeper.isNull())
  { // if not exists then nothing to do
    Core* core = &Core::i();
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
        core->decreaseWinCounter();
      }
    });

    core->processPayload(payload);

    if (core->isUnusable())
    {
      delete core;
    }
  }
}

// return file detectable string
void CALLTYPE FUNC_WRAPPER_EXPORT(ListGetDetectString)(char* DetectString, int maxlen)
{
  _log_line;
  CHECK_GLOBAL_ERROR();

  LibraryReleaser libReleaser;

  QString sDetect;
  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    IAbstractWlxPlugin* pIface = keeper.iface();
    _assert(pIface);
    if ( pIface )
    {
      sDetect = pIface->getDetectString();
    }
  }

  _assert(sDetect.size() + 1 < maxlen);
  if (sDetect.size() + 1 < maxlen)
  {
    QByteArray loc8bit = sDetect.toLocal8Bit();
    strcpy_s(DetectString, maxlen, loc8bit.constData());
  }
}

static int listSearchText(HWND ListWin, const QString& SearchString, int SearchParameter)
{
  _assert(Core::isExists());
  int result = LISTPLUGIN_ERROR;

  auto payload = createCorePayload([&]()
  {
    ParentWlxWindow* parent = ParentWlxWindow::getByHandle(ListWin);
    result = parent->childWindow()->searchText(SearchString, SearchParameter);
  });

  Core::i().processPayload(payload);

  return result;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSearchText) (HWND ListWin, char* SearchString, int SearchParameter)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  LibraryReleaser libReleaser;

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listSearchText(ListWin, _toString(SearchString), SearchParameter);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSearchTextW) (HWND ListWin, WCHAR* SearchString, int SearchParameter)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  LibraryReleaser libReleaser;

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listSearchText(ListWin, _toString(SearchString), SearchParameter);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSearchDialog) (HWND ListWin, int FindNext)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  LibraryReleaser libReleaser;

  _assert(Core::isExists());
  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    int result = LISTPLUGIN_ERROR;

    auto payload = createCorePayload([&]()
    {
      ParentWlxWindow* parent = ParentWlxWindow::getByHandle(ListWin);
      result = parent->childWindow()->searchDialog(FindNext);
    });

    Core::i().processPayload(payload);

    return result;
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSendCommand) (HWND ListWin, int Command, int Parameter)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  LibraryReleaser libReleaser;

  _assert(Core::isExists());
  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    int result = LISTPLUGIN_ERROR;

    auto payload = createCorePayload([&]()
    {
      ParentWlxWindow* parent = ParentWlxWindow::getByHandle(ListWin);
      result = parent->childWindow()->sendCommand(Command, Parameter);
    });

    Core::i().processPayload(payload);

    return result;
  }

  return LISTPLUGIN_ERROR;
}

int listPrint(HWND ListWin, const QString& FileToPrint, const QString& DefPrinter, int PrintFlags, RECT* Margins)
{
  _assert(Core::isExists());

  int result = LISTPLUGIN_ERROR;

  auto payload = createCorePayload([&]()
  {
    ParentWlxWindow* parent = ParentWlxWindow::getByHandle(ListWin);
    QMargins margins(Margins->left, Margins->top, Margins->right, Margins->bottom);
    result = parent->childWindow()->print(FileToPrint, DefPrinter, PrintFlags, margins);
  });

  Core::i().processPayload(payload);

  return result;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListPrint) (HWND ListWin, char* FileToPrint, char* DefPrinter,
                                             int PrintFlags, RECT* Margins)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  LibraryReleaser libReleaser;

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listPrint(ListWin, _toString(FileToPrint), _toString(DefPrinter),
                     PrintFlags, Margins);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListPrintW) (HWND ListWin, WCHAR* FileToPrint, WCHAR* DefPrinter,
                                              int PrintFlags, RECT* Margins)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  LibraryReleaser libReleaser;

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listPrint(ListWin, _toString(FileToPrint), _toString(DefPrinter),
                     PrintFlags, Margins);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListNotificationReceived)(HWND ListWin, int Message, WPARAM wParam, LPARAM lParam)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  LibraryReleaser libReleaser;

  _log(QString("ListWin: 0x%1, Message: 0x%2, wParam: 0x%3, lParam: 0x%4")
       .arg(QString::number((qint64)ListWin, 16))
       .arg(QString::number((qint64)Message, 16))
       .arg(QString::number((qint64)wParam, 16))
       .arg(QString::number((qint64)lParam, 16)));

  return 0;
}

// default params setter
void _set_default_params(ListDefaultParamStruct* dps);

void CALLTYPE FUNC_WRAPPER_EXPORT(ListSetDefaultParams)(ListDefaultParamStruct* dps)
{
  _log_line;
  CHECK_GLOBAL_ERROR();

  LibraryReleaser libReleaser;

  _set_default_params(dps);

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    IAbstractWlxPlugin* pIface = keeper.iface();
    _assert(pIface);
    if ( pIface )
    {
      pIface->setDefaultParams(dps);
    }
  }
}


static HBITMAP listGetPreviewBitmap(const InterfaceKeeper& keeper, const QString& FileToLoad,
                                    int width, int height, char *contentbuf, int contentbuflen)
{
  QPixmap pixmap;

  IAbstractWlxPlugin* pIface = keeper.iface();
  _assert(pIface);
  if ( pIface )
  {
    pixmap = pIface->previewBitmap(FileToLoad, QSize(width, height),
                                   QByteArray::fromRawData(contentbuf, contentbuflen));
  }

  return QtWin::toHBITMAP(pixmap, QtWin::HBitmapAlpha);
}

HBITMAP CALLTYPE FUNC_WRAPPER_EXPORT(ListGetPreviewBitmap)(char* FileToLoad,int width,int height,
                                                           char* contentbuf,int contentbuflen)
{
  _log_line;
  CHECK_GLOBAL_ERROR(NULL);

  LibraryReleaser libReleaser;

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listGetPreviewBitmap(keeper, _toString(FileToLoad), width, height,
                                contentbuf, contentbuflen);
  }

  return NULL;
}

HBITMAP CALLTYPE FUNC_WRAPPER_EXPORT(ListGetPreviewBitmapW)(WCHAR* FileToLoad,int width,int height,
                                                            char* contentbuf,int contentbuflen)
{
  _log_line;
  CHECK_GLOBAL_ERROR(NULL);

  LibraryReleaser libReleaser;

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listGetPreviewBitmap(keeper, _toString(FileToLoad), width, height,
                                contentbuf, contentbuflen);
  }

  return NULL;
}

// can WLX plugin be unloaded
int CALLTYPE FUNC_WRAPPER_EXPORT(GetUnloadableStatus)()
{
  if ( LibraryLoader::isExists() )
  {
    return LibraryLoader::i().containsLibrary(_ReturnAddress()) ? 0 : 1;
  }
  else
  {
    return 1;
  }
}

#include "common.h"
#include "listplug.h"
#include "listplug_qt_iface.h"

#include "core.h"
#include "libraryloader.h"

#include <stdio.h>
#include <intrin.h>

#include <QtWin>

#pragma intrinsic(_ReturnAddress)

#define KEEPER LibraryLoader::i().keeper(_ReturnAddress())

HWND CALLTYPE FUNC_WRAPPER_EXPORT(ListLoad)(HWND ParentWin, char* FileToLoad, int ShowFlags)
{
  _log_line;
  CHECK_GLOBAL_ERROR(NULL);

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return Core::i().createWindow(keeper, ParentWin, _toString(FileToLoad), ShowFlags);
  }

  return NULL;
}

HWND CALLTYPE FUNC_WRAPPER_EXPORT(ListLoadW)(HWND ParentWin, WCHAR* FileToLoad, int ShowFlags)
{
  _log_line;
  CHECK_GLOBAL_ERROR(NULL);

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return Core::i().createWindow(KEEPER, ParentWin, _toString(FileToLoad), ShowFlags);
  }

  return NULL;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListLoadNext)(HWND ParentWin, HWND PluginWin,
                                               char* FileToLoad, int ShowFlags)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  _assert(Core::isExists());
  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return Core::i().loadFile(KEEPER, ParentWin, PluginWin, _toString(FileToLoad), ShowFlags);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListLoadNextW)(HWND ParentWin, HWND PluginWin,
                                                WCHAR* FileToLoad, int ShowFlags)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  _assert(Core::isExists());
  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return Core::i().loadFile(KEEPER, ParentWin, PluginWin, _toString(FileToLoad), ShowFlags);
  }

  return LISTPLUGIN_ERROR;
}

void CALLTYPE FUNC_WRAPPER_EXPORT(ListCloseWindow)(HWND ListWin)
{
  _log_line;
  CHECK_GLOBAL_ERROR();

  _assert(Core::isExists());
  InterfaceKeeper keeper = KEEPER;
  if (Core::isExists() && !keeper.isNull())
  { // if not exists then nothing to do
    Core::i().destroyWindow(keeper, ListWin);
  }
}

// return file detectable string
void CALLTYPE FUNC_WRAPPER_EXPORT(ListGetDetectString)(char* DetectString, int maxlen)
{
  _log_line;
  CHECK_GLOBAL_ERROR();

  QString sDetect;
  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    ListPlugQtIface* pIface = keeper.iface();
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

static int listSearchText(const InterfaceKeeper& keeper, HWND ListWin, const QString& SearchString, int SearchParameter)
{
  _assert(Core::isExists());
  return Core::i().searchText(keeper, ListWin, SearchString, SearchParameter);
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSearchText) (HWND ListWin, char* SearchString, int SearchParameter)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listSearchText(keeper, ListWin, _toString(SearchString), SearchParameter);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSearchTextW) (HWND ListWin, WCHAR* SearchString, int SearchParameter)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listSearchText(keeper, ListWin, _toString(SearchString), SearchParameter);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSearchDialog) (HWND ListWin, int FindNext)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  _assert(Core::isExists());
  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return Core::i().searchDialog(keeper, ListWin, FindNext);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSendCommand) (HWND ListWin, int Command, int Parameter)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  _assert(Core::isExists());
  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return Core::i().sendCommand(keeper, ListWin, Command, Parameter);
  }

  return LISTPLUGIN_ERROR;
}

static int listPrint(const InterfaceKeeper& keeper, HWND ListWin,
                     const QString& FileToPrint, const QString& DefPrinter,
                     int PrintFlags, RECT* Margins)
{
  _assert(Core::isExists());
  return Core::i().print(keeper, ListWin, FileToPrint, DefPrinter, PrintFlags, Margins);
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListPrint) (HWND ListWin, char* FileToPrint, char* DefPrinter,
                                             int PrintFlags, RECT* Margins)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listPrint(keeper, ListWin, _toString(FileToPrint), _toString(DefPrinter),
                     PrintFlags, Margins);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListPrintW) (HWND ListWin, WCHAR* FileToPrint, WCHAR* DefPrinter,
                                              int PrintFlags, RECT* Margins)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listPrint(keeper, ListWin, _toString(FileToPrint), _toString(DefPrinter),
                     PrintFlags, Margins);
  }

  return LISTPLUGIN_ERROR;
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListNotificationReceived)(HWND ListWin, int Message, WPARAM wParam, LPARAM lParam)
{
  _log_line;
  CHECK_GLOBAL_ERROR(LISTPLUGIN_ERROR);

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

  _set_default_params(dps);

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    ListPlugQtIface* pIface = keeper.iface();
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

  ListPlugQtIface* pIface = keeper.iface();
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

  InterfaceKeeper keeper = KEEPER;
  if ( ! keeper.isNull() )
  {
    return listGetPreviewBitmap(keeper, _toString(FileToLoad), width, height,
                                contentbuf, contentbuflen);
  }

  return NULL;
}



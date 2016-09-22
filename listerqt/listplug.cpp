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

#include <QCoreApplication>
#include <QtWin>
#include <QThread>

#pragma intrinsic(_ReturnAddress)

#define MANAGER Manager(_ReturnAddress())

static void callWlxLoadFile(ParentWlxWindow* parent, const QString& fileName, int flags)
{
  // should be called inside qApp thread
  _assert(QThread::currentThread() == qApp->thread());

  _assert(parent);
  if (parent)
  {
    // check that signature is the same at compile-time
    void (ParentWlxWindow::*loadFileFunc)(const QString&, int) = &ParentWlxWindow::loadFile;
    Q_UNUSED(loadFileFunc);

    bool ok = parent->metaObject()->invokeMethod(parent, "loadFile", Qt::QueuedConnection, Q_ARG(QString, fileName), Q_ARG(int, flags));
    _assert(ok);
  }
}

static HWND createWindow(Manager& manager, HWND hLister, const QString& fileName, int ShowFlags)
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
        parent->show();

        _log(QString("Window created. Parent: 0x") + QString::number((quint64)parent.get(), 16)
             + QString(", HWND: 0x") + QString::number((quint64)parent->winId(), 16));

        // postpone load file event
        callWlxLoadFile(parent.get(), fileName, ShowFlags);

        hWnd = (HWND)parent->winId();
        pWnd = parent.release();
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

  manager.core()->processPayload(payload, false);

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

static int listLoadNext(Manager& manager, HWND PluginWin, const QString& fileName, int ShowFlags)
{
  int result = LISTPLUGIN_ERROR;

  auto payload = createCorePayload([&]()
  {
    Interface iface = manager.iface();
    _assert(iface);
    if ( iface && iface->isFileAcceptable(fileName) )
    {
        ParentWlxWindow* parent = ParentWlxWindow::getByHandle(PluginWin);
        _assert(parent);
        if (parent)
        {
          IAbstractWlxWindow* child = parent->childWindow();
          _assert(child);
          if(child)
          {
            // postpone load file event
            callWlxLoadFile(parent, fileName, ShowFlags);
            result = LISTPLUGIN_OK;
          }
        }
    }
  });

  manager.core()->processPayload(payload, false);
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
        parent->setChildWindow(NULL);

        bool closed = parent->close();
        _assert(closed);

        _log(QString("Window destroyed. Parent: 0x") + QString::number((quint64)parent, 16)
             + QString(", HWND: 0x") + QString::number((quint64)parent->winId(), 16));

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
      IAbstractWlxWindow* child = parent->childWindow();
      _assert(child);
      if(child)
      {
          result = child->searchText(SearchString, SearchParameter);
      }
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
      IAbstractWlxWindow* child = parent->childWindow();
      _assert(child);
      if(child)
      {
        result = child->searchDialog(FindNext);
      }
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
      IAbstractWlxWindow* child = parent->childWindow();
      _assert(child);
      if(child)
      {
        result = child->sendCommand(Command, Parameter);
      }
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
      IAbstractWlxWindow* child = parent->childWindow();
      _assert(child);
      if(child)
      {
        QMarginsF margins(Margins->left, Margins->top, Margins->right, Margins->bottom);
        result = child->print(FileToPrint, DefPrinter, PrintFlags, margins);
      }
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
  _set_default_params(dps);
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

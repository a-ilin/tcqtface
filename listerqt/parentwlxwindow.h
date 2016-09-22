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

#ifndef TCMDPARENTWINDOW_H
#define TCMDPARENTWINDOW_H

#include "common.h"
#include "libraryloader.h"
#include "wlx_interfaces.h"

#include <QWidget>

class QTimer;
class IAbstractWlxWindow;

class ParentWlxWindow : public QWidget, public IParentWlxWindow
{
  Q_OBJECT

public:
  ParentWlxWindow(const Interface& keeper, WId parentWin);
  ~ParentWlxWindow();

  void setChildWindow(IAbstractWlxWindow* childWindow);
  IAbstractWlxWindow* childWindow() const { return m_childWindow; }

  WNDPROC origWndProc() const { return m_origWndProc; }
  WNDPROC listerWndProc() const { return m_listerWndProc; }

  static ParentWlxWindow* getByHandle(HWND hwnd);

public:
  // IParentWlxWindow
  void setKeyboardExclusive(bool enable) Q_DECL_OVERRIDE { m_keyboardExclusive = enable; }
  bool isKeyboardExclusive() const Q_DECL_OVERRIDE { return m_keyboardExclusive; }

  void setEscapeOverride(bool is) Q_DECL_OVERRIDE { m_escOverride = is; }
  bool isEscapeOverride() const Q_DECL_OVERRIDE { return m_escOverride; }

  void setListerOptions(int itemtype, int value) const Q_DECL_OVERRIDE;

  QString listerTitle() const;
  void setListerTitle(const QString& title);

public slots:
  void loadFile(const QString& file, int showFlags);
  void reloadFile();

protected:
  void releaseChild();

  void setNativeParent(WId hParentWin);
  WId nativeParent() const;

  void showEvent(QShowEvent* e);
  void resizeEvent(QResizeEvent* e);

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#else
  bool winEvent(MSG *msg, long *result);
#endif

protected slots:
  void onFirstShowTimer();

protected:
  Interface m_keeper;

  bool m_keyboardExclusive;
  bool m_escOverride;

  WNDPROC m_origWndProc;
  WNDPROC m_listerWndProc;

  QTimer* m_firstShowTimer;
  int m_firstShowTimerCounter;

  IAbstractWlxWindow* m_childWindow;
  QWidget* m_childWidget;
};

#endif // TCMDPARENTWINDOW_H


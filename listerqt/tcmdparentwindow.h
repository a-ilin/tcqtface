#ifndef TCMDPARENTWINDOW_H
#define TCMDPARENTWINDOW_H

#include "common.h"
#include "libraryloader.h"

#include <QWidget>

class QTimer;
class TCmdChildWindow;

class TCmdParentWindow : public QWidget
{
  Q_OBJECT

public:
  TCmdParentWindow(const InterfaceKeeper& keeper, WId parentWin);
  ~TCmdParentWindow();

  void setChildWindow(TCmdChildWindow* childWindow);
  TCmdChildWindow* childWindow() const { return m_childWindow; }

  static TCmdParentWindow* getByHandle(HWND hwnd);

  WNDPROC origWndProc() const { return m_origWndProc; }
  WNDPROC listerWndProc() const { return m_listerWndProc; }

  bool isKeyboardExclusive() const { return m_keyboardExclusive; }

  void reloadWidget();

protected:
  void releaseChild();

  void setNativeParent(WId hParentWin);
  WId nativeParent() const;

  void showEvent(QShowEvent *);

#if QT_VERSION >= 0x050000
  bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#else
  bool winEvent(MSG *msg, long *result);
#endif

protected slots:
  void onFirstShowTimer();
  // enable full keyboard processing by the plugin if true
  // otherwise keyboard events will be processed by Lister
  void onSetKeyboardExclusive(bool enable) { m_keyboardExclusive = enable; }
  // set Lister options (see TC docs, section WM_COMMAND)
  void onSetListerOptions(int itemtype, int value);

protected:
  InterfaceKeeper m_keeper;

  bool m_keyboardExclusive;

  WNDPROC m_origWndProc;
  WNDPROC m_listerWndProc;

  QTimer* m_firstShowTimer;
  TCmdChildWindow* m_childWindow;
};

#endif // TCMDPARENTWINDOW_H


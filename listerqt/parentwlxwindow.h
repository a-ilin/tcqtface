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
  ParentWlxWindow(const InterfaceKeeper& keeper, WId parentWin);
  ~ParentWlxWindow();

  void setChildWindow(IAbstractWlxWindow* childWindow);
  IAbstractWlxWindow* childWindow() const { return m_childWindow; }

  WNDPROC origWndProc() const { return m_origWndProc; }
  WNDPROC listerWndProc() const { return m_listerWndProc; }

  void reloadWidget();

  static ParentWlxWindow* getByHandle(HWND hwnd);

public:
  // IParentWlxWindow
  void setKeyboardExclusive(bool enable) Q_DECL_OVERRIDE { m_keyboardExclusive = enable; }
  bool isKeyboardExclusive() const Q_DECL_OVERRIDE { return m_keyboardExclusive; }
  void setListerOptions(int itemtype, int value) const Q_DECL_OVERRIDE;

protected:
  void releaseChild();

  void setNativeParent(WId hParentWin);
  WId nativeParent() const;

  void showEvent(QShowEvent *);

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#else
  bool winEvent(MSG *msg, long *result);
#endif

protected slots:
  void onFirstShowTimer();

protected:
  InterfaceKeeper m_keeper;

  bool m_keyboardExclusive;

  WNDPROC m_origWndProc;
  WNDPROC m_listerWndProc;

  QTimer* m_firstShowTimer;
  IAbstractWlxWindow* m_childWindow;
};

#endif // TCMDPARENTWINDOW_H


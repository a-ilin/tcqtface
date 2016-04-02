#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFrame>
#include <QPointer>

#include <wlx_interfaces.h>

class ProxyWidget : public QFrame
{
public:
  ProxyWidget(QWidget* parent);

  QString load(const QString& filePath);
  QWidget* uiWidget() const { return m_uiWidget; }

protected:
  QWidget* m_uiWidget;
};

class MainWindow : public QWidget, public IAbstractWlxWindow
{
  Q_OBJECT

public:
  MainWindow(IParentWlxWindow* parent);

  int loadFile(const QString& file, int showFlags) Q_DECL_OVERRIDE;
  void reload() Q_DECL_OVERRIDE;

  int print(const QString& file, const QString& printer, int flags, const QMarginsF& margins) Q_DECL_OVERRIDE;

protected:
  QString m_filePath;
  int m_showFlags;

  QPointer<ProxyWidget> m_proxy;
};

#endif // MAINWINDOW_H

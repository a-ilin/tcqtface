#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <listplug_qt_iface.h>

class MainWindow : public QWidget, public TCmdChildWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent);
  ~MainWindow();

  void initEmbedded();

  int loadFile(const QString& file, int showFlags);
  void reload();

signals:
  // sets exclusive keyboard input (no keyboard events to Lister will be propagated)
  void setKeyboardExclusive(bool);

protected:
  QString m_filePath;
  int m_showFlags;
};

#endif // MAINWINDOW_H

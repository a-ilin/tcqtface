#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <wlx_interfaces.h>

class MainWindow : public QWidget, public IAbstractWlxWindow
{
  Q_OBJECT

public:
  MainWindow(IParentWlxWindow* parent);

  int loadFile(const QString& file, int showFlags) Q_DECL_OVERRIDE;
  void reload() Q_DECL_OVERRIDE;

protected:
  QString m_filePath;
  int m_showFlags;
};

#endif // MAINWINDOW_H

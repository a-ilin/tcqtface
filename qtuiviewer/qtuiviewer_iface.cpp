#include <listplug_qt_iface.h>

#include "mainwindow.h"

#include <QXmlInputSource>
#include <QXmlSimpleReader>

class UiViewerListPlugQtIface : public ListPlugQtIface
{
public:

  TCmdChildWindow* createChildWindow(QWidget* parent)
  {
    return new MainWindow(parent);
  }

  QString getDetectString()
  {
    return "EXT=\"UI\"";
  }

  bool isFileAcceptable(const QString& fileName)
  {
    if ( ! fileName.toLower().endsWith(".ui") )
    {
      return false;
    }

    QFile f(fileName);
    if ( ! f.open(QIODevice::ReadOnly) )
    {
      return false;
    }

    QXmlInputSource src(&f);
    QXmlSimpleReader reader;
    if ( ! reader.parse(&src) )
    {
      return false;
    }

    return true;
  }
};


ListPlugQtIface* GetListPlugQtIface()
{
  return new UiViewerListPlugQtIface();
}

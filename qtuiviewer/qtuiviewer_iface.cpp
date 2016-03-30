#include <wlx_interfaces.h>

#include "mainwindow.h"

#include <QXmlInputSource>
#include <QXmlSimpleReader>

class UiViewerWlxPlugin : public IAbstractWlxPlugin
{
public:

  IAbstractWlxWindow* createWindow(IParentWlxWindow* parent) const Q_DECL_OVERRIDE
  {
    return new MainWindow(parent);
  }

  QString getDetectString() const Q_DECL_OVERRIDE
  {
    return "EXT=\"UI\"";
  }

  bool isFileAcceptable(const QString& fileName) const Q_DECL_OVERRIDE
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


IAbstractWlxPlugin* GetWlxPlugin()
{
  return new UiViewerWlxPlugin();
}

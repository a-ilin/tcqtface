#ifndef LISTPLUG_QT_IFACE_H
#define LISTPLUG_QT_IFACE_H

#include <listplug.h>

#include <QWidget>

/* Your plugin should be a DLL
 * that export a function which will create the plugin interface class object:
 * ListPlugQtIface* GetListPlugQtIface();
 *
 * The core plugin library will create a separated thread for the QApplication.
 * The qApp thread will incapsulate all Qt event handling, window management, etc.
 * All interactions with QWidget classes (including plugin window) are provided inside
 * qApp thread.
 *
 */

class TCmdChildWindow;

// Base interface class
class ListPlugQtIface
{
public:
  virtual ~ListPlugQtIface() {}

  // creates a plugin window, this function will be called from qApp thread
  virtual TCmdChildWindow* createChildWindow(QWidget* parent) = 0;
  
  // detect string which used by Total Commander, see TC docs
  // called at plugin first time load
  virtual QString getDetectString() = 0;
  
  // checks if file can be opened by the plugin, called each time on file viewing;
  // use this for fast precheck only
  virtual bool isFileAcceptable(const QString& fileName) = 0;

  // create preview for the file:
  // size: requested size of the preview
  // content: first 8kB data of the file
  virtual QPixmap previewBitmap(const QString& fileName, const QSize& size,
                                const QByteArray& content)
  { Q_UNUSED(fileName); Q_UNUSED(size); Q_UNUSED(content); return QPixmap(); }

  // setup default parameters of the plugin
  virtual void setDefaultParams(ListDefaultParamStruct* dps) { Q_UNUSED(dps); }

};

// typedef for function to export
typedef ListPlugQtIface* (*PGetListPlugQtIface)();
#ifndef PLUGIN_CORE
extern "C" Q_DECL_EXPORT ListPlugQtIface* GetListPlugQtIface();
#endif

// Base class for widget,
// all interactions with widgets are provided in qApp thread, which is separated from
// main TotalCmd thread.
class TCmdChildWindow
{
public:
  virtual ~TCmdChildWindow() {}

  // called after embedding into parent widget, at this moment signals are available
  virtual void initEmbedded() {}

  // load file, return LISTPLUGIN_OK or LISTPLUGIN_ERROR
  virtual int loadFile(const QString& file, int showFlags) = 0;
  // reload file (on F2 press)
  virtual void reload() {}

  // see TC Lister Plugin docs for description of those

  virtual int print(const QString& file, const QString& printer, int flags, const QMargins& margins) {
    Q_UNUSED(file); Q_UNUSED(printer); Q_UNUSED(flags); Q_UNUSED(margins); return LISTPLUGIN_ERROR; }

  virtual int searchText(const QString& str, int searchParameter) {
    Q_UNUSED(str); Q_UNUSED(searchParameter); return LISTPLUGIN_ERROR; }

  virtual int searchDialog(int findNext) { Q_UNUSED(findNext); return LISTPLUGIN_ERROR; }

  virtual int sendCommand(int command, int parameter)
  { Q_UNUSED(command); Q_UNUSED(parameter); return LISTPLUGIN_ERROR; }

public:
  virtual QWidget* widget() const
  { return dynamic_cast<QWidget*>(const_cast<TCmdChildWindow*>(this)); }

  // declare those signals in your class to control specific features
/*
signals:
  // set exclusive keyboard input,
  // when enabled your window will receive all keyboard input, except F2 and ESC keys
  void setKeyboardExclusive(bool);

  // set Lister window options (see TC Lister Plugin docs, section WM_COMMAND)
  void setListerOptions(int itemtype, int value);
*/
};


#endif // LISTPLUG_QT_IFACE_H

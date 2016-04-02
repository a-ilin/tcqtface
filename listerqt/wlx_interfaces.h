#ifndef WLX_INTERFACES_H
#define WLX_INTERFACES_H

#include <listplug.h>

#include <QWidget>

/* Your plugin should be a DLL that exports a function
 * IAbstractWlxPlugin* GetWlxPlugin();
 *
 * This function whould return plugin interface class object. The returned object will be
 * deleted after the last plugin window was closed.
 *
 * NOTE.
 * The core plugin library will create a separate thread for the QApplication.
 * The QApplication thread will incapsulate all Qt event handling, window management, etc.
 * All interactions with QWidget classes (including plugin windows) are inside
 * QApplication thread.
 *
 */

class IAbstractWlxWindow;
class IParentWlxWindow;
class IAbstractWlxPlugin;


// Function to export from plugin DLL
typedef IAbstractWlxPlugin* (*GetWlxPluginFunc)();
#ifndef PLUGIN_CORE
extern "C" Q_DECL_EXPORT IAbstractWlxPlugin* GetWlxPlugin();
#endif


// Base plugin's interface class
class IAbstractWlxPlugin
{
public:
  virtual ~IAbstractWlxPlugin() {}

  // create a plugin window
  // NOTE: Called from QApplication thread!!!
  virtual IAbstractWlxWindow* createWindow(IParentWlxWindow* parent) const = 0;
  
  // detect string which used by Total Commander, see TC docs
  // NOTE: Called from TC thread at plugin's first time load
  virtual QString getDetectString() const = 0;
  
  // checks if file can be opened by the plugin, use this for fast precheck only
  // NOTE: Called from TC thread each time on file viewing
  virtual bool isFileAcceptable(const QString& fileName) const = 0;

  // create preview for the file:
  // size: requested size of the preview
  // content: first 8kB data of the file
  // NOTE: Called from TC thread
  virtual QPixmap previewBitmap(const QString& fileName, const QSize& size, const QByteArray& content) const
  { Q_UNUSED(fileName); Q_UNUSED(size); Q_UNUSED(content); return QPixmap(); }

  // setup default parameters of the plugin
  // NOTE: Called from TC thread
  virtual void setDefaultParams(ListDefaultParamStruct* dps)
  { Q_UNUSED(dps); }
};


// Base class for widget,
// all interactions with widgets are provided in qApp thread, which is separated from
// main TotalCmd thread.
class IAbstractWlxWindow
{
public:
  virtual ~IAbstractWlxWindow() {}

  // called after embedding the window into parent widget
  virtual void initEmbedded() {}

  // load file, return LISTPLUGIN_OK or LISTPLUGIN_ERROR
  virtual int loadFile(const QString& file, int showFlags)
  { Q_UNUSED(file); Q_UNUSED(showFlags); return LISTPLUGIN_ERROR; }
  // reload file (on F2 press)
  virtual void reload() {}

  // see TC Lister Plugin docs for description of those

  virtual int print(const QString& file, const QString& printer, int flags, const QMarginsF& margins)
  { Q_UNUSED(file); Q_UNUSED(printer); Q_UNUSED(flags); Q_UNUSED(margins); return LISTPLUGIN_ERROR; }

  virtual int searchText(const QString& str, int searchParameter)
  { Q_UNUSED(str); Q_UNUSED(searchParameter); return LISTPLUGIN_ERROR; }

  virtual int searchDialog(int findNext)
  { Q_UNUSED(findNext); return LISTPLUGIN_ERROR; }

  virtual int sendCommand(int command, int parameter)
  { Q_UNUSED(command); Q_UNUSED(parameter); return LISTPLUGIN_ERROR; }

  virtual QWidget* widget() const
  { return dynamic_cast<QWidget*>(const_cast<IAbstractWlxWindow*>(this)); }
};


// Parent widget's interface class
class IParentWlxWindow
{
public:
  virtual ~IParentWlxWindow() {}

  // set exclusive keyboard input,
  // when enabled your window will receive all keyboard input, except F2 and ESC keys
  virtual void setKeyboardExclusive(bool enable) = 0;
  virtual bool isKeyboardExclusive() const = 0;

  // set Lister window options (see TC Lister Plugin docs, section WM_COMMAND)
  virtual void setListerOptions(int itemtype, int value) const = 0;

  virtual QWidget* widget() const
  { return dynamic_cast<QWidget*>(const_cast<IParentWlxWindow*>(this)); }
};

#endif // WLX_INTERFACES_H

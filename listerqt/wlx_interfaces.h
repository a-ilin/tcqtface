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
 * All methods of interfaces specified below are called from QApplication thread.
 *
 * NOTE.
 * QApplication object is not living permanently. It will be created
 * and destroyed between interactions with the plugin. As QApplication as your plugin
 * interface will be requested and deleted multiple times.
 */

class IAbstractWlxWindow;
class IParentWlxWindow;
class IAbstractWlxPlugin;

// Function to export from plugin DLL: IAbstractWlxPlugin* GetWlxPlugin();
#ifdef PLUGIN_CORE
typedef IAbstractWlxPlugin* (CALLTYPE_IMPORT *GetWlxPluginFunc)();
#else
typedef IAbstractWlxPlugin* (CALLTYPE_EXPORT *GetWlxPluginFunc)();
EXTERN_EXPORT IAbstractWlxPlugin* GetWlxPlugin();
#endif

// Base plugin's interface class
class IAbstractWlxPlugin
{
public:
  virtual ~IAbstractWlxPlugin() {}

  // Create a plugin window
  virtual IAbstractWlxWindow* createWindow(IParentWlxWindow* parent) const
  { Q_UNUSED(parent); return NULL; }
  
  // Return detect string which is used by TC, see TC docs
  // Called at plugin's first time load
  virtual QString getDetectString() const = 0;
  
  // Check if the file can be opened by the plugin
  // Called each time before creating new window
  virtual bool isFileAcceptable(const QString& fileName) const
  { Q_UNUSED(fileName); return true; }

  // Create preview for the file:
  // size: requested size of the preview
  // content: first several bytes of the file (usually 8kB)
  virtual QPixmap previewBitmap(const QString& fileName, const QSize& size, const QByteArray& content) const
  { Q_UNUSED(fileName); Q_UNUSED(size); Q_UNUSED(content); return QPixmap(); }

  // Setup default parameters of the plugin, see TC docs
  virtual void setDefaultParams(ListDefaultParamStruct* dps)
  { Q_UNUSED(dps); }
};


// Parent widget's interface class
class IParentWlxWindow
{
public:
  virtual ~IParentWlxWindow() {}

  // Set exclusive keyboard input,
  // when enabled your window will receive all keyboard input, except F2 and ESC keys
  virtual void setKeyboardExclusive(bool enable) = 0;
  virtual bool isKeyboardExclusive() const = 0;

  // Set Lister window options (see TC Lister Plugin docs, section WM_COMMAND)
  virtual void setListerOptions(int itemtype, int value) const = 0;

  // Widget of parent window
  QWidget* widget() const
  { return dynamic_cast<QWidget*>(const_cast<IParentWlxWindow*>(this)); }
};


// Base class for widget
class IAbstractWlxWindow
{
public:
  virtual ~IAbstractWlxWindow() {}

  // Called after embedding the window into parent widget
  virtual void initEmbedded() {}

  // Load file, return LISTPLUGIN_OK or LISTPLUGIN_ERROR
  virtual int loadFile(const QString& file, int showFlags)
  { Q_UNUSED(file); Q_UNUSED(showFlags); return LISTPLUGIN_ERROR; }
  // reload file (on F2 press)
  virtual void reload() {}

  // See TC Lister Plugin docs for description of those

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

  IParentWlxWindow* parentWlx() const
  { return dynamic_cast<IParentWlxWindow*>(widget()->parent()); }
};

#endif // WLX_INTERFACES_H

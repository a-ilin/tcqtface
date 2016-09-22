/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 by Aleksei Ilin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

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

// string conversion
#ifndef TOSTRING
#define TOSTRING2(x) #x
#define TOSTRING(x) TOSTRING2(x)
#endif

/* Logging can be customized via lsplugin.ini file
 * Supported entries:
 * LogFile     (default LogNone): full back-slashed path to the log file
 * LogFacility                  : specify minimum facility to be written into the log file
 */

enum LogFacility
{
  LogNone      = 0,  // no logging
  LogCritical  = 1,  // log critical info/assert failures and (optionally) show messageboxes
  LogDebug     = 2   // log all info
};

// Internal core with useful methods
class IWlxCore
{
public:
    virtual ~IWlxCore() {}

    // return path to module by memory addr
    virtual QString pathModuleByAddr(void* addr) const = 0;

    // get directory from complete path
    virtual QString dirByPath(const QString& path) const = 0;

    // return the path to the calling module
    QString pathToThisModule() const
    {
        volatile static TCHAR* localVar = (TCHAR*)0x12345;
        return pathModuleByAddr(&localVar);
    }

    // for logging use macro below instead of those functions
    virtual void __log(const QByteArray& buf, int facility) = 0;
    virtual QByteArray __log_string(const QString& msg,
                                    const char* file, const char* function, const char* line,
                                    bool bTimeStamp, int facility) = 0;
    virtual void __assert(const QString& msg) = 0;
};

// Macro for logging
#define WLX_LOG(iwlxcore, msg, facility) \
    (iwlxcore)->__log((iwlxcore)->__log_string((msg), __FILE__, __FUNCTION__, TOSTRING(__LINE__), true, (facility)), (facility))
#define WLX_ASSERT_EX(iwlxcore, expr, msg) \
    if (0 == (expr)) { \
        WLX_LOG((iwlxcore), QString("ASSERT: ") + QString(msg), LogCritical); \
        (iwlxcore)->__assert(QString(msg)); \
    }
#define WLX_ASSERT(iwlxcore, expr) WLX_ASSERT_EX(iwlxcore, expr, #expr)


// Base plugin's interface class
class IAbstractWlxPlugin
{
public:
  virtual ~IAbstractWlxPlugin() {}

  // setup the plugin for core
  virtual void initCore(IWlxCore* core)
  {
    Q_UNUSED(core);
  }

  // Create a plugin window
  virtual IAbstractWlxWindow* createWindow(IParentWlxWindow* parent) const
  {
    Q_UNUSED(parent);
    return NULL;
  }
  
  // Return detect string which is used by TC, see TC docs
  // Called at plugin's first time load
  virtual QString getDetectString() const = 0;
  
  // Check if the file can be opened by the plugin
  // Called each time before creating new window
  virtual bool isFileAcceptable(const QString& fileName) const
  {
    Q_UNUSED(fileName);
    return true;
  }

  // Create preview for the file:
  // size: requested size of the preview
  // content: first several bytes of the file (usually 8kB)
  virtual QPixmap previewBitmap(const QString& fileName, const QSize& size, const QByteArray& content) const
  {
    Q_UNUSED(fileName);
    Q_UNUSED(size);
    Q_UNUSED(content);
    return QPixmap();
  }

  // Setup default parameters of the plugin, see TC docs
  virtual void setDefaultParams(ListDefaultParamStruct* dps)
  {
    Q_UNUSED(dps);
  }
};


// Parent widget's interface class
class IParentWlxWindow
{
public:
  virtual ~IParentWlxWindow() {}

  // Enable processing all keyboard events. Default is FALSE.
  // When enabled the plugin window will receive all keyboard input, except F2 and ESC keys.
  virtual void setKeyboardExclusive(bool enable) = 0;
  virtual bool isKeyboardExclusive() const = 0;

  // Do not close the window when ESC is pressed. Default is FALSE.
  // When enabled the plugin window will receive ESC key events.
  virtual void setEscapeOverride(bool is) = 0;
  virtual bool isEscapeOverride() const = 0;

  // Set Lister window options (see TC Lister Plugin docs, section WM_COMMAND)
  virtual void setListerOptions(int itemtype, int value) const = 0;

  // Change Lister window title
  virtual QString listerTitle() const = 0;
  virtual void setListerTitle(const QString& title) = 0;

  // Widget of parent window
  QWidget* widget() const
  {
    return dynamic_cast<QWidget*>(const_cast<IParentWlxWindow*>(this));
  }
};


// Base class for widget
class IAbstractWlxWindow
{
public:
  virtual ~IAbstractWlxWindow() {}

  // Called after embedding the window into parent widget,
  // this method could be called after loadFile
  virtual void initEmbedded() {}

  // Load file
  virtual void load(const QString& file, int showFlags)
  {
    Q_UNUSED(file);
    Q_UNUSED(showFlags);
  }

  // Reload file (on F2 press)
  virtual void reload() {}

  // See TC Lister Plugin docs for description of those

  virtual int print(const QString& file, const QString& printer, int flags, const QMarginsF& margins)
  {
    Q_UNUSED(file);
    Q_UNUSED(printer);
    Q_UNUSED(flags);
    Q_UNUSED(margins);
    return LISTPLUGIN_ERROR;
  }

  virtual int searchText(const QString& str, int searchParameter)
  {
    Q_UNUSED(str);
    Q_UNUSED(searchParameter);
    return LISTPLUGIN_ERROR;
  }

  virtual int searchDialog(int findNext)
  {
    Q_UNUSED(findNext);
    return LISTPLUGIN_ERROR;
  }

  virtual int sendCommand(int command, int parameter)
  {
    Q_UNUSED(command);
    Q_UNUSED(parameter);
    return LISTPLUGIN_ERROR;
  }

  // cast IAbstractWlxWindow* to QWidget*
  QWidget* widget() const
  {
    return dynamic_cast<QWidget*>(const_cast<IAbstractWlxWindow*>(this));
  }

  // cast parent QWidget* to IParentWlxWindow*
  IParentWlxWindow* parentWlx() const
  {
    return dynamic_cast<IParentWlxWindow*>(widget()->parent());
  }
};

#endif // WLX_INTERFACES_H

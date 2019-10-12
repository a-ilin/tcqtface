# tcqtface
###### Qt API for Total Commander (c) WLX extensions

This plugin is designed for plugin writers who interested in development of TC plugins based on Qt framework.

Currently this plugin supports TC WLX (Lister) integration. Other plugin types are not supported.

## How to use
1. Build the core plugin module **listerqt**:
```bat
cd listerqt
qmake INSTALL_PATH=c:\SomeDirectory
nmake
nmake install
```

2. Include the shipped *listerqt.pri* into your plugin's .pro file:
`include(C:\SomeDirectory\listerqt.pri)`

3. Implement interfaces specified into *wlx_interfaces.h*. To specify which functions to export from the plugin
   use **CONFIG** variable in your plugin's .pro file. Supported values:

Value | Exported WLX Functions | Description
----- | ---------------------- | -----------
PLUG_LIST_LOAD | ListLoad, ListLoadNext, ListCloseWindow, ListSendCommand | Plugin can create new windows and load files
PLUG_LIST_SEARCH_TEXT | ListSearchText | Plugin window can search a text
PLUG_LIST_SEARCH_DIALOG | ListSearchDialog | Plugin window has it's own search dialog
PLUG_LIST_PRINT | ListPrint | Plugin window can print it's contents
PLUG_LIST_PREVIEW_BITMAP | ListGetPreviewBitmap | Plugin can generate previews for TC

## Examples
There is an example plugin that renders Qt .ui files: qtuiviewer. **It's highly recommended to have a quick view over the code!**

![Screenshot](/qtuiviewer/screenshot.png?raw=true)

## Binaries
Some of precompiled binaries can be found at [releases](https://github.com/a-ilin/tcqtface/releases).

## Known Issues
Static linkage with Qt is preferred because of dynamic linkage has a set of issues:
  - There is no common solution to keep both 32 and 64 bit binaries (wlx, wlx64) at the same directory because of Qt framework shares the same names of it's libraries.
  - Qt doesn't unloads it's plugins and therefore some Qt DLL stays in memory while others are unloaded. This leads to crash on plugin reload: [details](http://www.hexblog.com/?p=991).
  - Static linkage often saves the size of a plugin and speeds up the loading.

## License
[**MIT**](/LICENSE?raw=true)

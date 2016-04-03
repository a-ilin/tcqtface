# tcqtface
###### Qt interface for Total Commander (c)

This plugin is designed for plugin writers who interested in development 
of TC plugins based on Qt framework.

Currently this plugin supports TC WLX (Lister) integration.
Other plugin types are not supported.

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
  * PLUG_LIST_LOAD : plugin can create new windows and load files (ListLoad, ListLoadNext, ListCloseWindow, ListSendCommand)
  * PLUG_LIST_SEARCH_TEXT : plugin window can search a text (ListSearchText)
  * PLUG_LIST_SEARCH_DIALOG : plugin window has it's own search dialog (ListSearchDialog)
  * PLUG_LIST_PRINT : plugin window can print it's contents (ListPrint)
  * PLUG_LIST_PREVIEW_BITMAP : plugin can generate previews for TC (ListGetPreviewBitmap)

## Examples
There is an example plugin that renders Qt .ui files: qtuiviewer. **It's highly recommended to have a quick view over the code!**

## Binaries
Some of precompiled binaries can be found at 'releases' tab: [link](https://github.com/a-ilin/tcqtface/releases).

## Known Issues
At this time there is no common solution to keep both 32 and 64 bit
binaries (wlx, wlx64) at the same directory because of Qt framework
shares the same names of it's libraries. However this can be implemented in future.
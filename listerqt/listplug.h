#ifndef LISTPLUG_H
#define LISTPLUG_H

#include <qt_windows.h>
#include <qglobal.h>

#define MACRO_HELPER(x) x
#define PLUGIN_CORE_SFX proxy_
#define FUNC_WRAPPER_HELPER2(func, sfx) sfx ## func
#define FUNC_WRAPPER_HELPER(func, sfx) FUNC_WRAPPER_HELPER2(func, sfx)
#define FUNC_WRAPPER(func) FUNC_WRAPPER_HELPER(MACRO_HELPER(func), MACRO_HELPER(PLUGIN_CORE_SFX))
#ifdef PLUGIN_CORE
  #define FUNC_WRAPPER_EXPORT(func) FUNC_WRAPPER(func)
  #define FUNC_WRAPPER_IMPORT(func) func
#else
  #define FUNC_WRAPPER_EXPORT(func) func
  #define FUNC_WRAPPER_IMPORT(func) FUNC_WRAPPER(func)
#endif
//#define CALLTYPE __stdcall
#define CALLTYPE
#define CALLTYPE_EXPORT(func, ret) extern "C" Q_DECL_EXPORT ret CALLTYPE FUNC_WRAPPER_EXPORT(func)
#define CALLTYPE_IMPORT(func, ret) extern "C" Q_DECL_IMPORT ret CALLTYPE FUNC_WRAPPER_IMPORT(func)
#define FUNC_DUP(ret, func, param) CALLTYPE_EXPORT(func, ret) param;\
                                   CALLTYPE_IMPORT(func, ret) param;


// internal
FUNC_DUP(int, GetUnloadableStatus, ())


/* Contents of file listplug.h */

#define lc_copy		1
#define lc_newparams	2
#define lc_selectall	3
#define lc_setpercent	4

#define lcp_wraptext	1
#define lcp_fittowindow 2
#define lcp_ansi		4
#define lcp_ascii		8
#define lcp_variable	12
#define lcp_forceshow	16
#define lcp_fitlargeronly 32
#define lcp_center	64

#define lcs_findfirst	1
#define lcs_matchcase	2
#define lcs_wholewords	4
#define lcs_backwards	8

#define itm_percent	0xFFFE
#define itm_fontstyle	0xFFFD
#define itm_wrap		0xFFFC
#define itm_fit		0xFFFB
#define itm_next		0xFFFA
#define itm_center	0xFFF9

#define LISTPLUGIN_OK	0

#define LISTPLUGIN_ERROR	1

typedef struct {
    int size;
    DWORD PluginInterfaceVersionLow;
    DWORD PluginInterfaceVersionHi;
    char DefaultIniName[MAX_PATH];
} ListDefaultParamStruct;


FUNC_DUP(HWND, ListLoad, (HWND ParentWin, char* FileToLoad, int ShowFlags))
FUNC_DUP(HWND, ListLoadW, (HWND ParentWin, WCHAR* FileToLoad, int ShowFlags))
FUNC_DUP(int, ListLoadNext, (HWND ParentWin,HWND PluginWin,char* FileToLoad,int ShowFlags))
FUNC_DUP(int, ListLoadNextW, (HWND ParentWin,HWND PluginWin,WCHAR* FileToLoad,int ShowFlags))
FUNC_DUP(void, ListCloseWindow, (HWND ListWin))
FUNC_DUP(void, ListGetDetectString, (char* DetectString, int maxlen))

FUNC_DUP(int, ListSearchText, (HWND ListWin,char* SearchString,int SearchParameter))
FUNC_DUP(int, ListSearchTextW, (HWND ListWin,WCHAR* SearchString,int SearchParameter))

FUNC_DUP(int, ListSearchDialog, (HWND ListWin,int FindNext))
FUNC_DUP(int, ListSendCommand, (HWND ListWin,int Command,int Parameter))
FUNC_DUP(int, ListPrint, (HWND ListWin,char* FileToPrint,char* DefPrinter,
                        int PrintFlags,RECT* Margins))
FUNC_DUP(int, ListPrintW, (HWND ListWin,WCHAR* FileToPrint,WCHAR* DefPrinter,
                        int PrintFlags,RECT* Margins))

FUNC_DUP(int, ListNotificationReceived, (HWND ListWin,int Message,WPARAM wParam,LPARAM lParam))
FUNC_DUP(void, ListSetDefaultParams, (ListDefaultParamStruct* dps))

FUNC_DUP(HBITMAP, ListGetPreviewBitmap, (char* FileToLoad,int width,int height,
    char* contentbuf,int contentbuflen))
FUNC_DUP(HBITMAP, ListGetPreviewBitmapW, (WCHAR* FileToLoad,int width,int height,
    char* contentbuf,int contentbuflen))


#endif // LISTPLUG_H

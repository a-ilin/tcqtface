#include <listplug.h>

// do not optimize those wrappers,
// otherwise it will be impossible to get module handle by RetAddr
#pragma optimize( "", off )

HWND CALLTYPE FUNC_WRAPPER_EXPORT(ListLoad)(HWND ParentWin, char* FileToLoad, int ShowFlags)
{
  return FUNC_WRAPPER_IMPORT(ListLoad)(ParentWin, FileToLoad, ShowFlags);
}

HWND CALLTYPE FUNC_WRAPPER_EXPORT(ListLoadW)(HWND ParentWin, WCHAR* FileToLoad, int ShowFlags)
{
  return FUNC_WRAPPER_IMPORT(ListLoadW)(ParentWin, FileToLoad, ShowFlags);
}

int CALLTYPE ListLoadNext(HWND ParentWin, HWND PluginWin, char* FileToLoad, int ShowFlags)
{
  return FUNC_WRAPPER_IMPORT(ListLoadNext)(ParentWin, PluginWin, FileToLoad, ShowFlags);
}

int CALLTYPE ListLoadNextW(HWND ParentWin, HWND PluginWin, WCHAR* FileToLoad, int ShowFlags)
{
  return FUNC_WRAPPER_IMPORT(ListLoadNextW)(ParentWin, PluginWin, FileToLoad, ShowFlags);
}

void CALLTYPE FUNC_WRAPPER_EXPORT(ListCloseWindow)(HWND ListWin)
{
  FUNC_WRAPPER_IMPORT(ListCloseWindow)(ListWin);
}

void CALLTYPE FUNC_WRAPPER_EXPORT(ListGetDetectString)(char* DetectString, int maxlen)
{
  FUNC_WRAPPER_IMPORT(ListGetDetectString)(DetectString, maxlen);
}

// add define to enable text search into PRO file: DEFINES += PLUG_LIST_SEARCH_TEXT
#ifdef PLUG_LIST_SEARCH_TEXT
int CALLTYPE FUNC_WRAPPER_EXPORT(ListSearchText)(HWND ListWin, char* SearchString, int SearchParameter)
{
  return FUNC_WRAPPER_IMPORT(ListSearchText)(ListWin, SearchString, SearchParameter);
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSearchTextW)(HWND ListWin, WCHAR* SearchString, int SearchParameter)
{
  return FUNC_WRAPPER_IMPORT(ListSearchTextW)(ListWin, SearchString, SearchParameter);
}
#endif // PLUG_LIST_SEARCH_TEXT

#if 0 // Qt has it's own event dispatcher
int CALLTYPE FUNC_WRAPPER_EXPORT(ListNotificationReceived)(HWND ListWin, int Message, WPARAM wParam, LPARAM lParam)
{
  return FUNC_WRAPPER_IMPORT(ListNotificationReceived)(ListWin, Message, wParam, lParam);
}
#endif

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSearchDialog)(HWND ListWin, int FindNext)
{
  return FUNC_WRAPPER_IMPORT(ListSearchDialog)(ListWin, FindNext);
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListSendCommand)(HWND ListWin, int Command, int Parameter)
{
  return FUNC_WRAPPER_IMPORT(ListSendCommand)(ListWin, Command, Parameter);
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListPrint)(HWND ListWin, char* FileToPrint, char* DefPrinter,
                                            int PrintFlags, RECT* Margins)
{
  return FUNC_WRAPPER_IMPORT(ListPrint)(ListWin, FileToPrint, DefPrinter, PrintFlags, Margins);
}

int CALLTYPE FUNC_WRAPPER_EXPORT(ListPrintW)(HWND ListWin, WCHAR* FileToPrint, WCHAR* DefPrinter,
                                             int PrintFlags, RECT* Margins)
{
  return FUNC_WRAPPER_IMPORT(ListPrintW)(ListWin, FileToPrint, DefPrinter, PrintFlags, Margins);
}

void CALLTYPE FUNC_WRAPPER_EXPORT(ListSetDefaultParams)(ListDefaultParamStruct* dps)
{
  FUNC_WRAPPER_IMPORT(ListSetDefaultParams)(dps);
}

// add define to enable preview into PRO file: DEFINES += PLUG_LIST_PREVIEW_BITMAP
#ifdef PLUG_LIST_PREVIEW_BITMAP
HBITMAP CALLTYPE FUNC_WRAPPER_EXPORT(ListGetPreviewBitmap)(char* FileToLoad,int width,int height,
                                                           char* contentbuf,int contentbuflen)
{
  return FUNC_WRAPPER_IMPORT(ListGetPreviewBitmap)(FileToLoad, width, height,
                                                   contentbuf, contentbuflen);
}

HBITMAP CALLTYPE FUNC_WRAPPER_EXPORT(ListGetPreviewBitmapW)(WCHAR* FileToLoad,int width,int height,
                                                           char* contentbuf,int contentbuflen)
{
  return FUNC_WRAPPER_IMPORT(ListGetPreviewBitmapW)(FileToLoad, width, height,
                                                    contentbuf, contentbuflen);
}
#endif // PLUG_LIST_PREVIEW_BITMAP

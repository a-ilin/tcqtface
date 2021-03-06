/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 by Aleksei Ilin
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

#include <listplug.h>

// do not optimize those wrappers,
// otherwise it will be impossible to get module handle by RetAddr
#pragma optimize( "", off )

#ifdef PLUG_LIST_LOAD
HWND CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListLoad)(HWND ParentWin, char* FileToLoad, int ShowFlags)
{
  return FUNC_WRAPPER_IMPORT(ListLoad)(ParentWin, FileToLoad, ShowFlags);
}

HWND CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListLoadW)(HWND ParentWin, WCHAR* FileToLoad, int ShowFlags)
{
  return FUNC_WRAPPER_IMPORT(ListLoadW)(ParentWin, FileToLoad, ShowFlags);
}

int CALLTYPE_EXPORT ListLoadNext(HWND ParentWin, HWND PluginWin, char* FileToLoad, int ShowFlags)
{
  return FUNC_WRAPPER_IMPORT(ListLoadNext)(ParentWin, PluginWin, FileToLoad, ShowFlags);
}

int CALLTYPE_EXPORT ListLoadNextW(HWND ParentWin, HWND PluginWin, WCHAR* FileToLoad, int ShowFlags)
{
  return FUNC_WRAPPER_IMPORT(ListLoadNextW)(ParentWin, PluginWin, FileToLoad, ShowFlags);
}

void CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListCloseWindow)(HWND ListWin)
{
  FUNC_WRAPPER_IMPORT(ListCloseWindow)(ListWin);
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListSendCommand)(HWND ListWin, int Command, int Parameter)
{
  return FUNC_WRAPPER_IMPORT(ListSendCommand)(ListWin, Command, Parameter);
}

#endif // PLUG_LIST_LOAD

void CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListGetDetectString)(char* DetectString, int maxlen)
{
  FUNC_WRAPPER_IMPORT(ListGetDetectString)(DetectString, maxlen);
}

#ifdef PLUG_LIST_SEARCH_TEXT
int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListSearchText)(HWND ListWin, char* SearchString, int SearchParameter)
{
  return FUNC_WRAPPER_IMPORT(ListSearchText)(ListWin, SearchString, SearchParameter);
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListSearchTextW)(HWND ListWin, WCHAR* SearchString, int SearchParameter)
{
  return FUNC_WRAPPER_IMPORT(ListSearchTextW)(ListWin, SearchString, SearchParameter);
}
#endif // PLUG_LIST_SEARCH_TEXT

#if 0 // Qt has it's own event dispatcher
int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListNotificationReceived)(HWND ListWin, int Message, WPARAM wParam, LPARAM lParam)
{
  return FUNC_WRAPPER_IMPORT(ListNotificationReceived)(ListWin, Message, wParam, lParam);
}
#endif

#ifdef PLUG_LIST_SEARCH_DIALOG
int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListSearchDialog)(HWND ListWin, int FindNext)
{
  return FUNC_WRAPPER_IMPORT(ListSearchDialog)(ListWin, FindNext);
}
#endif // PLUG_LIST_SEARCH_DIALOG

#ifdef PLUG_LIST_PRINT
int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListPrint)(HWND ListWin, char* FileToPrint, char* DefPrinter,
                                            int PrintFlags, RECT* Margins)
{
  return FUNC_WRAPPER_IMPORT(ListPrint)(ListWin, FileToPrint, DefPrinter, PrintFlags, Margins);
}

int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListPrintW)(HWND ListWin, WCHAR* FileToPrint, WCHAR* DefPrinter,
                                             int PrintFlags, RECT* Margins)
{
  return FUNC_WRAPPER_IMPORT(ListPrintW)(ListWin, FileToPrint, DefPrinter, PrintFlags, Margins);
}
#endif // PLUG_LIST_PRINT

void CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListSetDefaultParams)(ListDefaultParamStruct* dps)
{
  FUNC_WRAPPER_IMPORT(ListSetDefaultParams)(dps);
}

#ifdef PLUG_LIST_PREVIEW_BITMAP
HBITMAP CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListGetPreviewBitmap)(char* FileToLoad,int width,int height,
                                                           char* contentbuf,int contentbuflen)
{
  return FUNC_WRAPPER_IMPORT(ListGetPreviewBitmap)(FileToLoad, width, height,
                                                   contentbuf, contentbuflen);
}

HBITMAP CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(ListGetPreviewBitmapW)(WCHAR* FileToLoad,int width,int height,
                                                           char* contentbuf,int contentbuflen)
{
  return FUNC_WRAPPER_IMPORT(ListGetPreviewBitmapW)(FileToLoad, width, height,
                                                    contentbuf, contentbuflen);
}
#endif // PLUG_LIST_PREVIEW_BITMAP


int CALLTYPE_EXPORT FUNC_WRAPPER_EXPORT(GetUnloadableStatus)()
{
  return FUNC_WRAPPER_IMPORT(GetUnloadableStatus)();
}

extern "C" Q_DECL_EXPORT const char _qt_version[128] = "_qt_version=" QMAKESPEC "_" QT_VERSION_STR "_" ARCH;

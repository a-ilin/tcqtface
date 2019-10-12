#
# The MIT License (MIT)
#
# Copyright (c) 2015-2019 by Aleksei Ilin
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

QT += core gui widgets winextras

# Switch static/shared library
contains(CONFIG, static) {
  DEFINES += STATIC_BUILD
}

# Architecture
contains(QMAKE_TARGET.arch, x86_64): {
  ARCH = x64
} else {
  ARCH = x86
}
DEFINES += ARCH=\\\"$$ARCH\\\"

# QMAKESPEC
mkspec = $$replace(QMAKESPEC, "\\", "/")
mkspec = $$section(mkspec, "/", -1, -1)
DEFINES += QMAKESPEC=\\\"$$mkspec\\\"

# library name rule:
# - 64bit has suffix "64"
# - debug build has suffix "d"
defineReplace(coreName) {
  name = $${1}

  equals(ARCH, x64): {
    name = $${name}64
  }

  CONFIG(debug, debug|release) {
    name = $${name}d
  }

  return($${name})
}

# Enable asynchronous SEH
QMAKE_CXXFLAGS_EXCEPTIONS_ON = -EHa
QMAKE_CXXFLAGS_STL_ON = -EHa

# link libs
contains(CONFIG, PLUGIN_CORE)|contains(CONFIG, static) {
  LIBS += -luser32
}

# Enable debuginfo
CONFIG += force_debug_info

# Setup name of core library
CORE_LIB_NAME = $$coreName(listerqt)

# User plugin
! contains(CONFIG, PLUGIN_CORE) {
  CONFIG(debug, debug|release) {
    TARGET = $${TARGET}d
  }

  # FIX: qt doesn't link plugins if TEMPLATE is not an app
  contains(CONFIG, static) {
    CONFIG += dll force_import_plugins
    QTPLUGIN += \
      qwindows \
      windowsprintersupport \
      qdds \
      qicns \
      qico \
      qsvg \
      qtga \
      qtiff \
      qwbmp \
      qwebp
  }

  INCLUDEPATH += $$PWD

  LIBS += -L$$PWD -l$${CORE_LIB_NAME}

  equals(ARCH, x64): {
    TARGET_EXT = ".wlx64"
  } else {
    TARGET_EXT = ".wlx"
  }

  HEADERS += $$PWD/wlx_interfaces.h
  SOURCES += $$PWD/listerqt.cpp

  # populate DEF file
  DEF_FILE = $$OUT_PWD/listerqt.def

  defContents = \
    "; Declares the module parameters for the DLL." \
    "EXPORTS" \
    "GetWlxPlugin" \
    "GetUnloadableStatus" \
    "ListGetDetectString" \
    "ListSetDefaultParams"

  contains(CONFIG, PLUG_LIST_LOAD) {
    DEFINES += PLUG_LIST_LOAD
    defContents += \
      "ListLoad" \
      "ListLoadW" \
      "ListLoadNext" \
      "ListLoadNextW" \
      "ListCloseWindow" \
      "ListSendCommand"
  }

  contains(CONFIG, PLUG_LIST_SEARCH_TEXT) {
    DEFINES += PLUG_LIST_SEARCH_TEXT
    defContents += \
      "ListSearchText" \
      "ListSearchTextW"
  }

  contains(CONFIG, PLUG_LIST_SEARCH_DIALOG) {
    DEFINES += PLUG_LIST_SEARCH_DIALOG
    defContents += \
      "ListSearchDialog"
  }

  contains(CONFIG, PLUG_LIST_PRINT) {
    DEFINES += PLUG_LIST_PRINT
    defContents += \
      "ListPrint" \
      "ListPrintW"
  }

  contains(CONFIG, PLUG_LIST_PREVIEW_BITMAP) {
    DEFINES += PLUG_LIST_PREVIEW_BITMAP
    defContents += \
      "ListGetPreviewBitmap" \
      "ListGetPreviewBitmapW"
  }

  write_file($$DEF_FILE, defContents)

} else {
  ! contains(CONFIG, static) {
    TARGET_EXT = ".dll"
  }
}

DEFINES += TARGET=\\\"$$TARGET\\\" TARGET_EXT=\\\"$$TARGET_EXT\\\"

#
# The MIT License (MIT)
#
# Copyright (c) 2015-2016 by Aleksei Ilin
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

TEMPLATE = lib
CONFIG += c++11 PLUGIN_CORE
DEFINES += PLUGIN_CORE

include(listerqt.pri)

TARGET = $${CORE_LIB_NAME}

! contains(CONFIG, static) {
  RC_FILE = version.rc
}

INCLUDEPATH += $$PWD

SOURCES += \
  listplug.cpp \
  parentwlxwindow.cpp \
  libraryloader.cpp \
  application.cpp \
  seexception.cpp \
  core.cpp \
  common.cpp \
  manager.cpp \
  wlxcore.cpp

HEADERS += \
  listplug.h \
  wlx_interfaces.h \
  parentwlxwindow.h \
  common.h \
  wlxcore.h \
  libraryloader.h \
  application.h \
  seexception.h \
  core.h \
  core_p.h \
  atomicmutex.h \
  manager.h \
  semaphore.h \
  event.h \
  thread.h


! isEmpty(INSTALL_PATH) {
distrib.files = \
  listerqt.pri \
  listerqt.cpp \
  wlx_interfaces.h \
  listplug.h

INSTALLS += \
  target \
  distrib

for (dist, INSTALLS) {
  $${dist}.path = $$INSTALL_PATH
}
}

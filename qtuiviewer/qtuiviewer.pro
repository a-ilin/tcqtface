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

QT += core gui printsupport widgets xml

# Designer uses shared linking while uitools uses static
#DEFINES += USE_DESIGNER

contains(DEFINES, USE_DESIGNER) {
  QT += designer
} else {
  QT += uitools
}

TARGET = qtuiviewer

TEMPLATE = lib

# Can create windows
CONFIG += PLUG_LIST_LOAD
# Can print
CONFIG += PLUG_LIST_PRINT
# No text search available
#CONFIG += PLUG_LIST_SEARCH_TEXT
# No search dialog available
#CONFIG += PLUG_LIST_SEARCH_DIALOG
# No preview available
#CONFIG += PLUG_LIST_PREVIEW_BITMAP

isEmpty(TCQTFACE_PATH) {
error(Specify TCQTFACE_PATH variable!)
}

include($$TCQTFACE_PATH/listerqt.pri)

RC_FILE = version.rc

SOURCES += \
  mainwindow.cpp \
  qtuiviewer_iface.cpp

HEADERS += \
  mainwindow.h


! isEmpty(INSTALL_PATH) {
distrib.files = \
  pluginst.inf

INSTALLS += \
  target \
  distrib

for (dist, INSTALLS) {
  $${dist}.path = $$INSTALL_PATH
}

}

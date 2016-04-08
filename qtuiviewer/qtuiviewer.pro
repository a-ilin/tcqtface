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

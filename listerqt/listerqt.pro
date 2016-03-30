QT += core gui widgets winextras

TEMPLATE = lib
CONFIG += shared c++11 PLUGIN_CORE
DEFINES += PLUGIN_CORE

include(listerqt.pri)

TARGET = $${CORE_LIB_NAME}

LIBS += -luser32

SOURCES += \
  listplug.cpp \
  tcmdparentwindow.cpp \
  libraryloader.cpp \
  application.cpp \
  seexception.cpp \
  core.cpp \
  atomicmutex.cpp \
  common.cpp

HEADERS += \
  listplug.h \
  listplug_qt_iface.h \
  tcmdparentwindow.h \
  common.h \
  libraryloader.h \
  application.h \
  seexception.h \
  core.h \
  core_p.h \
  atomicmutex.h


! isEmpty(INSTALL_PATH) {
distrib.files = \
  listerqt.pri \
  listerqt.cpp \
  listplug_qt_iface.h \
  listplug.h

INSTALLS += \
  target \
  distrib

for (dist, INSTALLS) {
  $${dist}.path = $$INSTALL_PATH
}
}

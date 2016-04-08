TEMPLATE = lib
CONFIG += c++11 PLUGIN_CORE
DEFINES += PLUGIN_CORE

include(listerqt.pri)

contains(CONFIG, static) {
  CONFIG += staticlib
}

TARGET = $${CORE_LIB_NAME}

LIBS += -luser32

! contains(CONFIG, staticlib) {
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
  common.cpp

HEADERS += \
  listplug.h \
  wlx_interfaces.h \
  parentwlxwindow.h \
  common.h \
  libraryloader.h \
  application.h \
  seexception.h \
  core.h \
  core_p.h \
  atomicmutex.h \
    manager.h \
    semaphore.h \
    event.h


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

QT += core gui widgets winextras

TEMPLATE = lib
CONFIG += shared c++11 PLUGIN_CORE
DEFINES += PLUGIN_CORE

include(listerqt.pri)

TARGET = $${CORE_LIB_NAME}

DESTDIR = $$PWD/../../dist/$${ARCH}/

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

DISTRIB = \
  listerqt.pri \
  listerqt.cpp \
  listplug_qt_iface.h \
  listplug.h

defineReplace(addCopyCmd) {
  src = $$PWD/$$1
  dst = $$DESTDIR/$$1
  src = $$replace(src, "/", "\\")
  dst = $$replace(dst, "/", "\\")

  return($(COPY) $$src $$dst)
}

for (dist, DISTRIB) {
  ! isEmpty(copyfs.commands) {
    copyfs.commands = $${copyfs.commands}$$escape_expand(\n\t)
  }
  copyfs.commands = $${copyfs.commands}$$addCopyCmd($$dist)
}

first.depends = copyfs $(first)
export(copyfs.commands)
export(first.depends)
QMAKE_EXTRA_TARGETS += copyfs first


QT += core gui widgets xml

# Designer uses shared linking while uitools uses static
DEFINES += USE_DESIGNER

contains(DEFINES, USE_DESIGNER) {
  QT += designer
} else {
  QT += uitools
}

TARGET = qtuiviewer

TEMPLATE = lib

# Architecture
contains(QMAKE_TARGET.arch, x86_64): {
  ARCH = x64
} else {
  ARCH = x86
}

DESTDIR = $$PWD/../../dist/$${ARCH}/

include($$DESTDIR/listerqt.pri)

SOURCES += \
    mainwindow.cpp \
    qtuiviewer_iface.cpp

HEADERS += \
    mainwindow.h


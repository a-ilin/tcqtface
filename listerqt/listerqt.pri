
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

# Enable debuginfo
QMAKE_CFLAGS_RELEASE   = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE   = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
CONFIG += force_debug_info

# Use static MSVC runtime
CONFIG += static_runtime

# Setup name of core library
CORE_LIB_NAME = $$coreName(listerqt)

# User plugin
! contains(CONFIG, PLUGIN_CORE) {
  CONFIG(debug, debug|release) {
    TARGET = $${TARGET}d
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
} else {
  TARGET_EXT = ".dll"
}

DEFINES += TARGET=\\\"$$TARGET\\\" TARGET_EXT=\\\"$$TARGET_EXT\\\"

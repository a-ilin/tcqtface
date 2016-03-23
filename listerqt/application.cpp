#include "application.h"
#include "common.h"

#include <QStyleFactory>

// pseudo command line arguments to be passed to QApplication
int argc = 1;
const char* argv[] = { "invalid", "" };

Application::Application() :
  QApplication(argc, const_cast<char**>(argv))
{
  setQuitOnLastWindowClosed(false);

  AppSet set;
  QString style = set.value("Style").toString();
  setStyle(style);
}


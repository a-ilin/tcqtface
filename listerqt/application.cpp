#include "application.h"
#include "common.h"
#include "seexception.h"
#include <QStyleFactory>

// pseudo command line arguments to be passed to QApplication
int argc = 1;
const char* argv[] = { "invalid", "" };

Application::Application() :
  QApplication(argc, const_cast<char**>(argv))
{
  setQuitOnLastWindowClosed(false);

  QString style = AppSet::readString("Style");
  if (style.size())
  {
    setStyle(style);
  }
}

static void showException(const QString& msg)
{
  QString prolog("Exception in QApplication Thread\n");
  _assert_ex(false, prolog + msg);
}

bool Application::notify(QObject* o, QEvent* e)
{
  bool res = true;

  try
  {
    res = QApplication::notify(o, e);
  }
  catch(SeException* ex)
  {
    showException(ex->msg());
  }
  catch(const std::exception& ex)
  {
    showException(ex.what());
  }
  catch(...)
  {
    showException("Unknown Exception");
  }

  return res;
}


/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 by Aleksei Ilin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

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
  catch(const QString& s)
  {
    showException(s);
  }
  catch(const char* s)
  {
    showException(s);
  }
  catch(...)
  {
    showException("Unknown Exception");
  }

  return res;
}


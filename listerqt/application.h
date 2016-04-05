#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class Application : public QApplication
{
public:
  Application();

protected:
  bool notify(QObject* o, QEvent* e);
};

#endif // APPLICATION_H

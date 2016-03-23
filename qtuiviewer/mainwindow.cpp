#include "mainwindow.h"

#ifdef USE_DESIGNER
#include <QFormBuilder>
#else
#include <QUiLoader>
#endif

#include <QApplication>
#include <QFile>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QSpacerItem>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

static void clearLayout(QLayout *layout)
{
    QLayoutItem *item;
    while((item = layout->takeAt(0)))
    {
        if (item->layout())
        {
            clearLayout(item->layout());
            delete item->layout();
        }

        if (item->widget())
        {
            delete item->widget();
        }

        delete item;
    }
}

class ProxyWidget : public QFrame
{
public:
  ProxyWidget(QWidget* parent);

  QString load(const QString& filePath);
};

ProxyWidget::ProxyWidget(QWidget* parent) :
  QFrame(parent)
{
  QVBoxLayout* lay = new QVBoxLayout(this);
  lay->setContentsMargins(0,0,0,0);
  setLayout(lay);

  // set original background
  setAutoFillBackground(true);
  QPalette pal = palette();
  pal.setBrush(QPalette::Window, qApp->palette().brush(QPalette::Window));
  setPalette(pal);

  setFrameShadow(QFrame::Plain);
  setFrameShape(QFrame::Box);
}

QString ProxyWidget::load(const QString& filePath)
{
#ifdef USE_DESIGNER
  QFormBuilder uiLoader;
#else
  QUiLoader uiLoader;
#endif

  QFile file(filePath);
  file.open(QFile::ReadOnly);
  QWidget* uiWidget = uiLoader.load(&file, this);
  file.close();

  QString error = uiLoader.errorString();

  if (uiWidget)
  {
    uiWidget->setWindowFlags(Qt::Widget);
    uiWidget->setParent(this);
    uiWidget->show();
    layout()->addWidget(uiWidget);
  }
  else if (error.isEmpty())
  {
    error = tr("Cannot load UI from file: ") + filePath;
  }

  return error;
}


MainWindow::MainWindow(QWidget* parent) :
  QWidget(parent),
  TCmdChildWindow(),
  m_showFlags(0)
{
  QGridLayout* lay = new QGridLayout(this);
  setLayout(lay);

  // set white background
  setAutoFillBackground(true);
  QPalette pal = palette();
  pal.setBrush(QPalette::Window, QBrush(Qt::white));
  setPalette(pal);
}

MainWindow::~MainWindow()
{

}

void MainWindow::initEmbedded()
{
  emit setKeyboardExclusive(true);
}

int MainWindow::loadFile(const QString& file, int showFlags)
{
  m_filePath = file;
  m_showFlags = showFlags;
  reload();
  return LISTPLUGIN_OK;
}

void MainWindow::reload()
{
  QGridLayout* lay = static_cast<QGridLayout*> (layout());

  // clear
  clearLayout(lay);

  ProxyWidget* proxy = new ProxyWidget(this);
  QString error = proxy->load(m_filePath);

  int layoutRow = 0;

  if ( ! error.isEmpty() )
  {
    QLabel * errLbl = new QLabel(this);
    QString errTmpl = "<html><head/><body><p><span style=\" "
                      "color:red;\"> %1 %2:<br>%3</span></p></body></html>";
    errLbl->setText(errTmpl.arg(tr("Error occured while loading"))
                           .arg(m_filePath)
                           .arg(error));

    lay->addWidget(errLbl, layoutRow++, 0);
  }

  lay->addWidget(proxy, layoutRow, 0);
  // horizontal spacer
  lay->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum), layoutRow, 1);

  ++layoutRow;

  // vertical spacer
  lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), layoutRow, 0);
}


#include "mainwindow.h"

#ifdef USE_DESIGNER
#ifdef STATIC_BUILD
  #define QT_DESIGNER_STATIC
  #undef QDESIGNER_UILIB_LIBRARY
#endif
#include <QFormBuilder>
#else
#include <QUiLoader>
#endif

#include <QApplication>
#include <QFile>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
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

ProxyWidget::ProxyWidget(QWidget* parent)
  : QFrame(parent)
  , m_uiWidget(NULL)
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
  m_uiWidget = uiLoader.load(&file, this);
  file.close();

  QString error = uiLoader.errorString();

  if (m_uiWidget)
  {
    m_uiWidget->setWindowFlags(Qt::Widget);
    m_uiWidget->setParent(this);
    m_uiWidget->show();
    layout()->addWidget(m_uiWidget);
  }
  else if (error.isEmpty())
  {
    error = tr("Cannot load UI from file: ") + filePath;
  }

  return error;
}


MainWindow::MainWindow(IParentWlxWindow* parent) :
  QWidget(parent->widget()),
  IAbstractWlxWindow(),
  m_showFlags(0)
{
  QGridLayout* lay = new QGridLayout(this);
  setLayout(lay);

  // set white background
  setAutoFillBackground(true);
  QPalette pal = palette();
  pal.setBrush(QPalette::Window, QBrush(Qt::white));
  setPalette(pal);

  parent->setKeyboardExclusive(true);
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

  m_proxy = new ProxyWidget(this);
  QString error = m_proxy->load(m_filePath);

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

  lay->addWidget(m_proxy, layoutRow, 0);
  // horizontal spacer
  lay->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum), layoutRow, 1);

  ++layoutRow;

  // vertical spacer
  lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), layoutRow, 0);
}

int MainWindow::print(const QString& /*file*/, const QString& printerName, int /*flags*/, const QMarginsF& margins)
{
  QPixmap pix;
  if (m_proxy && m_proxy->uiWidget())
  {
    pix = m_proxy->uiWidget()->grab();
  }

  if ( pix.isNull() )
  {
    QMessageBox::information(this, tr("Nothing to print"), tr("No file is loaded"));
    return LISTPLUGIN_ERROR;
  }

  QPrinterInfo printInfo;
  QList<QPrinterInfo> infos = QPrinterInfo::availablePrinters();
  for (const QPrinterInfo& info : infos)
  {
    if (info.printerName() == printerName)
    {
      printInfo = info;
    }
  }

  QPrinter printer(printInfo);
  printer.setPageMargins(margins);

  QPrintDialog d(&printer, this);

  if ( ! d.exec() )
  {
    return LISTPLUGIN_ERROR;
  }

  QPainter painter(&printer);

  double xscale = double(printer.pageRect().width()) / double(pix.width());
  double yscale = double(printer.pageRect().height()) / double(pix.height());
  double scale = std::min(xscale, yscale);
  painter.translate(printer.paperRect().x() + printer.pageRect().width()/2,
                     printer.paperRect().y() + printer.pageRect().height()/2);
  painter.scale(scale, scale);
  painter.translate(-pix.width()/2, -pix.height()/2);

  painter.drawPixmap(QPoint(0,0), pix);

  return LISTPLUGIN_OK;
}

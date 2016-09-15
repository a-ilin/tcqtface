#include "parentwlxwindow.h"

#include "wlx_interfaces.h"

#include <QApplication>
#include <QLabel>
#include <QResizeEvent>
#include <QSet>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QWindow>
#define QT_WA(unicode, ansi) unicode
#endif


static LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  ParentWlxWindow* tcmdWin = ParentWlxWindow::getByHandle(hWnd);
  if (tcmdWin)
  {
    // choice of WndProc
    WNDPROC proc = tcmdWin->isKeyboardExclusive() ? tcmdWin->origWndProc() :
                                                    tcmdWin->listerWndProc();

    if (WM_KEYDOWN == Msg)
    {
      switch (wParam)
      {
      case VK_F2:
        tcmdWin->reloadWidget();
        break;
      case VK_ESCAPE:
        // set proc to Lister
        proc = tcmdWin->listerWndProc();
        break;
      default:
        break;
      }
    }
    else
    {
      proc = tcmdWin->listerWndProc();
    }

    return CallWindowProc(proc, hWnd, Msg, wParam, lParam);
  }

  return E_FAIL;
}


ParentWlxWindow::ParentWlxWindow(const Interface& keeper, WId hParentWin) :
  QWidget(),
  m_keeper(keeper),
  m_keyboardExclusive(false),
  m_origWndProc((WNDPROC)GetWindowLongPtr((HWND)winId(), GWLP_WNDPROC)),
  m_listerWndProc(NULL),
  m_firstShowTimer(new QTimer(this)),
  m_childWindow(NULL),
  m_childWidget(NULL)
{
  // keep the pointer to this into GWLP_USERDATA
  SetWindowLongPtr((HWND)winId(), GWLP_USERDATA, (LONG_PTR)this);

  setNativeParent(hParentWin);

  setAttribute(Qt::WA_DeleteOnClose, true);
}

ParentWlxWindow::~ParentWlxWindow()
{
  releaseChild();

  // set previous window proc
  WNDPROC proc = m_listerWndProc ? m_listerWndProc : m_origWndProc;
  _assert(proc != WndProc);
  SetWindowLongPtr((HWND)winId(), GWLP_WNDPROC, (LONG_PTR)proc);
  m_listerWndProc = NULL;
  m_origWndProc = NULL;
}

void ParentWlxWindow::setChildWindow(IAbstractWlxWindow* childWindow)
{
  releaseChild();
  _assert( ! m_childWidget );
  _assert( ! m_childWindow );

  if (childWindow)
  {
    m_childWidget = childWindow->widget();
    _assert(m_childWidget);
    if (m_childWidget)
    {
      _assert(m_childWidget->parent() == this);

      m_childWindow = childWindow;
      _log("Window is embedded");
    }
    else
    {
      m_childWidget = new QLabel(QString("Cannot cast IAbstractWlxWindow* to QWidget*"));
      _log("Window is NOT embedded. Cannot cast to QWidget*.");
    }

    m_childWidget->resize(size());
  }
}

ParentWlxWindow* ParentWlxWindow::getByHandle(HWND hwnd)
{
  QWidget* p = (QWidget*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  return qobject_cast<ParentWlxWindow*> (p);
}

void ParentWlxWindow::reloadWidget()
{
  _assert(m_childWindow);
  if (m_childWindow)
  {
    m_childWindow->reload();
  }
}

void ParentWlxWindow::releaseChild()
{
  if (m_childWidget)
  {
    bool closed = m_childWidget->close();
    _assert(closed);

    m_childWidget->deleteLater();
    m_childWidget = NULL;
  }

  if (m_childWindow)
  {
    _log(QString("Child window destroyed: ") + QString::number((quint64)m_childWindow, 16));
    m_childWindow = NULL;
  }
}

void ParentWlxWindow::setNativeParent(WId hParentWin)
{
  // make the widget window style be WS_CHILD so SetParent will work
  SetWindowLongPtr((HWND)winId(), GWL_STYLE,
                   (hParentWin ? WS_CHILD : WS_POPUP) |
                   WS_CLIPCHILDREN |
                   WS_CLIPSIBLINGS  /*|
                   WS_EX_NOPARENTNOTIFY*/);

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  QWindow* window = windowHandle();
  window->setProperty("_q_embedded_native_parent_handle", hParentWin ? (WId)hParentWin : QVariant());
  SetParent((HWND)winId(), (HWND)hParentWin);
  window->setFlags(Qt::FramelessWindowHint);
#else
  SetParent(winId(), (HWND)hParentWin);
#endif

  QEvent e(QEvent::EmbeddingControl);
  QApplication::sendEvent(this, &e);
}

WId ParentWlxWindow::nativeParent() const
{
  return (WId)GetAncestor((HWND)winId(), GA_PARENT);
}

void ParentWlxWindow::showEvent(QShowEvent* e)
{
  QWidget::showEvent(e);

  connect(m_firstShowTimer, &QTimer::timeout, this, &ParentWlxWindow::onFirstShowTimer);
  m_firstShowTimer->start(200);
}

void ParentWlxWindow::resizeEvent(QResizeEvent* e)
{
  QWidget::resizeEvent(e);

  if (m_childWidget)
  {
    m_childWidget->resize(e->size());
  }
}

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
bool ParentWlxWindow::nativeEvent(const QByteArray&, void* message, long* /*result*/)
#else
bool ParentWlxWindow::winEvent(MSG* msg, long* /*result*/)
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  MSG *msg = (MSG *)message;
#endif
  if (msg->message == WM_DESTROY)
  {
    close();
  }

  return false;
}

void ParentWlxWindow::onFirstShowTimer()
{
  WNDPROC proc = (WNDPROC)GetWindowLongPtr((HWND)winId(), GWLP_WNDPROC);
  if ( ( proc != m_origWndProc ) && ( ! m_listerWndProc ) )
  {
    m_listerWndProc = proc;

    // replace default window procedure
    SetWindowLongPtr((HWND)winId(), GWLP_WNDPROC, (LONG_PTR)WndProc);

    m_firstShowTimer->stop();
  }
}

void ParentWlxWindow::setListerOptions(int itemtype, int value) const
{
  PostMessage((HWND)nativeParent(), WM_COMMAND, MAKELONG(value, itemtype),(LPARAM)winId());
}

QString ParentWlxWindow::listerTitle() const
{
  if (HWND hParent = (HWND)nativeParent())
  {
    QVector<TCHAR> wStrTitle(GetWindowTextLength(hParent) + 1);
    GetWindowText(hParent, wStrTitle.data(), wStrTitle.size());
    QString title = QString::fromWCharArray(wStrTitle.constData());
    return title;
  }

  _assert_ex(false, "Parent handle is not accessible");
  return QString();
}

void ParentWlxWindow::setListerTitle(const QString& title)
{
  if (HWND hParent = (HWND)nativeParent())
  {
    QVector<TCHAR> wStrTitle(title.size() + 1);
    title.toWCharArray(wStrTitle.data());
    SetWindowText(hParent, wStrTitle.constData());
  }
  else
  {
    _assert_ex(false, "Parent handle is not accessible");
  }
}

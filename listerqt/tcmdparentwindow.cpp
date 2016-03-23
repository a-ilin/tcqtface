#include "tcmdparentwindow.h"

#include "core.h"
#include "listplug_qt_iface.h"

#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QTimer>
#include <QVBoxLayout>

#if QT_VERSION >= 0x050000
#include <QWindow>
#define QT_WA(unicode, ansi) unicode
#endif

static LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  TCmdParentWindow* tcmdWin = TCmdParentWindow::getByHandle(hWnd);
  if (tcmdWin)
  {
    // choice of WndProc
    WNDPROC proc = tcmdWin->isKeyboardExclusive() ? tcmdWin->origWndProc() :
                                                    tcmdWin->listerWndProc();

    QSet<UINT> keyEvents;
    keyEvents.insert(WM_KEYDOWN);
    keyEvents.insert(WM_KEYUP);
    keyEvents.insert(WM_DEADCHAR);
    keyEvents.insert(WM_SYSDEADCHAR);
    keyEvents.insert(WM_CHAR);
    keyEvents.insert(WM_UNICHAR);
    keyEvents.insert(WM_HOTKEY);

    if (keyEvents.contains(Msg))
    {
      // reload
      switch(wParam)
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
      // if Msg is not WM_KEYUP then proc should be ListerWndProc
      proc = tcmdWin->listerWndProc();
    }

    return CallWindowProc(proc, hWnd, Msg, wParam, lParam);
  }

  return E_FAIL;
}

// returns S_OK for all events
static LRESULT CALLBACK DummyWndProc(HWND /*hWnd*/, UINT /*Msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
  return S_OK;
}

TCmdParentWindow::TCmdParentWindow(const InterfaceKeeper& keeper, WId hParentWin) :
  QWidget(),
  m_keeper(keeper),
  m_keyboardExclusive(false),
  m_origWndProc((WNDPROC)GetWindowLongPtr((HWND)winId(), GWLP_WNDPROC)),
  m_listerWndProc(NULL),
  m_firstShowTimer(new QTimer(this)),
  m_childWindow(NULL)
{
  // keep the pointer to this into GWLP_USERDATA
  SetWindowLongPtr((HWND)winId(), GWLP_USERDATA, (LONG_PTR)this);

  setNativeParent(hParentWin);

  setAttribute(Qt::WA_DeleteOnClose, true);

  // layout
  QVBoxLayout* lay = new QVBoxLayout(this);
  lay->setContentsMargins(0,0,0,0);
  lay->setSpacing(0);
  setLayout(lay);
}

TCmdParentWindow::~TCmdParentWindow()
{
  releaseChild();

  // set previous window proc
  WNDPROC proc = m_listerWndProc ? m_listerWndProc : m_origWndProc;
  _assert(proc != WndProc);
  SetWindowLongPtr((HWND)winId(), GWLP_WNDPROC, (LONG_PTR)proc);
  m_listerWndProc = NULL;
  m_origWndProc = NULL;
}

void TCmdParentWindow::setChildWindow(TCmdChildWindow* childWindow)
{
  releaseChild();

  m_childWindow = childWindow;
  QWidget* w = m_childWindow->widget();

  layout()->addWidget(w);

  connect(w, SIGNAL(setKeyboardExclusive(bool)),
          this, SLOT(onSetKeyboardExclusive(bool)));
  connect(w, SIGNAL(setListerOptions(int,int)),
          this, SLOT(onSetListerOptions(int,int)));

  m_childWindow->initEmbedded();
}

TCmdParentWindow* TCmdParentWindow::getByHandle(HWND hwnd)
{
   QWidget* p = (QWidget*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
   TCmdParentWindow* ptr = NULL;
   try
   {
     ptr = p ? qobject_cast<TCmdParentWindow*> (p) : NULL;
   }
   catch(...)
   {
     _assert_ex(false, "Exception on Parent ptr cast");
   }

   return ptr;
}

void TCmdParentWindow::reloadWidget()
{
  _assert(m_childWindow);
  if (m_childWindow)
  {
    m_childWindow->reload();
  }
}

void TCmdParentWindow::releaseChild()
{
  // clear layout
  while(layout()->count())
  {
    layout()->takeAt(0);
  }

  delete m_childWindow;
  _log(QString("Child window destroyed: ") + QString::number((quint64)m_childWindow, 16));
  m_childWindow = NULL;
}

void TCmdParentWindow::setNativeParent(WId hParentWin)
{
  // make the widget window style be WS_CHILD so SetParent will work
  SetWindowLongPtr((HWND)winId(), GWL_STYLE,
                   (hParentWin ? WS_CHILD : WS_POPUP) |
                   WS_CLIPCHILDREN |
                   WS_CLIPSIBLINGS  /*|
                   WS_EX_NOPARENTNOTIFY*/);

#if QT_VERSION >= 0x050000
  QWindow *window = windowHandle();
  window->setProperty("_q_embedded_native_parent_handle", hParentWin ? (WId)hParentWin : QVariant());
  SetParent((HWND)winId(), (HWND)hParentWin);
  window->setFlags(Qt::FramelessWindowHint);
#else
  SetParent(winId(), (HWND)hParentWin);
#endif

  QEvent e(QEvent::EmbeddingControl);
  QApplication::sendEvent(this, &e);
}

WId TCmdParentWindow::nativeParent() const
{
  return (WId)GetAncestor((HWND)winId(), GA_PARENT);
}

void TCmdParentWindow::showEvent(QShowEvent* e)
{
  QWidget::showEvent(e);

  connect(m_firstShowTimer, &QTimer::timeout, this, &TCmdParentWindow::onFirstShowTimer);
  m_firstShowTimer->start(200);
}

#if QT_VERSION >= 0x050000
bool TCmdParentWindow::nativeEvent(const QByteArray &, void *message, long * /*result*/)
#else
bool MainWindow::winEvent(MSG *msg, long * result)
#endif
{
#if QT_VERSION >= 0x050000
  MSG *msg = (MSG *)message;
#endif
  if (msg->message == WM_DESTROY)
  {
    close();
  }

  return false;
}

void TCmdParentWindow::onFirstShowTimer()
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

void TCmdParentWindow::onSetListerOptions(int itemtype, int value)
{
  PostMessage((HWND)nativeParent(), WM_COMMAND, MAKELONG(value, itemtype),(LPARAM)winId());
}

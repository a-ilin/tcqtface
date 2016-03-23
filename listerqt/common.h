#ifndef COMMON_H
#define COMMON_H

#ifdef UNICODE
#define _UNICODE
#endif

#include <qt_windows.h>
#include <tchar.h>

#include <string>

#include <QSettings>
#include <QString>

// filename without path
#define __FILENAME__(x) (strrchr(x, '\\') ? (strrchr(x, '\\') + 1) : (x))

// string conversion
#define TOSTRING2(x) #x
#define TOSTRING(x) TOSTRING2(x)

static inline
QByteArray _TQ_helper(const QString& str) {
  QByteArray result;
#ifdef UNICODE
  result.fill(0, (str.size() + 1) * sizeof(TCHAR));
  str.toWCharArray((TCHAR*)result.data());
#else
  result = str.toLocal8Bit();
#endif
  return result;
}
#define _TQ(x) (TCHAR*)_TQ_helper(QString(x)).constData()

static inline QString _toString(const char* s) { return QString(s); }
static inline QString _toString(const wchar_t* s) { return QString::fromWCharArray(s); }


// logging
enum LogFacility
{
  LogNone      = 0,  // no logging
  LogCritical  = 1,  // log assert failures and show messageboxes
  LogDebug     = 2   // log all info
};
QString _log_string_helper(const QString& msg, const QString& file,
                    const QString& function, const QString& line,
                    bool timeStamp);
#define _log_string(msg, timeStamp) \
  _log_string_helper(QString(msg), QString(__FILE__), \
                     QString(__FUNCTION__), QString(TOSTRING(__LINE__)), timeStamp)

void _log_ex_helper(const QString& str, int facility);

#define _log_ex(msg, facility) _log_ex_helper(_log_string(msg, true), facility)
#define _log(msg) _log_ex(msg, LogDebug)
#define _log_line _log(QString())


// native MessageBox
template <typename T1, typename T2>
void _messagebox_ex(T1 msg, T2 title, DWORD flags) {
  MessageBox(NULL, _TQ(msg), _TQ(title), flags);
}
#define _messagebox(msg) _messagebox_ex(msg, "MSG", MB_ICONINFORMATION | MB_OK)

// assertion
void _assert_ex_helper(const QString& str);
#define _assert_ex(expr, msg) \
  if (0 == (expr)) { \
    _log_ex(QString("ASSERT: ") + QString(msg), LogCritical); \
    _assert_ex_helper(QString(msg)); \
  }

#define _assert(expr) _assert_ex( expr, #expr )
// Main Switch
extern bool GlobalError;
#define CHECK_GLOBAL_ERROR(...) if ( GlobalError ) { _log("GlobalError is set."); return __VA_ARGS__; }

// settings
class AppSet : public QSettings
{
public:
  AppSet();
};


// semaphore control
#define LockSemaphoreEx(hSem, timeMs) \
  WaitForSingleObject(hSem, timeMs)

#define LockSemaphore(hSem) \
  LockSemaphoreEx(hSem, INFINITE)

#define UnlockSemaphoreEx(hSem, iCount, pPrevCount) \
  BOOL res = ReleaseSemaphore(hSem, iCount, pPrevCount); \
  _assert(res);

#define UnlockSemaphore(hSem) \
  UnlockSemaphoreEx(hSem, 1, NULL);

#define WaitForSemaphore(hSem)\
  LockSemaphore(hSem); \
  UnlockSemaphore(hSem);

// NOTE: non-thread safe because of racing!
inline
int GetSemaphoreCount(HANDLE hSem)
{
  long lPrev = 0;
  UnlockSemaphoreEx(hSem, 1, &lPrev);
  LockSemaphore(hSem);
  return lPrev;
}

#endif // COMMON_H


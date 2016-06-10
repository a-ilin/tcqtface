#ifndef COMMON_H
#define COMMON_H

#ifdef UNICODE
#define _UNICODE
#endif

#include <qt_windows.h>
#include <tchar.h>

#include <string>

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
QByteArray _log_string_helper(const QString& msg, const char* file,
                    const char* function, const char* line,
                    bool bTimeStamp, int facility);
#define _log_string(msg, bTimeStamp, facility) \
  _log_string_helper(msg, __FILE__, \
                     __FUNCTION__, TOSTRING(__LINE__), bTimeStamp, facility)

void _log_ex_helper(const QByteArray& buf, int facility);

#define _log_ex(msg, facility) _log_ex_helper(_log_string(msg, true, facility), facility)
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

// settings
class AppSet
{
public:
  static QString readString(const QString& key, const QString& defValue = QString());
  static uint readInt(const QString& key, int defValue = -1);
};


#endif // COMMON_H

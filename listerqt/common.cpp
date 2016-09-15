#include "common.h"

#include "atomicmutex.h"

#include <QByteArray>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>

#include "listplug.h"

// log time performance statistics
static QElapsedTimer g_perfTimer;
// log mutex
static AtomicMutex g_logMutex;
// log file name
static QString g_logFile;
// temporary log buffer
static QList<QPair<int, QByteArray> > g_logBuffer;
// max log facility
static int g_logFacility = LogDebug;
// show message box on assert
static bool g_msgOnAssert = true;
// settings file name
static QString g_setFileName;
// default params struct
static ListDefaultParamStruct g_defaultParams;
ListDefaultParamStruct* g_pDefaultParams = &g_defaultParams;

// create a single log string from multiple components
QByteArray _log_string_helper(const QString& msg, const char* file,
                    const char* function, const char* line,
                    bool bTimeStamp, int facility)
{
  if (facility > g_logFacility)
  { // skip message
    return QByteArray();
  }

  QString datetime;
  if (bTimeStamp)
  {
    if ( ! g_perfTimer.isValid() )
    {
      g_perfTimer.start();
    }

    datetime += QString::number(g_perfTimer.nsecsElapsed() / 1000).rightJustified(9);
    datetime += QDateTime::currentDateTime().toString(" yyyy-MM-dd HH:mm:ss.zzz ");
  }

  QString fileName = QFileInfo(file).fileName();

  QString result = QString(datetime +
                           fileName.leftJustified(20) +
                           QChar(':') + QString(line).rightJustified(4) +
                           QChar(' ') +
                           QString(function).leftJustified(30) +
                           QString(":") + msg + QChar('\n'));

  return result.toLocal8Bit();
}

// write to log
void _log_ex_helper(const QByteArray& buf, int facility)
{
  if ( buf.isEmpty() || (facility > g_logFacility) )
  { // nothing to do
    return;
  }

  AtomicLocker locker(&g_logMutex);

  if ( ! g_logFile.isEmpty() )
  {
    FILE* fd = fopen(g_logFile.toLatin1().constData(), "at");
    if (fd)
    {
      fwrite(buf.constData(), buf.size(), 1, fd);
      fclose(fd);
    }
  }
  else
  {
    g_logBuffer.append(qMakePair(facility, buf));
  }
}

void _set_default_params(ListDefaultParamStruct* dps)
{
  static bool isSet = false;
  if (isSet || !dps)
  {
    return;
  }

  if (dps->size < sizeof(ListDefaultParamStruct))
  { // old TC version?
    _log_ex("dps->size < sizeof(ListDefaultParamStruct)", LogCritical);
    return;
  }

  isSet = true;

  g_setFileName = dps->DefaultIniName;
  _log(g_setFileName);

  g_logFacility = AppSet::readInt("LogFacility", LogNone);
  g_logFile = AppSet::readString("LogFile", QString());
  g_msgOnAssert = AppSet::readInt("MsgOnAssert", 0);

  if (g_logFile.isEmpty() || (g_logFacility == LogNone) )
  { // switch off logging if no filename had given
    g_logFacility = LogNone;
  }
  else
  { // flush the log buffer
    QList<QPair<int, QByteArray> > logBuffer;
    std::swap(logBuffer, g_logBuffer);
    for (const QPair<int, QByteArray>& msg : logBuffer)
    {
      _log_ex_helper(msg.second, msg.first);
    }
  }
  g_logBuffer.clear();

  memcpy(&g_defaultParams, dps, sizeof(ListDefaultParamStruct));
}

void _assert_ex_helper(const QString& str)
{
  if (g_msgOnAssert && (g_logFacility >= LogCritical))
  {
    _messagebox_ex(str, "tcqtface ASSERTION FAILED!", MB_ICONERROR | MB_OK);
  }
}

QString AppSet::readString(const QString& key, const QString& defValue)
{
  const int maxSize = 255;
  QString result(maxSize, '\0');
  int sz = GetPrivateProfileString(L"qtface", (LPCWSTR)key.utf16(), (LPCWSTR)defValue.utf16(),
                         (LPWSTR)result.data(), maxSize, (LPCWSTR)g_setFileName.utf16());
  result.resize(sz);
  return result;
}

uint AppSet::readInt(const QString& key, int defValue)
{
  return GetPrivateProfileInt(L"qtface", (LPCWSTR)key.utf16(), defValue, (LPCWSTR)g_setFileName.utf16());
}

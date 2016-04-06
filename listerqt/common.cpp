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
static QByteArray g_logBuffer;
// max log facility
static int g_logFacility = LogDebug;
// show message box on assert
static bool g_msgOnAssert = true;
// settings file name
static QString g_setFileName;

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
void _log_ex_helper(const QByteArray& buf)
{
  if (buf.isEmpty())
  { // nothing to do
    return;
  }

  AtomicLocker locker(&g_logMutex);

  if ( ! g_logFile.isEmpty() )
  {
    QFile f;
    f.setFileName(g_logFile);
    if (f.open(QIODevice::Append | QIODevice::Text))
    {
      f.write(buf);
    }
  }
  else
  {
    g_logBuffer.append(buf);
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

  AppSet set;
  g_logFacility = set.value("LogFacility", LogNone).toInt();
  g_logFile = set.value("LogFile", QString()).toString();
  g_msgOnAssert = set.value("MsgOnAssert", false).toBool();

  if (g_logFile.isEmpty() || (g_logFacility == LogNone) )
  { // switch off logging if no filename had given
    g_logFacility = LogNone;
  }
  else
  { // flush the log buffer
    _log_ex_helper(g_logBuffer);
  }
  g_logBuffer.clear();
}


AppSet::AppSet() :
  QSettings(g_setFileName, QSettings::IniFormat)
{
  beginGroup("qtface");
}

void _assert_ex_helper(const QString& str)
{
  if (g_logFacility >= LogCritical)
  {
    _messagebox_ex(str, "tcqtface ASSERTION FAILED!", MB_ICONERROR | MB_OK);
  }
}

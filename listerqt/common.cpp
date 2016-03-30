#include "common.h"

#include "atomicmutex.h"

#include <QDateTime>
#include <QFileInfo>

#include "libraryloader.h"
#include "listplug.h"

// global plugin error
bool GlobalError = false;

static AtomicMutex g_logMutex;
static const QString g_logFile = LibraryLoader::dirByPath(LibraryLoader::pathThis()) +
                                 QChar('\\') + QString("listplug_dbg.log");

// show message box on assert
static bool g_msgOnAssert = true;

static int g_logFacility = LogDebug;
static QString g_setFileName;


QString _log_string_helper(const QString& msg, const QString& file,
                    const QString& function, const QString& line,
                    bool timeStamp)
{
  QString datetime = timeStamp ?
                       QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz ") :
                       QString();

  QString fileName = QFileInfo(file).fileName();

  QString result = QString(datetime +
                           fileName.leftJustified(20) +
                           QChar(':') + line.rightJustified(4) +
                           QChar(' ') +
                           function.leftJustified(30) +
                           QString(":") + msg + QChar('\n'));

  return result;
}

void _log_ex_helper(const QString& str, int facility)
{
  if (facility > g_logFacility)
  { // skip message
    return;
  }

  AtomicLocker locker(&g_logMutex);

  FILE* dbgFile = NULL;
  errno_t err = fopen_s(&dbgFile, g_logFile.toLocal8Bit().constData(), "at");
  if (err == S_OK)
  {
    QByteArray loc8bit = str.toLocal8Bit();
    fputs(loc8bit.constData(), dbgFile);
    fclose(dbgFile);
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
    _log("dps->size < sizeof(ListDefaultParamStruct)");
    return;
  }

  isSet = true;

  g_setFileName = dps->DefaultIniName;
  _log(g_setFileName);

  AppSet set;
  g_logFacility = set.value("LogFacility", LogCritical).toInt();
  g_msgOnAssert = set.value("MsgOnAssert", true).toBool();
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
    _messagebox_ex(str, "ASSERTION FAILED!", MB_ICONERROR | MB_OK);
  }
}

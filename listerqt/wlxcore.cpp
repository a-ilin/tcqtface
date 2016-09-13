#include "wlxcore.h"

#include "common.h"
#include "libraryloader.h"

QString WlxCore::pathModuleByAddr(void* addr) const
{
    Library lib = Loader::handle(addr, true);
    return Loader::pathModule(lib.get());
}

QString WlxCore::dirByPath(const QString& path) const
{
    return Loader::dirByPath(path);
}

void WlxCore::__log(const QByteArray& buf, int facility)
{
    _log_ex_helper(buf, facility);
}

QByteArray WlxCore::__log_string(const QString& msg, const char* file, const char* function, const char* line, bool bTimeStamp, int facility)
{
    return _log_string_helper(msg, file, function, line, bTimeStamp, facility);
}

void WlxCore::__assert(const QString& msg)
{
    _assert_ex_helper(msg);
}


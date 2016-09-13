#ifndef WLXCORE_H
#define WLXCORE_H

#include "wlx_interfaces.h"

class WlxCore : public IWlxCore
{
public:
    WlxCore()
      : IWlxCore() {}

    QString pathModuleByAddr(void* addr) const;
    QString dirByPath(const QString& path) const;

    void __log(const QByteArray& buf, int facility);
    QByteArray __log_string(const QString& msg,
                            const char* file, const char* function, const char* line,
                            bool bTimeStamp, int facility);
    void __assert(const QString& msg);
};

#endif // WLXCORE_H

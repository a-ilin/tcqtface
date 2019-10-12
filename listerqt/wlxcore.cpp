/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 by Aleksei Ilin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

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


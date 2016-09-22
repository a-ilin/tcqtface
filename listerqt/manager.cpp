/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 by Aleksei Ilin
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

#include "manager.h"

#include "listplug.h"
#include "wlx_interfaces.h"

extern ListDefaultParamStruct* g_pDefaultParams;

Manager::Manager(void* retAddr)
    : m_pLoader(Loader::i())
    , m_pModule(Loader::handle(retAddr))
    , m_pCore(Core::i())
{
    _assert(m_pModule);
}

Manager::~Manager()
{
    destroyCore();
    destroyLoader();
}

Interface Manager::iface()
{
    bool created = false;
    Interface iface = m_pLoader->iface(m_pModule, &created);

    if (created)
    {
        iface->initCore(core()->wlxCore());
        iface->setDefaultParams(g_pDefaultParams);
    }

    return iface;
}

void Manager::destroyCore()
{
    m_pCore.reset();
    CoreLocker coreLocker;
    Core::destroy();
}

void Manager::destroyLoader()
{
    m_pLoader.reset();
    LoaderLocker loaderLocker;
    Loader::destroy();
}

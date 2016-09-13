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

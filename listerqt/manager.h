#ifndef MANAGER_H
#define MANAGER_H

#include "common.h"
#include "core.h"
#include "libraryloader.h"

class Manager
{
public:
  Manager(void* retAddr)
    : m_pLoader(Loader::i())
    , m_pModule(Loader::handle(retAddr))
    , m_pCore(Core::i())
  {
    _assert(m_pModule);
  }

  ~Manager()
  {
    destroyCore();
    destroyLoader();
  }

  std::shared_ptr<Loader>& loader() { return m_pLoader; }
  Interface iface() { return m_pLoader->iface(m_pModule); }

  std::shared_ptr<Core>& core() { return m_pCore; }

private:
  void destroyCore()
  {
    m_pCore.reset();
    CoreLocker coreLocker;
    Core::destroy();
  }

  void destroyLoader()
  {
    m_pLoader.reset();
    LoaderLocker loaderLocker;
    Loader::destroy();
  }

private:
  std::shared_ptr<Loader> m_pLoader;
  Library m_pModule;
  std::shared_ptr<Core> m_pCore;
};

#endif // MANAGER_H

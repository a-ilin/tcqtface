#ifndef MANAGER_H
#define MANAGER_H

#include "common.h"
#include "core.h"
#include "libraryloader.h"

class Manager
{
public:
  Manager(void* retAddr);
  ~Manager();

  std::shared_ptr<Loader>& loader() { return m_pLoader; }
  Interface iface();
  std::shared_ptr<Core>& core() { return m_pCore; }

private:
  void destroyCore();
  void destroyLoader();

private:
  std::shared_ptr<Loader> m_pLoader;
  Library m_pModule;
  std::shared_ptr<Core> m_pCore;
};

#endif // MANAGER_H

#include "libraryloader.h"

#include <QFileInfo>

#include "atomicmutex.h"
#include "common.h"
#include "wlx_interfaces.h"

// plugin loader
std::shared_ptr<Loader> g_pLoader;
AtomicMutex g_loaderMutex;

class LibraryMap
{
public:
  void add(const Interface& pWlx,
           const Library& pModule)
  {
    _log(QString("Library acquired: %1").arg(Loader::pathModule(pModule.get())));
    _assert(pWlx && pModule);
    _assert(m_wlxToModule.find(pWlx) == m_wlxToModule.cend());
    _assert(m_moduleToWlx.find(pModule) == m_moduleToWlx.cend());
    m_wlxToModule[pWlx] = pModule;
    m_moduleToWlx[pModule] = pWlx;
  }

  void remove(const Interface& pWlx)
  {
    auto it = m_wlxToModule.find(pWlx);
    _assert(it != m_wlxToModule.cend());
    if (it != m_wlxToModule.cend())
    {
      _log(QString("Library released: %1").arg(Loader::pathModule((*it).second.get())));
      size_t count = m_moduleToWlx.erase((*it).second);
      _assert(count == 1);
      m_wlxToModule.erase(it);
    }
  }

  Interface wlx(const Library& pModule) const
  {
    Interface pWlx;
    auto it = m_moduleToWlx.find(pModule);
    if (it != m_moduleToWlx.cend())
    {
      pWlx = (*it).second;
    }
    return pWlx;
  }

  Library module(const Interface& pWlx) const
  {
    Library pModule;
    auto it = m_wlxToModule.find(pWlx);
    if (it != m_wlxToModule.cend())
    {
      pModule = (*it).second;
    }
    return pModule;
  }

  void simplify()
  {
    _assert(m_wlxToModule.size() == m_moduleToWlx.size());

    for (auto it = m_wlxToModule.begin(); it != m_wlxToModule.end();)
    {
      if ((*it).first.use_count() == 2)
      {
        it = m_wlxToModule.erase(it);
      }
      else
      {
        ++it;
      }
    }

    for (auto it = m_moduleToWlx.begin(); it != m_moduleToWlx.end();)
    {
      if ((*it).second.use_count() == 1)
      {
        it = m_moduleToWlx.erase(it);
      }
      else
      {
        ++it;
      }
    }

    _assert(m_wlxToModule.size() == m_moduleToWlx.size());
  }

  bool isEmpty() const { return m_wlxToModule.empty(); }
  bool contains(const std::shared_ptr<void>& pModule) const { return m_moduleToWlx.find(pModule) != m_moduleToWlx.cend(); }

private:
  std::map<Interface, Library> m_wlxToModule;
  std::map<Library, Interface> m_moduleToWlx;
};


class LoaderPrivate
{
public:
  mutable AtomicMutex mapMutex;
  LibraryMap map;
};


std::shared_ptr<Loader> Loader::i()
{
  LoaderLocker locker;
  if ( ! g_pLoader )
  {
    new Loader();
  }

  _assert(g_pLoader);
  return g_pLoader;
}

bool Loader::isExists()
{
  _assert(g_loaderMutex.isLocked());
  return static_cast<bool>(g_pLoader);
}

bool Loader::destroy()
{
  _assert(g_loaderMutex.isLocked());
  _assert(g_pLoader);

  LoaderPrivate* d = g_pLoader->d_func();

  if (g_pLoader.use_count() == 1)
  {
    d->map.simplify();
    if (d->map.isEmpty())
    {
      g_pLoader.reset();
      return true;
    }
  }

  return false;
}

QString Loader::pathModule(void* hModule)
{
  TCHAR strPath[MAX_PATH + 1];
  memset(strPath, 0, sizeof(strPath));

  _assert(hModule);
  if (hModule)
  {
    GetModuleFileName((HMODULE)hModule, strPath, sizeof(strPath));
  }

  return _toString(strPath);
}

QString Loader::pathThis()
{
  return pathModule(handleThis(true).get());
}

Loader::Loader() :
  d_ptr(new LoaderPrivate())
{
  _assert(g_loaderMutex.isLocked());
  _assert( ! g_pLoader );
  g_pLoader.reset(this, [](Loader* pLoader)
  {
    delete pLoader;
  });
  _log("LibraryLoader created");
}

Loader::~Loader()
{
  delete d_ptr;
  _log("LibraryLoader destroyed");
}

Interface Loader::iface(Library& pModule)
{
  Q_D(Loader);

  _assert(pModule);

#ifdef CORE_STATICLIB
  _assert(pModule == handleThis(true));
#else
  _assert(pModule != handleThis(true));
#endif

  if ( ! pModule )
  {
    return Interface();
  }

  AtomicLocker mapLocker(&d->mapMutex);
  Interface pWlx(d->map.wlx(pModule));

  if ( ! pWlx )
  { // doesn't exist, need create
    GetWlxPluginFunc pFunc = (GetWlxPluginFunc)GetProcAddress((HMODULE)pModule.get(), "GetWlxPlugin");
    _assert(pFunc);
    if ( ! pFunc )
    {
      return Interface();
    }

    pWlx.reset(pFunc());

    _assert(pWlx);
    if ( ! pWlx )
    {
      return Interface();
    }

    d->map.add(pWlx, pModule);

    const char* qtVersion = (const char*)GetProcAddress((HMODULE)pModule.get(), "_qt_version");
    _assert(qtVersion);
    _log(QString("Qt version: ") + QString(qtVersion + sizeof("_qt_version=") - 1));
  }

  return pWlx;
}

bool Loader::containsLibrary(void* addr) const
{
  Q_D(const Loader);
  AtomicLocker mapLocker(&d->mapMutex);
  return d->map.contains(handle(addr, true));
}

QString Loader::dirByPath(const QString& path)
{
  return QFileInfo(path).canonicalPath().replace('/', '\\');
}

Library Loader::handle(void* addr, bool noref)
{
  HMODULE hModule = NULL;

  if ( ! GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           (noref ? GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT : 0),
                           (TCHAR*) addr,
                           &hModule))
  {
    int ret = GetLastError();
    _assert_ex(false, QString("GetModuleHandle returned error: 0x%1")
                        .arg(QString::number(ret, 16)));
  }

  if (noref)
  {
    return Library(hModule, [](void*){});
  }

  return Library(hModule, [](void* ptr){FreeLibrary((HMODULE)ptr);});
}

Library Loader::handleThis(bool noref)
{
  volatile static TCHAR* localVar = (TCHAR*)0x12345;
  return handle(&localVar, noref);
}

Library Loader::handlePath(const QString& path)
{
  HMODULE hDll = LoadLibraryEx(_TQ(path), NULL,
                               LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
                               LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);

  return Library(hDll, [](void* ptr){FreeLibrary((HMODULE)ptr);});
}

void Loader::lock()
{
  g_loaderMutex.lock();
}

void Loader::unlock()
{
  _assert(g_loaderMutex.isLocked());
  g_loaderMutex.unlock();
}

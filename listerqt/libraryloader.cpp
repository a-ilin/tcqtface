#include "libraryloader.h"

#include <QFileInfo>

#include "common.h"
#include "wlx_interfaces.h"

// plugin loader
std::unique_ptr<LibraryLoader> g_pLibraryLoader;

class LibraryMap
{
public:
  void add(const std::shared_ptr<IAbstractWlxPlugin>& pWlx,
           const std::shared_ptr<void>& pModule)
  {
    _log(QString("Library acquired: %1").arg(LibraryLoader::pathModule(pModule.get())));
    _assert(pWlx && pModule);
    _assert(m_wlxToModule.find(pWlx) == m_wlxToModule.cend());
    _assert(m_moduleToWlx.find(pModule) == m_moduleToWlx.cend());
    m_wlxToModule[pWlx] = pModule;
    m_moduleToWlx[pModule] = pWlx;
  }

  void remove(const std::shared_ptr<IAbstractWlxPlugin>& pWlx)
  {
    auto it = m_wlxToModule.find(pWlx);
    _assert(it != m_wlxToModule.cend());
    if (it != m_wlxToModule.cend())
    {
      _log(QString("Library released: %1").arg(LibraryLoader::pathModule((*it).second.get())));
      size_t count = m_moduleToWlx.erase((*it).second);
      _assert(count == 1);
      m_wlxToModule.erase(it);
    }
  }

  std::shared_ptr<IAbstractWlxPlugin> wlx(const std::shared_ptr<void>& pModule) const
  {
    std::shared_ptr<IAbstractWlxPlugin> pWlx;
    auto it = m_moduleToWlx.find(pModule);
    if (it != m_moduleToWlx.cend())
    {
      pWlx = (*it).second;
    }
    return pWlx;
  }

  std::shared_ptr<void> module(const std::shared_ptr<IAbstractWlxPlugin>& pWlx) const
  {
    std::shared_ptr<void> pModule;
    auto it = m_wlxToModule.find(pWlx);
    if (it != m_wlxToModule.cend())
    {
      pModule = (*it).second;
    }
    return pModule;
  }

  bool isEmpty() const { return m_wlxToModule.empty(); }
  bool contains(const std::shared_ptr<void>& pModule) const { return m_moduleToWlx.find(pModule) != m_moduleToWlx.cend(); }

private:
  std::map<std::shared_ptr<IAbstractWlxPlugin>, std::shared_ptr<void> > m_wlxToModule;
  std::map<std::shared_ptr<void>, std::shared_ptr<IAbstractWlxPlugin> > m_moduleToWlx;
};


class LibraryLoaderPrivate
{
public:
  LibraryMap map;
};


LibraryLoader& LibraryLoader::i()
{
  if ( ! g_pLibraryLoader )
  {
    g_pLibraryLoader.reset(new LibraryLoader());
  }

  return *g_pLibraryLoader;
}

bool LibraryLoader::isExists()
{
  return static_cast<bool>(g_pLibraryLoader);
}

QString LibraryLoader::pathModule(void* hModule)
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

QString LibraryLoader::pathThis()
{
  return pathModule(handleThis(true).get());
}

LibraryLoader::LibraryLoader() :
  d_ptr(new LibraryLoaderPrivate())
{
  _log("LibraryLoader created");
}

LibraryLoader::~LibraryLoader()
{
  delete d_ptr;
  g_pLibraryLoader.release();

  _log("LibraryLoader destroyed");
}

InterfaceKeeper LibraryLoader::keeper(void* addr)
{
  Q_D(LibraryLoader);

  std::shared_ptr<void> pModule(handle(addr));
  _assert(pModule);

  // wrong addr passed
  _assert(pModule != handleThis(true));

  if ( ! pModule )
  {
    return InterfaceKeeper();
  }

  std::shared_ptr<IAbstractWlxPlugin> pWlx(d->map.wlx(pModule));

  if ( ! pWlx )
  { // doesn't exist, need create
    GetWlxPluginFunc pFunc = (GetWlxPluginFunc)GetProcAddress((HMODULE)pModule.get(), "GetWlxPlugin");
    _assert(pFunc);
    if ( ! pFunc )
    {
      return InterfaceKeeper();
    }

    pWlx.reset(pFunc());

    _assert(pWlx);
    if ( ! pWlx )
    {
      return InterfaceKeeper();
    }

    d->map.add(pWlx, pModule);

    const char* qtVersion = (const char*)GetProcAddress((HMODULE)pModule.get(), "_qt_version");
    _assert(qtVersion);
    _log(QString("Qt version: ") + QString(qtVersion + sizeof("_qt_version=") - 1));
  }

  return InterfaceKeeper(pWlx);
}

bool LibraryLoader::containsLibrary(void* addr) const
{
  Q_D(const LibraryLoader);
  return d->map.contains(handle(addr, true));
}

bool LibraryLoader::isEmpty() const
{
  Q_D(const LibraryLoader);
  return d->map.isEmpty();
}

QString LibraryLoader::dirByPath(const QString& path)
{
  return QFileInfo(path).canonicalPath().replace('/', '\\');
}

std::shared_ptr<void> LibraryLoader::handle(void* addr, bool noref)
{
  HMODULE hModule = NULL;

  if ( ! GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           (noref ? GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT : 0),
                           (TCHAR*) addr,
                           &hModule))
  {
    int ret = GetLastError();
    _messagebox(QString("GetModuleHandle returned error: 0x%1")
                .arg(QString::number(ret, 16)));
  }

  if (noref)
  {
    return std::shared_ptr<void>(hModule, [](void*){});
  }

  return std::shared_ptr<void>(hModule, [](void* ptr){FreeLibrary((HMODULE)ptr);});
}

std::shared_ptr<void> LibraryLoader::handleThis(bool noref)
{
  volatile static TCHAR* localVar = (TCHAR*)0x12345;
  return handle(&localVar, noref);
}

std::shared_ptr<void> LibraryLoader::handlePath(const QString& path)
{
  HMODULE hDll = LoadLibraryEx(_TQ(path), NULL,
                               LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
                               LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);

  return std::shared_ptr<void>(hDll, [](void* ptr){FreeLibrary((HMODULE)ptr);});
}

InterfaceKeeper::~InterfaceKeeper()
{
  if (m_ptr && m_ptr.use_count() == 3)
  { // after destruction it will be the single (2 in map)
    LibraryLoader::i().d_func()->map.remove(m_ptr);
  }
}

#include "libraryloader.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include "common.h"
#include "listplug_qt_iface.h"

// plugin loader
QScopedPointer<LibraryLoader> g_pLibraryLoader;

class LibraryHolder
{
public:
  LibraryHolder(HMODULE hModule) :
    m_hModule(hModule)
  { }

  ~LibraryHolder()
  {
    if (m_hModule)
    {
      FreeLibrary(m_hModule);
    }
  }

  HMODULE module() const { return m_hModule; }

private:
  HMODULE m_hModule;
};

struct Library
{
  Library() :
    iRef(NULL) {}

  Library(ListPlugQtIface* ptr, QSharedPointer<LibraryHolder> hLib) :
    pIface(QSharedPointer<ListPlugQtIface>(ptr, releaseIface)),
    pLib(hLib),
    iRef(NULL) {}

  bool isNull() const { return pIface.isNull(); }

  QSharedPointer<ListPlugQtIface> pIface;
  QSharedPointer<LibraryHolder> pLib;
  mutable QAtomicInt iRef;

private:
  static void releaseIface(ListPlugQtIface* iface)
  {
    delete iface;
  }
};

class LibraryMap
{
public:
  LibraryMap() :
    m_defLib(Library()) {}

  ListPlugQtIface* iface(HMODULE hMod) const
  {
    return lib(hMod)->pIface.data();
  }

  Library* add(ListPlugQtIface* ptr, QSharedPointer<LibraryHolder> hLib)
  {
    _assert(ptr);
    _assert( ! hLib.isNull() );
    _assert(hLib->module() != NULL);
    _assert(lib(ptr) == &m_defLib); // should not exist

    _log(QString("Library acquired: %1")
         .arg(LibraryLoader::i().pathModule(hLib->module())));

    Library neu(ptr, hLib);

    // current ref is 0
    m_libs.append(neu);
    return &(m_libs.last());
  }

  void ref(const ListPlugQtIface* iface)
  {
    ref(lib(iface));
  }

  void deref(const ListPlugQtIface* iface)
  {
    deref(lib(iface));
  }

private:
  const Library* lib(HMODULE hMod) const
  {
    for (int i = 0; i < m_libs.size(); ++i)
    {
      const Library& l = m_libs[i];
      if (l.pLib->module() == hMod)
      {
        return &l;
      }
    }
    return &m_defLib;
  }

  const Library* lib(const ListPlugQtIface* ptr) const
  {
    for (int i = 0; i < m_libs.size(); ++i)
    {
      const Library& l = m_libs[i];
      if (l.pIface.data() == ptr)
      {
        return &l;
      }
    }
    return &m_defLib;
  }

  void ref(const Library* l)
  {
    _assert(l);
    _assert( ! l->isNull() ); // not default
    _assert( ! lib(l->pIface.data())->isNull() ); // present in the list
    _assert(l->iRef >= 0);
    ++(l->iRef);
  }

  void deref(const Library* l)
  {
    _assert(l);
    _assert( ! l->isNull() ); // not default
    _assert( ! lib(l->pIface.data())->isNull() ); // present in the list
    _assert(l->iRef > 0);
    if ( ! (--(l->iRef)) )
    { // all references released
      for (int i = 0; i < m_libs.size(); ++i)
      {
        if (l == &m_libs.at(i))
        {
          _log(QString("Library released: %1")
               .arg(LibraryLoader::i().pathModule(l->pLib->module())));
          m_libs.takeAt(i);
          break;
        }
      }
    }
  }

private:
  QList<Library> m_libs;
  const Library m_defLib;
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

QString LibraryLoader::pathModule(void* handle)
{
  TCHAR strPath[MAX_PATH + 1];
  memset(strPath, 0, sizeof(strPath));

  _assert(handle);
  if (handle)
  {
    GetModuleFileName((HMODULE)handle, strPath, sizeof(strPath));
  }

  return _toString(strPath);
}

LibraryLoader::LibraryLoader() :
  d_ptr(new LibraryLoaderPrivate())
{
  _log("LibraryLoader created");

  // enable loading Qt plugins
  QCoreApplication::setLibraryPaths(QStringList() << dirByPath(pathThis()));
}

LibraryLoader::~LibraryLoader()
{
  delete d_ptr;
  g_pLibraryLoader.take();

  QCoreApplication::setLibraryPaths(QStringList());

  _log("LibraryLoader destroyed");
}

InterfaceKeeper LibraryLoader::keeper(void* addr)
{
  Q_D(LibraryLoader);

  HMODULE hDll = (HMODULE)handle(addr);
  _assert(hDll);

  // wrong addr passed
  _assert(hDll != handleThis(true));

  if ( ! hDll )
  {
    return InterfaceKeeper();
  }

  QSharedPointer<LibraryHolder> holder(new LibraryHolder(hDll));

  ListPlugQtIface* iface = d->map.iface(hDll);

  if ( ! iface )
  { // doesn't exist, need create
    PGetListPlugQtIface pFunc = (PGetListPlugQtIface)GetProcAddress(hDll, "GetListPlugQtIface");
    _assert(pFunc);
    if ( ! pFunc )
    {
      return InterfaceKeeper();
    }

    iface = pFunc();
    _assert(iface);
    if ( ! iface )
    {
      return InterfaceKeeper();
    }

    d->map.add(iface, holder);
  }

  return InterfaceKeeper(iface);
}

QString LibraryLoader::pathThis()
{
  return pathModule(handleThis(true));
}

QString LibraryLoader::dirByPath(const QString& path)
{
  return QFileInfo(path).canonicalPath().replace('/', QDir::separator());
}

void* LibraryLoader::handle(void* addr, bool noref)
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

  return hModule;
}

void* LibraryLoader::handleThis(bool noref)
{
  volatile static TCHAR* localVar = (TCHAR*)0x12345;
  return handle(&localVar, noref);
}

void* LibraryLoader::handlePath(const QString& path)
{
  HMODULE hDll = LoadLibraryEx(_TQ(path), NULL,
                               LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
                               LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);

  return hDll;
}


class InterfaceKeeperPrivate
{
public:
  InterfaceKeeperPrivate() :
    iface(NULL)
  {}

  ListPlugQtIface* iface;
};


InterfaceKeeper::InterfaceKeeper():
  d_ptr(new InterfaceKeeperPrivate())
{}

InterfaceKeeper::InterfaceKeeper(ListPlugQtIface* iface) :
  d_ptr(new InterfaceKeeperPrivate())
{
  Q_D(InterfaceKeeper);
  d->iface = iface;
  if ( ! isNull() )
  { // acquire ref
    LibraryLoader::i().d_func()->map.ref(d->iface);
  }
}

InterfaceKeeper::InterfaceKeeper(const InterfaceKeeper& other) :
  d_ptr(new InterfaceKeeperPrivate())
{
  Q_D(InterfaceKeeper);
  d->iface = other.d_func()->iface;
  if ( ! isNull() )
  { // acquire ref
    LibraryLoader::i().d_func()->map.ref(d->iface);
  }
}

InterfaceKeeper::~InterfaceKeeper()
{
  Q_D(InterfaceKeeper);
  if ( ! isNull() )
  { // release ref
    LibraryLoader::i().d_func()->map.deref(d->iface);
  }

  delete d_ptr;
}

ListPlugQtIface* InterfaceKeeper::iface() const
{
  Q_D(const InterfaceKeeper);
  return d->iface;
}

bool InterfaceKeeper::isNull() const
{
  return iface() == NULL;
}

bool InterfaceKeeper::operator==(const InterfaceKeeper& other) const
{
  return iface() == other.iface();
}

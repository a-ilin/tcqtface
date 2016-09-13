#ifndef LIBRARYLOADER_H
#define LIBRARYLOADER_H

#include <memory>

#include <QString>

class IAbstractWlxPlugin;

typedef std::shared_ptr<void> Library;
typedef std::shared_ptr<IAbstractWlxPlugin> Interface;

class LoaderPrivate;
class Loader
{
public:
  // get plugin interface for module that contains addr,
  // set pCreated if the interface is newly created
  Interface iface(Library& hModule, bool* pCreated);

  // returns if library contains a module handle specified by addr
  bool containsLibrary(void* addr) const;

  static std::shared_ptr<Loader> i();
  static bool isExists();
  static bool destroy();

  // path to module
  static QString pathModule(void* hModule);
  // path to this Dll
  static QString pathThis();
  // get directory from path
  static QString dirByPath(const QString& path);

  // get handle by address
  static Library handle(void* addr, bool noref = false);
  // get handle of this Dll
  static Library handleThis(bool noref = false);
  // get handle of Dll specified in path
  static Library handlePath(const QString& path);

  // multithreading guard
  static void lock();
  static void unlock();

private:
  Loader();
  ~Loader();

private:
  LoaderPrivate* const d_ptr;

  Q_DISABLE_COPY(Loader)
  Q_DECLARE_PRIVATE(Loader)
};

class LoaderLocker
{
public:
  LoaderLocker()  { Loader::lock(); }
  ~LoaderLocker() { Loader::unlock(); }
};

#endif // LIBRARYLOADER_H

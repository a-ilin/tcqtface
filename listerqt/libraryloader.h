#ifndef LIBRARYLOADER_H
#define LIBRARYLOADER_H

#include <memory>

#include <QString>

class IAbstractWlxPlugin;

class InterfaceKeeper
{
public:
  InterfaceKeeper(){}
  InterfaceKeeper(const std::shared_ptr<IAbstractWlxPlugin>& ptr) : m_ptr(ptr) {}
  ~InterfaceKeeper();

  IAbstractWlxPlugin* get() const { return m_ptr.get(); }
  operator bool() const { return static_cast<bool>(m_ptr); }

  operator IAbstractWlxPlugin*() const { return m_ptr.get(); }
  IAbstractWlxPlugin* operator->() const { return m_ptr.get(); }

private:
  InterfaceKeeper& operator= (const InterfaceKeeper&) = delete;

private:
  std::shared_ptr<IAbstractWlxPlugin> m_ptr;
};

class LibraryLoaderPrivate;
class LibraryLoader
{
public:
  ~LibraryLoader();

  // get plugin interface for module that contains addr
  InterfaceKeeper iface(void* addr);

  // returns if library contains a module handle specified by addr
  bool containsLibrary(void* addr) const;

  // returns if the library loader map is empty
  bool isEmpty() const;

  static LibraryLoader& i();

  static bool isExists();

  // path to module
  static QString pathModule(void* hModule);
  // path to this Dll
  static QString pathThis();
  // get directory from path
  static QString dirByPath(const QString& path);

  // get handle by address
  static std::shared_ptr<void> handle(void* addr, bool noref = false);
  // get handle of this Dll
  static std::shared_ptr<void> handleThis(bool noref = false);
  // get handle of Dll specified in path
  static std::shared_ptr<void> handlePath(const QString& path);

private:
  LibraryLoader();

private:
  LibraryLoaderPrivate* const d_ptr;

  Q_DISABLE_COPY(LibraryLoader)
  Q_DECLARE_PRIVATE(LibraryLoader)

  friend class InterfaceKeeper;
};

#endif // LIBRARYLOADER_H

#ifndef LIBRARYLOADER_H
#define LIBRARYLOADER_H

#include <QScopedPointer>

class ListPlugQtIface;

class InterfaceKeeperPrivate;
class InterfaceKeeper
{
public:
  InterfaceKeeper();
  InterfaceKeeper(ListPlugQtIface* iface);
  InterfaceKeeper(const InterfaceKeeper& other);

  ~InterfaceKeeper();

  ListPlugQtIface* iface() const;
  bool isNull() const;

  bool operator== (const InterfaceKeeper& other) const;

private:
  InterfaceKeeper& operator= (const InterfaceKeeper&) = delete;

private:
  InterfaceKeeperPrivate* const d_ptr;

  Q_DECLARE_PRIVATE(InterfaceKeeper)
};

class LibraryLoaderPrivate;
class LibraryLoader
{
public:
  ~LibraryLoader();

  // get plugin interface for module that contains addr
  InterfaceKeeper keeper(void* addr);

  static LibraryLoader& i();

  // path to module
  static QString pathModule(void* handle);
  // path to this Dll
  static QString pathThis();
  // get directory from path
  static QString dirByPath(const QString& path);

  // get handle by address
  static void* handle(void* addr, bool noref = false);
  // get handle of this Dll
  static void* handleThis(bool noref = false);
  // get handle of Dll specified in path
  static void* handlePath(const QString& path);

private:
  LibraryLoader();

private:
  LibraryLoaderPrivate* const d_ptr;

  Q_DISABLE_COPY(LibraryLoader)
  Q_DECLARE_PRIVATE(LibraryLoader)

  friend class InterfaceKeeper;
};

#endif // LIBRARYLOADER_H

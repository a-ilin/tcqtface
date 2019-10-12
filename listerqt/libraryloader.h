/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 by Aleksei Ilin
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

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 by Aleksei Ilin
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

#ifndef CORE_H
#define CORE_H

#include <functional>
#include <memory>

#include <qt_windows.h>
#include <qglobal.h>

struct CoreData;

class CoreEvent;
class CorePayload;

class IWlxCore;

class Core
{
  Q_DISABLE_COPY(Core)

public:
  void processPayload(CorePayload& payload, bool processEvents = true);

  void increaseWinCounter();
  void decreaseWinCounter();
  int winCounter() const;

  // return WlxCore instance
  IWlxCore* wlxCore() const;

  static std::shared_ptr<Core> i();
  static bool isExists();
  static bool destroy();

  // multithreading guard
  static void lock();
  static void unlock();

private:
  bool startApplication();
  void stopApplication();

  void processPayload_helper(CoreEvent* event);

  // dispatches system messages until hSem is available
  void dispatchMessages();

private:
  Core();
  ~Core();

private:
  std::unique_ptr<CoreData> d;
};

class CoreLocker
{
public:
  CoreLocker()  { Core::lock(); }
  ~CoreLocker() { Core::unlock(); }
};

class CoreEvent;
class CorePayload
{
public:
  CorePayload() {}
  virtual ~CorePayload() {}

  CoreEvent* createEvent();

  virtual bool preprocess() { return true; }
  virtual void process() = 0;
  virtual void postprocess() {}
};

template <typename FuncPre, typename Func, typename FuncPost>
class CorePayloadTmpl : public CorePayload
{
public:
  CorePayloadTmpl(FuncPre funcPre, Func func, FuncPost funcPost)
    : CorePayload()
    , m_funcPre(funcPre)
    , m_func(func)
    , m_funcPost(funcPost) {}

  bool preprocess() Q_DECL_OVERRIDE { return m_funcPre(); }
  void process() Q_DECL_OVERRIDE { m_func(); }
  void postprocess() Q_DECL_OVERRIDE { m_funcPost(); }

private:
  FuncPre m_funcPre;
  Func m_func;
  FuncPost m_funcPost;
};

template <typename FuncPre, typename Func, typename FuncPost> inline
CorePayloadTmpl<FuncPre, Func, FuncPost> createCorePayloadEx(FuncPre funcPre, Func func, FuncPost funcPost)
{
  return CorePayloadTmpl<FuncPre, Func, FuncPost>(funcPre, func, funcPost);
}

template <typename Func> inline
CorePayloadTmpl<std::function<bool()>, Func, std::function<void()> > createCorePayload(Func func)
{
  return createCorePayloadEx(std::function<bool()>([]()->bool{return true;}), func, std::function<void()>([]{}));
}

#endif // CORE_H

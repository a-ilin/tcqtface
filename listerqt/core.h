#ifndef CORE_H
#define CORE_H

#include <functional>
#include <memory>

#include <qt_windows.h>
#include <qglobal.h>

struct CoreData;

class AtomicMutex;
class CoreEvent;
class CorePayload;

class Core
{
  Q_DISABLE_COPY(Core)

public:
  ~Core();

  void processPayload(CorePayload& payload);

  void increaseWinCounter();
  void decreaseWinCounter();

  // core is not used by any task
  bool isUnusable() const;

  static Core& i();
  static bool isExists();

private:
  bool startApplication();
  void stopApplication();

  void processPayload_helper(CoreEvent* event);

  // dispatches system messages until hSem is available
  static void dispatchMessages(HANDLE hSem);

private:
  Core();

private:
  std::unique_ptr<CoreData> d;
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

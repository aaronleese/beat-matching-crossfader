#ifndef __REC_UTIL_THREAD_CALLBACK0__
#define __REC_UTIL_THREAD_CALLBACK0__

#include "rec/util/thread/Callback.h"

namespace rec {
namespace util {
namespace thread {
namespace callback {

template <typename Type, typename Method>
class Callback0 : public Callback {
 public:
  Callback0(Type* o, Method m) : object_(o), method_(m) {}
  virtual bool operator()() {
    (object_->*method_)();
    return true;
  }

 private:
  Type* object_;
  Method method_;
};

template <typename Type, typename Method>
class CallbackBool0 : public Callback {
 public:
  CallbackBool0(Type* o, Method m) : object_(o), method_(m) {}

  virtual bool operator()() { return (object_->*method_)(); }

 private:
  Type* object_;
  Method method_;
};

}  // namespace callback

template <typename Type, typename Method>
Callback* makeCallback(Type* o, Method m) {
  return new thread::callback::Callback0<Type, Method>(o, m);
}

template <typename Type, typename Method>
Callback* makeCallbackBool(Type* o, Method m) {
  return new thread::callback::CallbackBool0<Type, Method>(o, m);
}

template <typename Type, typename Method>
void callAsync(Type* o, Method m) {
  (new thread::callback::CallbackMessage(makeCallback<Type, Method>(o, m)))->post();
}

template <typename Type, typename Method>
Thread* makeThread(const String& name, Type* o, Method m) {
  return new thread::callback::Thread(name, makeCallback<Type, Method>(o, m));
}

}  // namespace thread
}  // namespace util
}  // namespace rec

#endif  // __REC_UTIL_THREAD_CALLBACK0__

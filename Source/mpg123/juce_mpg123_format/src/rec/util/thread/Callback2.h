#ifndef __REC_UTIL_THREAD_CALLBACK2__
#define __REC_UTIL_THREAD_CALLBACK2__

#include "rec/util/thread/Callback.h"

namespace rec {
namespace util {
namespace thread {
namespace callback {

template <typename Type, typename Method, typename V1, typename V2>
class Callback2 : public Callback {
 public:
  Callback2(Type* o, Method m, V1 v1, V2 v2) : object_(o), method_(m), v1_(v1), v2_(v2) {}
  virtual bool operator()() {
    (object_->*method_)(v1_, v2_);
    return true;
  }

 private:
  Type* object_;
  Method method_;
  V1 v1_;
  V2 v2_;
};

template <typename Type, typename Method, typename V1, typename V2>
class CallbackBool2 : public Callback {
 public:
  CallbackBool2(Type* o, Method m, V1 v1, V2 v2) : object_(o), method_(m), v1_(v1), v2_(v2) {}

  virtual bool operator()() { return (object_->*method_)(v1_, v2_); }

 private:
  Type* object_;
  Method method_;
  V1 v1_;
  V2 v2_;
};

}  // namespace callback

template <typename Type, typename Method, typename V1, typename V2>
Callback* makeCallback(Type* o, Method m, V1 v1, V2 v2) {
  return new thread::callback::Callback2<Type, Method, V1, V2>(o, m, v1, v2);
}

template <typename Type, typename Method, typename V1, typename V2>
Callback* makeCallbackBool(Type* o, Method m, V1 v1, V2 v2) {
  return new thread::callback::CallbackBool2<Type, Method, V1, V2>(o, m, v1, v2);
}

template <typename Type, typename Method, typename V1, typename V2>
void callAsync(Type* o, Method m, V1 v1, V2 v2) {
  (new thread::callback::CallbackMessage(makeCallback<Type, Method, V1, V2>(o, m, v1, v2)))->post();
}

template <typename Type, typename Method, typename V1, typename V2>
Thread* makeThread(const String& name, Type* o, Method m, V1 v1, V2 v2) {
  return new thread::callback::Thread(name, makeCallback<Type, Method, V1, V2>(o, m, v1, v2));
}

}  // namespace thread
}  // namespace util
}  // namespace rec

#endif  // __REC_UTIL_THREAD_CALLBACK2__

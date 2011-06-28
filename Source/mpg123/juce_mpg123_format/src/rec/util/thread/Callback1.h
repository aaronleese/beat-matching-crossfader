#ifndef __REC_UTIL_THREAD_CALLBACK1__
#define __REC_UTIL_THREAD_CALLBACK1__

#include "rec/util/thread/Callback.h"

namespace rec {
namespace util {
namespace thread {
namespace callback {

template <typename Type, typename Method, typename Value>
class Callback1 : public Callback {
 public:
  Callback1(Type* o, Method m, Value v) : object_(o), method_(m), value_(v) {}
  virtual bool operator()() {
    (object_->*method_)(value_);
    return true;
  }

 private:
  Type* object_;
  Method method_;
  Value value_;
};

template <typename Type, typename Method, typename Value>
class CallbackBool1 : public Callback {
 public:
  CallbackBool1(Type* o, Method m, Value v) : object_(o), method_(m), value_(v) {}

  virtual bool operator()() { return (object_->*method_)(); }

 private:
  Type* object_;
  Method method_;
  Value value_;
};

}  // namespace callback

template <typename Type, typename Method, typename Value>
Callback* makeCallback(Type* o, Method m, Value v) {
  return new thread::callback::Callback1<Type, Method, Value>(o, m, v);
}

template <typename Type, typename Method, typename Value>
Callback* makeCallbackBool(Type* o, Method m, Value v) {
  return new thread::callback::CallbackBool1<Type, Method, Value>(o, m, v);
}

template <typename Type, typename Method, typename Value>
void callAsync(Type* o, Method m, Value v) {
  (new thread::callback::CallbackMessage(makeCallback<Type, Method, Value>(o, m, v)))->post();
}

template <typename Type, typename Method, typename Value>
Thread* makeThread(const String& name, Type* o, Method m, Value v) {
  return new thread::callback::Thread(name, makeCallback<Type, Method, Value>(o, m, v));
}

}  // namespace thread
}  // namespace util
}  // namespace rec

#endif  // __REC_UTIL_THREAD_CALLBACK1__

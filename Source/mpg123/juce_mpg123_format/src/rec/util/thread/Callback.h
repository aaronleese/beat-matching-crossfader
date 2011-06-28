#ifndef __REC_THREAD_CALLBACK__
#define __REC_THREAD_CALLBACK__

#include "rec/base/base.h"

namespace rec {
namespace util {
namespace thread {
namespace callback {

class Callback {
 public:
  virtual bool operator()() = 0;
  virtual ~Callback() {}
};

class Wrapper : public Callback, public ptr<Callback> {
 public:
  explicit Wrapper(Callback* r = NULL) : ptr<Callback>(r) {}
  virtual ~Wrapper() {}
  virtual bool operator()() { return (*get())(); }
};

class CallbackMessage : public juce::CallbackMessage, public Wrapper {
 public:
  CallbackMessage(Callback* r = NULL) : Wrapper(r) {}
  virtual void messageCallback() { (*this)(); }
};

class Thread : public juce::Thread, public Wrapper {
 public:
  Thread(const String& name, Callback* r = NULL)
      : juce::Thread(name), Wrapper(r) {
  }
  virtual void run() { (*this)(); }
};

class Loop : public juce::Thread, public Wrapper {
 public:
  Loop(const String& name, int waitTime, Callback* r = NULL)
      : Thread(name), Wrapper(r), waitTime_(waitTime) {
  }

  virtual void run() {
    while (!threadShouldExit() && (*this)())
      wait(waitTime_);
  }

  static Loop* make(const String& name, int per, int prio, Callback* cb) {
    ptr<Loop> thread(new Loop(name, per, cb));
    thread->setPriority(prio);
    thread->startThread();
    return thread.transfer();
  }

  const int waitTime_;
};

class TimeSliceClient : public juce::TimeSliceClient, public Wrapper {
 public:
  TimeSliceClient(Callback* r = NULL) : Wrapper(r) {}
  virtual bool useTimeSlice() { return (*this)(); }
};

}  // namespace callback
}  // namespace thread

typedef thread::callback::Callback Callback;

}  // namespace util
}  // namespace rec

#include "rec/util/thread/Callback0.h"
#include "rec/util/thread/Callback1.h"
#include "rec/util/thread/Callback2.h"

#endif  // __REC_THREAD_CALLBACK__

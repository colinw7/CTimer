#include "std_c++.h"
#include "CTimer/CTimer.h"
#include "CXLib/CXMachine.h"

class Timer : public CTimer {
 private:
  static int ticks_;

  int secs_;

 public:
  Timer(int secs);

  void timeOut();
};

int Timer::ticks_;

int
main(int, char **)
{
  CTimerMgrInst->setExternalUpdate(true);

  Timer *timer1 = new Timer(1); assert(timer1);
  Timer *timer2 = new Timer(3); assert(timer2);
  Timer *timer3 = new Timer(5); assert(timer3);

  CXMachineInst->openDisplay();

  CXMachineInst->mainLoop();

  return 0;
}

Timer::
Timer(int secs) :
 CTimer(1000*secs, CTIMER_FLAGS_REPEAT), secs_(secs)
{
}

void
Timer::
timeOut()
{
  if (secs_ == 1)
    ++ticks_;

  cerr << ticks_ << ": " << secs_ << endl;
}

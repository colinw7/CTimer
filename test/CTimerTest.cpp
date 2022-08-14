#include <CTimer.h>

class Timer : public CTimer {
 public:
  Timer(int secs);

  void timeOut();

 private:
  int secs_ { 0 };
};

static time_t start_time;

int
main(int, char **)
{
  Timer *timer1 = new Timer(1); assert(timer1);
  Timer *timer2 = new Timer(3); assert(timer2);
  Timer *timer3 = new Timer(5); assert(timer3);

  start_time = time(NULL);

  for (;;) {
    sleep(1);
  }
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
  time_t t = time(NULL);

  double d = difftime(t, start_time);

  cerr << "Timer " << secs_ << ":" << d << endl;
}

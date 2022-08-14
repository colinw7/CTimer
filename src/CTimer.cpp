#include <CTimer.h>
#include <COSSignal.h>
#include <COSTime.h>

// TODO: allow use of timer_create(), timer_settime(), timer_delete()

CTimerMgr::
CTimerMgr()
{
}

void
CTimerMgr::
addTimer(CTimer *timer)
{
  if (! signal_handler_) {
    if (! external_update_)
      COSSignal::addSignalHandler(SIGALRM, CTimerMgr::signalHandler);
    else
      last_t_ = COSTime::getHRTime();

    signal_handler_ = true;
  }

  if (! external_update_) {
    ulong msecs = timer->getMSecsInit();

    if (itimer_active_)
      updateTimer(msecs);
    else
      setTimer(msecs);
  }

  timers_.push_back(timer);
}

void
CTimerMgr::
removeTimer(CTimer *timer)
{
  if (! timer_updating_)
    timers_.remove(timer);
  else
    rtimers_.insert(timer);
}

void
CTimerMgr::
signalHandler(int)
{
  CTimerMgrInst->updateTimers();
}

void
CTimerMgr::
tick()
{
  if (! signal_handler_)
    return;

  CHRTime t = COSTime::getHRTime();

  CHRTime dt = COSTime::diffHRTime(last_t_, t);

  itimer_msecs_ = ulong(dt.getMSecs());

  if (itimer_msecs_ > 0) {
    last_t_ = t;

    updateTimers();
  }
}

void
CTimerMgr::
updateTimers()
{
  if (timers_.empty())
    return;

  ulong msecs      = 0;
  ulong msecs_left = 0;

  timer_updating_ = true;

  TimerList timers = timers_;

  for (auto &timer : timers) {
    if (! timer->isActive())
      continue;

    msecs_left = timer->getMSecsLeft();

    if (itimer_msecs_ + 1 < msecs_left)
      timer->setMSecsLeft(msecs_left - itimer_msecs_);
    else
      timer->setMSecsLeft(0);

    if (timer->getMSecsLeft() == 0) {
      timer->timeOut();

      if      (timer->isRepeat())
        timer->setMSecsLeft(timer->getMSecsInit());
      else if (timer->isDelete())
        rtimers_.insert(timer);
      else {
        timer->setMSecsLeft(timer->getMSecsInit());

        timer->setActive(false);
      }
    }

    msecs_left = timer->getMSecsLeft();

    if (msecs_left > 0) {
      if (msecs == 0 || msecs_left < msecs)
        msecs = msecs_left;
    }
  }

  timer_updating_ = false;

  for (auto &timer : rtimers_) {
    timers_.remove(timer);
  }

  rtimers_.clear();

  if (! external_update_) {
    if (msecs > 0)
      setTimer(msecs);
    else
      itimer_active_ = false;
  }
}

void
CTimerMgr::
updateTimer(ulong msecs)
{
  getTimer(&itimer_msecs_);

  if (msecs < itimer_msecs_)
    setTimer(msecs);
}

void
CTimerMgr::
getTimer(ulong *msecs)
{
  int error = getitimer(ITIMER_REAL, &itimer_);

  if (error == -1) return;

  *msecs = 1000*ulong(itimer_.it_value.tv_sec) + ulong(itimer_.it_value.tv_usec/1000);
}

void
CTimerMgr::
setTimer(ulong msecs)
{
  itimer_msecs_ = msecs;

  if (! external_update_) {
    itimer_.it_interval.tv_usec = 0;
    itimer_.it_interval.tv_sec  = 0;
    itimer_.it_value   .tv_usec = 1000*(itimer_msecs_ % 1000);
    itimer_.it_value   .tv_sec  = itimer_msecs_ / 1000;

    int error = setitimer(ITIMER_REAL, &itimer_, NULL);

    if (error != -1)
      itimer_active_ = true;
  }
}

//---------------

CTimer::
CTimer(ulong msecs, CTimerFlags flags)
{
  msecs_init_ = msecs;
  msecs_left_ = msecs;
  flags_      = flags;

  CTimerMgrInst->addTimer(this);

  if (! (flags & CTIMER_FLAGS_NO_ACTIVE))
    active_ = true;
  else
    active_ = false;
}

CTimer::
~CTimer()
{
  active_ = false;

  CTimerMgrInst->removeTimer(this);
}

void
CTimer::
stop()
{
  active_ = false;
}

void
CTimer::
restart()
{
  msecs_left_ = msecs_init_;

  active_ = true;
}

void
CTimer::
restart(ulong msecs)
{
  msecs_init_ = msecs;

  restart();
}

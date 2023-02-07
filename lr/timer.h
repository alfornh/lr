#ifndef _TIMER_H__
#define _TIMER_H__

#include <memory>
#include <map>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "event.h"
#include "event_type.h"

#define PTIMER Timer::_instance

class Event;
class Timer :public ESource {
public:
  Timer() {
    _stype = EVENT_TYPE_TIMER;
    _stop_flag = false;
    _current_time = 0;
    _r_event_pool_id = 0;
    _timer_thread_group_id = 0;
  }

  virtual ~Timer() { }

  int init();

  int start();

  int add(int, std::shared_ptr<Event>);

  void stop();
public:
  void _timer_thread();

private:
  typedef std::map<int, std::shared_ptr<Event>, std::less<int>> TimerEventContainer;

  TimerEventContainer _timer_events;
  std::condition_variable _cv_timer_events;
  std::mutex _mutex_timer_events;
  int _current_time;

  //int _timer_fd;
  bool _stop_flag;

  int _timer_thread_group_id;
public:
  int _r_event_pool_id;
public:
  static std::shared_ptr<Timer> _instance;
};

#endif//_TIMER_H__

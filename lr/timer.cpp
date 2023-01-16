#include "timer.h"

#include "zlog.h"
#include "configure.h"
#include "event.h"
#include "event_type.h"
#include "event_pool.h"
#include "thread_manager.h"

std::shared_ptr<Timer> Timer::_instance = std::make_shared<Timer>();

int Timer::init() {
  _stop_flag = false;
  _r_event_pool_id = EventPool::reserve_event_queue();
  if (_r_event_pool_id < 1) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "error get event pool slot id");
    return -1;
  }

  _timer_thread_group_id = PTHREADMANAGER->reserve_thread_group();
  if (_timer_thread_group_id < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
    return -1;
  }
  PTHREADMANAGER->start(_timer_thread_group_id, 1);
  return 0;
}

int Timer::start() {
  if (_timer_thread_group_id < 1) {
    ZLOG_WARN(__FILE__, __LINE__, __func__, "_timer_thread_group_id", _timer_thread_group_id);
    return -1;
  }

  RUN_TASK(_timer_thread_group_id,
    MAKE_SHARED(Task, std::bind(&Timer::_timer_thread, Timer::_instance))
  );
  ZLOG_INFO(__FILE__, __LINE__, __func__, "start _timer_thread");

  return 0;
}

int Timer::add(int time, std::shared_ptr<Event> event) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, time);
  if (! PCONFIGURE->line_enable(CONFIGURE_TIMER) ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "timer not enable");
    return -1;
  }

  {
    //if exist in second, then +1 sec, and then to look up again, until a now one.
    std::unique_lock<std::mutex> l(_mutex_timer_events);
    do {
      TimerEventContainer::iterator it = _timer_events.find(time);
      if (it == _timer_events.end()) {
        _timer_events.insert(std::pair<int, std::shared_ptr<Event> >(time, event));
        break;
      }
    //}
    } while (++time);

    if (_current_time == 0 || time > _current_time) {
      return 0;
    }

  }

  _cv_timer_events.notify_one();

  return 0;
}

void Timer::_timer_thread() {
  Event::ptr ev;

  int span = 0;
  while ( !_stop_flag ) {
    int now = 0;
    ev = Event::ptr();

    {
      std::unique_lock<std::mutex> l(_mutex_timer_events);
      while (_timer_events.empty() || span > 0) {
        now = time(0);
        _current_time = now + span;
        _cv_timer_events.wait_for(l, std::chrono::seconds(span));

        if (_timer_events.empty())
          span = 30;
        else
          span = 0;
      }


      TimerEventContainer::iterator beg = _timer_events.begin();
      if (beg == _timer_events.end()) {
        continue;
      }

      now = time(0);
      span = beg->first - now;

      if (span < 1) {
        ev = beg->second;
        _timer_events.erase(beg);
        span = 0;

      } else if (span > 30) {
        span = 30;
      }
    }

    if ( ev ) {
      //ev->_stype = PTIMER->_stype | EVENT_SUBTYPE_READ;
      ADD_EVENT(_r_event_pool_id, ev);
    }
  }
}

void Timer::stop() {
  ZLOG_WARN(__FILE__, __LINE__, __func__);
  _stop_flag = true;
}


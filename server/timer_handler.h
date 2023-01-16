#ifndef _TIMER_HANDLER_H__
#define _TIMER_HANDLER_H__

#include "handler.h"
#include "event_type.h"

class Event;
class TimerHandler: public Handler {
public:
  TimerHandler() { _stype = EVENT_TYPE_TIMER; }
  virtual ~TimerHandler() { }

  virtual int handle(EVENTID id, std::shared_ptr<Event>);

};

#endif //_TIMER_HANDLER_H__

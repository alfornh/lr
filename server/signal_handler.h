#ifndef _SIGNAL_HANDLER_H__
#define _SIGNAL_HANDLER_H__

#include "handler.h"

class SignalHandler: public Handler {
public:
  SignalHandler() { _stype = EVENT_TYPE_SIGNAL; }

  virtual ~SignalHandler() {}

  virtual int handle(EVENTID id, std::shared_ptr<Event>);
};

#endif //_SIGNAL_HANDLER_H__

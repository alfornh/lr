#ifndef _SIGNALS_HANDLER_H__
#define _SIGNALS_HANDLER_H__

#include "handler.h"

class SignalsHandler: public Handler {
public:
  SignalsHandler() { _stype = EVENT_TYPE_SIGNAL; }

  virtual ~SignalsHandler() {}

  virtual int handle(EVENTID id, std::shared_ptr<Event>);
};

#endif //_SIGNALS_HANDLER_H__

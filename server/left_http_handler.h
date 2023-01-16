#ifndef _LEFT_HTTP_HANDLER_H__
#define _LEFT_HTTP_HANDLER_H__

#include "handler.h"
#include "event_type.h"

class Event;
class LeftHttpHandler :public Handler {
public:
  LeftHttpHandler() { _stype = EVENT_TYPE_HTTPSOCKET; }
  virtual ~LeftHttpHandler() {}

  virtual int handle(EVENTID id, std::shared_ptr<Event>);
};

#endif //_LEFT_HTTP_HANDLER_H__

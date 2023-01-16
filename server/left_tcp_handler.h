#ifndef _LEFT_TCP_HANDLER_H__
#define _LEFT_TCP_HANDLER_H__

#include "handler.h"
#include "event_type.h"

class Event;
class LeftTcpHandler: public Handler {
public:
  LeftTcpHandler() { _stype = EVENT_TYPE_SOCKET_TCP; }

  virtual ~LeftTcpHandler() {}
  virtual int handle(EVENTID id, std::shared_ptr<Event>);


};

#endif //_LEFT_TCP_SOCKET_handler_H__

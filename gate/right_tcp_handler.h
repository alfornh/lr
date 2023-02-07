#ifndef _RIGHT_TCP_HANDLER_H__
#define _RIGHT_TCP_HANDLER_H__

#include "handler.h"
#include "event_type.h"

class Event;
class RightTcpHandler: public Handler {
public:
  RightTcpHandler() {
    _stype = EVENT_TYPE_SOCKET_TCP;
  }

  virtual ~RightTcpHandler() {}

  virtual int handle(EVENTID id, std::shared_ptr<Event>);

  int handle_read(std::shared_ptr<Event> &ev);
  int handle_close(std::shared_ptr<Event> &ev);
};

#endif //_RIGHT_TCP_HANDLER_H__

#ifndef _RIGHT_UDP_HANDLER_H__
#define _RIGHT_UDP_HANDLER_H__

#include "handler.h"
#include "event_type.h"

class Event;
class RightUdpHandler: public Handler {
public:
  RightUdpHandler() {
    _stype = EVENT_TYPE_SOCKET_UDP;
  }

  virtual ~RightUdpHandler() {}

  virtual int handle(EVENTID id, std::shared_ptr<Event>);
};

#endif//_GATE_UDP_HANDLER_H__

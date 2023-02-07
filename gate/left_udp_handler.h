#ifndef _LEFT_UDP_HANDLER_H__
#define _LEFT_UDP_HANDLER_H__

#include <memory>

#include "event_type.h"
#include "handler.h"

class LeftUdpHandler: public Handler {
public:
  LeftUdpHandler() {
    _stype = EVENT_TYPE_SOCKET_UDP;
  }

  virtual ~LeftUdpHandler() {}


  virtual int handle(EVENTID id, std::shared_ptr<Event>);
  int handle_read(std::shared_ptr<Event> &ev);

};

#endif //_LEFT_UDP_HANDLER_H__

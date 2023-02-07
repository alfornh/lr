#ifndef _LEFT_TCP_HANDLER_H__
#define _LEFT_TCP_HANDLER_H__

#include "handler.h"

class LeftTcpHandler: public Handler {
public:
  LeftTcpHandler() { _stype = EVENT_TYPE_SOCKET_TCP; }

  virtual ~LeftTcpHandler() {}
  virtual int handle(EVENTID id, std::shared_ptr<Event>);


  int handle_read(std::shared_ptr<Event> &ev);
  int handle_close(std::shared_ptr<Event> &ev);
};

#endif //_LEFT_TCP_HANDLER_H__

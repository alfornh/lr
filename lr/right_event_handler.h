#ifndef _RIGHT_EVENT_HANDLER_H__
#define _RIGHT_EVENT_HANDLER_H__

#include <memory>

#define PRIGHTEVENTHANDLER RightEventHandler::_instance

#include "handler.h"

class Event;
class RightEventHandler {
public:
  typedef std::shared_ptr<RightEventHandler> ptr;

  int handle(std::shared_ptr<Event>);

public:
  static void set_handlers(
    std::shared_ptr<Handler> tcp_handler,
    std::shared_ptr<Handler> udp_handler
  );


public:
  static std::shared_ptr<Handler> _tcp_handler;
  static std::shared_ptr<Handler> _udp_handler;

public:
  static std::shared_ptr<RightEventHandler> _instance;
};

#endif //_RIGHT_EVENT_HANDLER_H__

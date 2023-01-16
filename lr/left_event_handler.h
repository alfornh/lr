#ifndef _LEFT_EVENT_HANDLER_H__
#define _LEFT_EVENT_HANDLER_H__

#include <memory>

#include "event_type.h"
#include "listener.h"
#include "handler.h"

#define SEND_BACK(event, data)  \
  std::static_pointer_cast<Socket>(event->_es)->esend(data)


#define PLEFTEVENTHANDLER LeftEventHandler::_instance


class Event;
class LeftEventHandler {
public:
  int handle(std::shared_ptr<Event>);

public:
  static std::shared_ptr<LeftEventHandler> _instance;

public:
  static void set_handlers(
    std::shared_ptr<Handler> tcp_handler,
    std::shared_ptr<Handler> http_handler,
    std::shared_ptr<Handler> websocket_handler,
    std::shared_ptr<Handler> udp_handler,
    std::shared_ptr<Handler> timer_handler,
    std::shared_ptr<Handler> signal_handler
  );

  static std::shared_ptr<Handler> _websocket_handler;
  static std::shared_ptr<Handler> _tcp_handler;
  static std::shared_ptr<Handler> _http_handler;
  static std::shared_ptr<Handler> _timer_handler;
  static std::shared_ptr<Handler> _signal_handler;
  static std::shared_ptr<Handler> _udp_handler;
};

#endif//_LEFT_EVENT_HANDLER_H__

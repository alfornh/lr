#include "right_event_handler.h"

#include "utils.h"
#include "event.h"
#include "event_type.h"
#include "zlog.h"
#include "socket.h"

std::shared_ptr<RightEventHandler> RightEventHandler::_instance = MAKE_SHARED(RightEventHandler);

std::shared_ptr<Handler> RightEventHandler::_tcp_handler = MAKE_SHARED(Handler);
std::shared_ptr<Handler> RightEventHandler::_udp_handler = MAKE_SHARED(Handler);

int RightEventHandler::handle(std::shared_ptr<Event> event) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, event->_stype);
  switch (event->_stype & EVENT_MAIN_TYPE_MASK) {

  case EVENT_TYPE_SOCKET_TCP:
    LOCK_GUARD_MUTEX_BEGIN(STATIC_CAST(Socket, event->_es)->_mutex_for_worker_threads)
    return RightEventHandler::_tcp_handler->handle(event->_stype, event);
    LOCK_GUARD_MUTEX_END
  break;

  case EVENT_TYPE_SOCKET_UDP:
    LOCK_GUARD_MUTEX_BEGIN(STATIC_CAST(Socket, event->_es)->_mutex_for_worker_threads)
    return RightEventHandler::_udp_handler->handle(event->_stype, event);
    LOCK_GUARD_MUTEX_END
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown event type", event->_stype);
    return -1;
  }

  return 0;

}

void RightEventHandler::set_handlers(
  std::shared_ptr<Handler> tcp_handler,
  std::shared_ptr<Handler> udp_handler) {

  RightEventHandler::_tcp_handler   = tcp_handler;
  RightEventHandler::_udp_handler   = udp_handler;
}



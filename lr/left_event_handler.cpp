#include "left_event_handler.h"

#include "data_buffer.h"
#include "event.h"
#include "socket.h"
#include "utils.h"
#include "zlog.h"

std::shared_ptr<LeftEventHandler> LeftEventHandler::_instance = std::make_shared<LeftEventHandler>();

Handler::ptr LeftEventHandler::_websocket_handler = MAKE_SHARED(Handler);
Handler::ptr LeftEventHandler::_tcp_handler       = MAKE_SHARED(Handler);
Handler::ptr LeftEventHandler::_http_handler      = MAKE_SHARED(Handler);
Handler::ptr LeftEventHandler::_udp_handler       = MAKE_SHARED(Handler);
Handler::ptr LeftEventHandler::_timer_handler     = MAKE_SHARED(Handler);
Handler::ptr LeftEventHandler::_signal_handler    = MAKE_SHARED(Handler);

void LeftEventHandler::set_handlers(std::shared_ptr<Handler> tcp_handler,
  std::shared_ptr<Handler> http_handler,
  std::shared_ptr<Handler> websocket_handler,
  std::shared_ptr<Handler> udp_handler,
  std::shared_ptr<Handler> timer_handler,
  std::shared_ptr<Handler> signal_handler ) {

  LeftEventHandler::_websocket_handler   = websocket_handler;
  LeftEventHandler::_http_handler        = http_handler;
  LeftEventHandler::_tcp_handler         = tcp_handler;
  LeftEventHandler::_timer_handler       = timer_handler;
  LeftEventHandler::_signal_handler      = signal_handler;
  LeftEventHandler::_udp_handler         = udp_handler;
}

int LeftEventHandler::handle(std::shared_ptr<Event> ev) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, ev->_stype);
  switch (ev->_stype & EVENT_MAIN_TYPE_MASK) {
  case EVENT_TYPE_SOCKET_TCP:
    LOCK_GUARD_MUTEX_BEGIN(STATIC_CAST(Socket, ev->_es)->_mutex_for_worker_threads)
    return _tcp_handler->handle(ev->_stype, ev);
    LOCK_GUARD_MUTEX_END
  break;

  case EVENT_TYPE_WEBSOCKET:
    LOCK_GUARD_MUTEX_BEGIN(STATIC_CAST(Socket, ev->_es)->_mutex_for_worker_threads)
    return _websocket_handler->handle(ev->_stype, ev);
    LOCK_GUARD_MUTEX_END
  break;

  case EVENT_TYPE_HTTPSOCKET:
    LOCK_GUARD_MUTEX_BEGIN(STATIC_CAST(Socket, ev->_es)->_mutex_for_worker_threads)
    return _http_handler->handle(ev->_stype, ev);
    LOCK_GUARD_MUTEX_END
  break;

  case EVENT_TYPE_TIMER:
    return _timer_handler->handle(ev->_stype, ev);
  break;

  case EVENT_TYPE_SIGNAL:
    return _signal_handler->handle(ev->_stype, ev);
  break;

  case EVENT_TYPE_SOCKET_UDP:
    LOCK_GUARD_MUTEX_BEGIN(STATIC_CAST(Socket, ev->_es)->_mutex_for_worker_threads)
    return _udp_handler->handle(ev->_stype, ev);
    LOCK_GUARD_MUTEX_END
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown event type", ev->_stype);
    return -1;
  }

  return 0;
}

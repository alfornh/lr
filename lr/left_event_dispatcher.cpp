#include "left_event_dispatcher.h"

#include "configure.h"
#include "event.h"
#include "event_pool.h"
#include "left_event_handler.h"
#include "left_event_listener.h"
#include "left_udp_end.h"
#include "left_tcp_end.h"
#include "timer.h"
#include "thread_manager.h"

std::shared_ptr<LeftEventDispatcher> LeftEventDispatcher::_instance = std::make_shared<LeftEventDispatcher>();

LeftEventDispatcher::LeftEventDispatcher() { }

int LeftEventDispatcher::init() {
  _stop_flag = false;
  _handler   = LeftEventHandler::_instance;


  int maxt;
  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_TCP)) {
    _tcp_thread_group_id = PTHREADMANAGER->reserve_thread_group();
    if (_tcp_thread_group_id < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
      return -1;
    }
    maxt = PCONFIGURE->get_max_thread_number(CONFIGURE_LEFT_END_TCP);
    maxt += PLEFTEVENTLISTENER->_tcp_listeners.size(); // for dispatcher every one
    PTHREADMANAGER->start(_tcp_thread_group_id, maxt);
  }


  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_WEBSOCKET)) {
    _websocket_thread_group_id = PTHREADMANAGER->reserve_thread_group();
    if (_websocket_thread_group_id < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
      return -1;
    }
    maxt = PCONFIGURE->get_max_thread_number(CONFIGURE_LEFT_END_WEBSOCKET);
    maxt += PLEFTEVENTLISTENER->_websocket_listeners.size();
    PTHREADMANAGER->start(_websocket_thread_group_id, maxt);
  }


  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_HTTP)) {
    _http_thread_group_id = PTHREADMANAGER->reserve_thread_group();
    if (_http_thread_group_id < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
      return -1;
    }
    maxt = PCONFIGURE->get_max_thread_number(CONFIGURE_LEFT_END_HTTP);
    maxt += PLEFTEVENTLISTENER->_http_listeners.size();
    PTHREADMANAGER->start(_http_thread_group_id, maxt);
  }
 

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_UDP)) {
    _udp_thread_group_id = PTHREADMANAGER->reserve_thread_group();
    if (_udp_thread_group_id < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
      return -1;
    }
    maxt = PCONFIGURE->get_max_thread_number(CONFIGURE_LEFT_END_UDP);
    maxt += PLEFTEVENTLISTENER->_udp_listeners.size();
    PTHREADMANAGER->start(_udp_thread_group_id, maxt);
  }

  if (PCONFIGURE->line_enable(CONFIGURE_TIMER)) {
    _timer_thread_group_id = PTHREADMANAGER->reserve_thread_group();
    if (_timer_thread_group_id < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
      return -1;
    }
    maxt = PCONFIGURE->get_max_thread_number(CONFIGURE_TIMER);
    maxt += 1;
    PTHREADMANAGER->start(_timer_thread_group_id, maxt);
  }

  return 0;
}

void LeftEventDispatcher::dispatch_timer(int sid) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, sid);
  Event::ptr e;
  while ( !_stop_flag ) {
    e = GET_EVENT(sid);
    if (e) {
      ZLOG_DEBUG(__FILE__, __LINE__, __func__, e->_stype, sid);
      RUN_TASK(
        _timer_thread_group_id, 
        MAKE_SHARED(Task, std::bind(&LeftEventHandler::handle, _handler, std::placeholders::_1), e)
      );
    }
  }
}

void LeftEventDispatcher::dispatch_tcp(int sid) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, sid);
  Event::ptr e;
  while ( !_stop_flag ) {
    e = GET_EVENT(sid);
    if (e) {
      ZLOG_DEBUG(__FILE__, __LINE__, __func__, e->_stype, sid);
      RUN_TASK(
        _tcp_thread_group_id,
        MAKE_SHARED(Task, std::bind(&LeftEventHandler::handle, _handler, std::placeholders::_1), e)
      );
    }
  }
}

void LeftEventDispatcher::dispatch_http(int sid) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, sid);
  Event::ptr e;
  while ( !_stop_flag ) {
    e = GET_EVENT(sid);
    if (e) {
      ZLOG_DEBUG(__FILE__, __LINE__, __func__, e->_stype, sid);
      RUN_TASK(
        _http_thread_group_id,
        MAKE_SHARED(Task, std::bind(&LeftEventHandler::handle, _handler, std::placeholders::_1), e)
      );
    }
  }
}

void LeftEventDispatcher::dispatch_websocket(int sid) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, sid);
  Event::ptr e;
  while ( !_stop_flag ) {
    e = GET_EVENT(sid);
    if (e) {
      ZLOG_DEBUG(__FILE__, __LINE__, __func__, e->_stype, sid);
      RUN_TASK(
        _websocket_thread_group_id,
        MAKE_SHARED(Task, std::bind(&LeftEventHandler::handle, _handler, std::placeholders::_1), e)
      );
    }
  }
}

void LeftEventDispatcher::dispatch_udp(int sid) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, sid);
  Event::ptr e;
  while ( !_stop_flag ) {
    e = GET_EVENT(sid);
    if (e) {
      ZLOG_DEBUG(__FILE__, __LINE__, __func__, e->_stype, sid);
      RUN_TASK(
        _udp_thread_group_id,
        MAKE_SHARED(Task, std::bind(&LeftEventHandler::handle, _handler, std::placeholders::_1), e)
      );
    }
  }
}

int LeftEventDispatcher::dispatch() {
  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_TCP)) {
    size_t ln = PLEFTEVENTLISTENER->_tcp_listeners.size();
    for (size_t i = 0; i < ln; ++i ) {
      if (!PLEFTEVENTLISTENER->_tcp_listeners[i]) {
        continue;
      }

      RUN_TASK(_tcp_thread_group_id,
        MAKE_SHARED(Task, \
          std::bind(&LeftEventDispatcher::dispatch_tcp, LeftEventDispatcher::_instance, std::placeholders::_1), \
          PLEFTEVENTLISTENER->_tcp_listeners[i]->_r_event_pool_id \
        )
      );
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_WEBSOCKET)) {
    size_t ln = PLEFTEVENTLISTENER->_websocket_listeners.size();
    for (size_t i = 0; i < ln; ++i) {
      if (!PLEFTEVENTLISTENER->_websocket_listeners[i]) {
        continue;
      }

      RUN_TASK(_websocket_thread_group_id,
        MAKE_SHARED(Task, \
          std::bind(&LeftEventDispatcher::dispatch_websocket, LeftEventDispatcher::_instance, std::placeholders::_1), \
          PLEFTEVENTLISTENER->_websocket_listeners[i]->_r_event_pool_id \
        )
      );
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_HTTP)) {
    size_t ln = PLEFTEVENTLISTENER->_http_listeners.size();
    for (size_t i = 0; i < ln; ++i) {
      if (!PLEFTEVENTLISTENER->_http_listeners[i]) {
        continue;
      }

      RUN_TASK(_http_thread_group_id,
        MAKE_SHARED(Task, \
          std::bind(&LeftEventDispatcher::dispatch_http, LeftEventDispatcher::_instance, std::placeholders::_1), \
          PLEFTEVENTLISTENER->_http_listeners[i]->_r_event_pool_id \
        )
      );
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_UDP)) {
    size_t ln = PLEFTEVENTLISTENER->_udp_listeners.size();
    for (size_t i = 0; i < ln; ++i) {
      if (!PLEFTEVENTLISTENER->_udp_listeners[i]) {
        continue;
      }

      RUN_TASK(_udp_thread_group_id,
        MAKE_SHARED(Task, \
          std::bind(&LeftEventDispatcher::dispatch_udp, LeftEventDispatcher::_instance, std::placeholders::_1), \
          PLEFTEVENTLISTENER->_udp_listeners[i]->_r_event_pool_id \
        )
      );
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_TIMER)) {
    RUN_TASK(_timer_thread_group_id,
      MAKE_SHARED(Task, \
        std::bind(&LeftEventDispatcher::dispatch_timer, LeftEventDispatcher::_instance, std::placeholders::_1), \
        PTIMER->_r_event_pool_id \
      )
    );
  }

  return 0;
}

void LeftEventDispatcher::stop() {
  ZLOG_WARN(__FILE__, __LINE__, __func__);
  _stop_flag = true;
}

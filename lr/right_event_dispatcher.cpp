#include "right_event_dispatcher.h"

#include "right_event_handler.h"
#include "utils.h"
#include "configure.h"
#include "right_event_listener.h"
#include "right_tcp_end.h"
#include "right_udp_end.h"
#include "thread_manager.h"
#include "configure_item.h"
#include "zlog.h"
#include "event_pool.h"
#include "socket.h"
#include "event.h"

std::shared_ptr<RightEventDispatcher> RightEventDispatcher::_instance = MAKE_SHARED(RightEventDispatcher);

int RightEventDispatcher::init() {
  _stop_flag = false;
  _handler   = RightEventHandler::_instance;

  int maxt;
  if (PRIGHTEVENTLISTENER->_tcp_listeners.size() > 0) {
    _tcp_thread_group_id = PTHREADMANAGER->reserve_thread_group();
    if (_tcp_thread_group_id < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
      return -1;
    }

    maxt = PCONFIGURE->get_max_thread_number(CONFIGURE_RIGHT_END_TCP);
    if (maxt < 1) {
      maxt = 1;
    }
    maxt += PRIGHTEVENTLISTENER->_tcp_listeners.size();
    PTHREADMANAGER->start(_tcp_thread_group_id, maxt);
  }

  if (PRIGHTEVENTLISTENER->_udp_listeners.size() > 0) {
    _udp_thread_group_id = PTHREADMANAGER->reserve_thread_group();
    if (_udp_thread_group_id < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
      return -1;
    }

    maxt = PCONFIGURE->get_max_thread_number(CONFIGURE_RIGHT_END_UDP);
    if (maxt < 0) {
      maxt = 1;
    }
    maxt += PRIGHTEVENTLISTENER->_udp_listeners.size();
    PTHREADMANAGER->start(_udp_thread_group_id, maxt);
  }
  
  return 0;
}

int RightEventDispatcher::dispatch() {
  for (RightTcpEnd::ptr &ga: PRIGHTEVENTLISTENER->_tcp_listeners) {
    if (!ga) {
      continue;
    }

    RUN_TASK(_tcp_thread_group_id,
      MAKE_SHARED(Task, 
        std::bind(
          &RightEventDispatcher::dispatch_tcp,
          RightEventDispatcher::_instance,
          std::placeholders::_1
        ),
        ga->_r_event_pool_id
      )
    );
  }

  for (RightUdpEnd::ptr &ga: PRIGHTEVENTLISTENER->_udp_listeners) {
    if (!ga) {
      continue;
    }
    RUN_TASK(_udp_thread_group_id,
      MAKE_SHARED(Task, 
        std::bind(
          &RightEventDispatcher::dispatch_udp,
          RightEventDispatcher::_instance,
          std::placeholders::_1
        ),
        ga->_r_event_pool_id
      )
    );
  }
  return 0;
}

int RightEventDispatcher::dispatch_tcp(int pid) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, pid);
  Event::ptr e;
  while (!_stop_flag) {
    e = GET_EVENT(pid);
    if (e) {
      ZLOG_DEBUG(__FILE__, __LINE__, __func__, e->_stype, pid);
      RUN_TASK(_tcp_thread_group_id, 
        MAKE_SHARED(Task, std::bind(&RightEventHandler::handle, _handler, std::placeholders::_1), e)
      );
    }
  }
  return 0;
}

int RightEventDispatcher::dispatch_udp(int pid) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, pid);
  Event::ptr e;
  while (!_stop_flag) {
    e = GET_EVENT(pid);
    if (e) {
      ZLOG_DEBUG(__FILE__, __LINE__, __func__, e->_stype, pid);
      RUN_TASK(_udp_thread_group_id,
        MAKE_SHARED(Task, std::bind(&RightEventHandler::handle, _handler, std::placeholders::_1), e)
      );
    }
  }

  return 0;
}

void RightEventDispatcher::stop() {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);

  _stop_flag = true;
}

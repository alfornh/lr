#include "left_object.h"

#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <vector>
#include <string>

#include "configure.h"
#include "event.h"
#include "event_pool.h"
#include "left_event_handler.h"
#include "left_tcp_end.h"
#include "left_event_listener.h"
#include "left_event_dispatcher.h"
#include "left_event_handler.h"
#include "signals.h"
#include "thread_manager.h"
#include "timer.h"
#include "utils.h"
#include "zlog.h"

std::shared_ptr<LeftObject> LeftObject::_instance = std::make_shared<LeftObject>();

LeftObject::LeftObject() {
  _stop_flag = false;
}

int LeftObject::init() {
  srand(time(0));

  int ret = PZLOG->init();
  if ( ret < 0 ) {
    std::cout << "zlog init error" << std::endl;
    return ret;
  }

  ret = PCONFIGURE->init();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "configure init failed");
    return ret;
  }

  PZLOG->set_log_level(PCONFIGURE->get_log_level());

  ret = PTHREADMANAGER->init();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "thread manager init failed");
    return ret;
  }

  ret = PLEFTEVENTLISTENER->init();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "left listener init failed");
    return ret;
  }

  ret = PSIGNALS->init();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "Signals::init error");
    return ret;
  }

  ret = PLEFTEVENTDISPATCHER->init();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "left dispatcher init failed");
    return ret;
  }

  return 0;
}

void LeftObject::set_event_handlers(
  std::shared_ptr<Handler> tcp_handler,
  std::shared_ptr<Handler> http_handler,
  std::shared_ptr<Handler> websocket_handler,
  std::shared_ptr<Handler> udp_handler,
  std::shared_ptr<Handler> timer_handler,
  std::shared_ptr<Handler> signal_handler ) {

  LeftEventHandler::set_handlers(
    tcp_handler,
    http_handler,
    websocket_handler,
    udp_handler,
    timer_handler,
    signal_handler
  );
}

int LeftObject::run() {
  int ret = PLEFTEVENTDISPATCHER->dispatch();
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "left dispatch");
    return -1;
  }
  ret = PLEFTEVENTLISTENER->listen();
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "left listen");
    return -1;
  }
  return 0;
}

int LeftObject::dispatch_signal() {
  ZLOG_INFO(__FILE__, __LINE__, __func__);
  Event::ptr e;
  while ( !_stop_flag ) {
    e = GET_EVENT(PSIGNALS->_r_event_pool_id);
    if (e) {
      ZLOG_DEBUG(__FILE__, __LINE__, __func__);
      RUN_TASK(PSIGNALS->_thread_group_id, 
        MAKE_SHARED(Task, std::bind(&LeftEventHandler::handle, PLEFTEVENTHANDLER, std::placeholders::_1), e)
      );
    }
  }
  return 0;
}


int LeftObject::wait() {

  dispatch_signal();

  ZLOG_INFO(__FILE__, __LINE__, __func__, "server stopped!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1");
  return 0;
}


void LeftObject::stop() {
  ZLOG_INFO(__FILE__, __LINE__, __func__);
  _stop_flag = true;

  PSIGNALS->stop();

  PTHREADMANAGER->stop();

  PLEFTEVENTLISTENER->stop();

  PLEFTEVENTDISPATCHER->stop();

  EventPool::stop();
}



#include "right_object.h"

#include "handler.h"
#include "utils.h"
#include "right_event_listener.h"
#include "configure.h"
#include "right_event_dispatcher.h"
#include "zlog.h"
#include "right_event_handler.h"


RightObject::ptr RightObject::_instance = MAKE_SHARED(RightObject);

int RightObject::init() {

  int ret = PRIGHTEVENTLISTENER->init();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "right listener init");
    return ret;
  }

  ret = PRIGHTEVENTDISPATCHER->init();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "right dispatcher init");
    return ret;
  }

  return 0;
}

void RightObject::set_event_handlers(
  std::shared_ptr<Handler> tcp_handler,
  std::shared_ptr<Handler> udp_handler ) {

  RightEventHandler::set_handlers (
    tcp_handler,
    udp_handler
  );
}

int RightObject::run() {
  int ret = PRIGHTEVENTDISPATCHER->dispatch();
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "right event dispatch");
    return -1;
  }
  ret = PRIGHTEVENTLISTENER->listen();
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "right event listener");
    return -1;
  }
  return 0;
}

void RightObject::stop() {
  PRIGHTEVENTLISTENER->stop();
  PRIGHTEVENTDISPATCHER->stop();
}


#include "timer_handler.h"

#include "zlog.h"
#include "utils.h"
#include "timer.h"
#include "socket.h"
#include "event.h"
#include "event_type.h"
#include "listener.h"
#include "right_tcp_end.h"

int TimerHandler::handle(EVENTID id, std::shared_ptr<Event> event) {
  switch (id & EVENT_SUB_TYPE_MASK) {
  case EVENT_SUBTYPE_HEART_BEAT:
    return heart_beat(event);
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown timer id", id);
  }
  return 0;
}

int TimerHandler::heart_beat(std::shared_ptr<Event> ev) {
  Socket::ptr right = STATIC_CAST(Socket, ev->_es);
  if (!right) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "heart beat socket null");
    return -1;
  }

  Socket::ptr hb = STATIC_CAST(RightTcpEnd, right->_line)->make_right();
//  if (false == right->_line->online() ) {
//    ZLOG_WARN(__FILE__, __LINE__, __func__, "heart beat socket offline");
//    right->_line->reconnect_main_socket();
//    return -1;
//  }
//
  if ( hb ) {
    hb->esend("heart happy");

  } else {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "make right fail");
  }

  int HEART_BEAT_SPAN = 30;
  ev->_es = hb;
  PTIMER->add(time(0) + HEART_BEAT_SPAN, ev);

  return 0;
}



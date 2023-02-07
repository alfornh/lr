#include "configure.h"
#include "left_object.h"
#include "left_tcp_handler.h"
#include "left_udp_handler.h"
#include "right_object.h"
#include "right_udp_handler.h"
#include "right_tcp_handler.h"
#include "signals_handler.h"
#include "timer_handler.h"
#include "utils.h"

int main(int argc, char **argv) {
  int ret = PLEFTOBJECT->init();
  if ( ret < 0 ) {
    return -1;
  }

  PCONFIGURE->_tcp_accept_event_flag = true;

  PLEFTOBJECT->set_event_handlers(
    MAKE_SHARED(LeftTcpHandler),
    MAKE_SHARED(Handler),
    MAKE_SHARED(Handler),
    MAKE_SHARED(LeftUdpHandler),
    MAKE_SHARED(TimerHandler),
    MAKE_SHARED(SignalsHandler)
  );

  ret = PRIGHTOBJECT->init();
  if ( ret < 0 ) {
    return -1;
  }

  PRIGHTOBJECT->set_event_handlers(
    MAKE_SHARED(RightTcpHandler),
    MAKE_SHARED(RightUdpHandler)
  ); 

  ret = PRIGHTOBJECT->run();
  if (ret < 0) {
    return -1;
  }

  ret = PLEFTOBJECT->run();
  if ( ret < 0 ) {
    return -1;
  }

  ret = PLEFTOBJECT->wait();
  if ( ret < 0 ) {
    return -1;
  }

  return 0;
}

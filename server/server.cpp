#include "left_object.h"

#include "left_tcp_handler.h"
#include "left_udp_handler.h"
#include "left_http_handler.h"
#include "left_websocket_handler.h"
#include "timer_handler.h"
#include "signals_handler.h"

#include "thread_manager.h"
#include "zlog.h"
#include "utils.h"

int main(int argc, char **argv) {

  int ret = PLEFTOBJECT->init();
  if ( ret < 0 ) {
    return -1;
  }

  PLEFTOBJECT->set_event_handlers(
    MAKE_SHARED(LeftTcpHandler),
    MAKE_SHARED(LeftHttpHandler),
    MAKE_SHARED(LeftWebSocketHandler),
    MAKE_SHARED(LeftUdpHandler),
    MAKE_SHARED(TimerHandler),
    MAKE_SHARED(SignalsHandler)
  );

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

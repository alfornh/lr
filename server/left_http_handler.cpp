#include "left_http_handler.h"

#include <time.h>

#include "data_buffer.h"
#include "event.h"
#include "socket.h"
#include "timer.h"
#include "utils.h"
#include "zlog.h"

int LeftHttpHandler::handle(EVENTID id, std::shared_ptr<Event> ev) {
  char req[512] = {0};
  STATIC_CAST(Socket, ev->_es)->get_r_data(req, sizeof(req));
  ZLOG_INFO(__FILE__, __LINE__, __func__, req); 

  Event::ptr e = std::make_shared<Event>();
  e->_stype = EVENT_TYPE_TIMER;
  Timer::_instance->add(time(0) + 10, e);

  return 0;
}

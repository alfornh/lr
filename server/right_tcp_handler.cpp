#include "right_tcp_handler.h"

#include "data_buffer.h"
#include "event.h"
#include "zlog.h"

int RightTcpHandler::handle(EVENTID id, std::shared_ptr<Event> event) {
  //char req[1024] = {0};
  //ev->_db->get(req, sizeof(req));
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, id);
  return 0;
}

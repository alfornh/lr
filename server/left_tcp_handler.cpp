#include "left_tcp_handler.h"

#include "data_buffer.h"
#include "event.h"
#include "socket.h"
#include "utils.h"
#include "zlog.h"

int LeftTcpHandler::handle(EVENTID id, std::shared_ptr<Event> ev) {
  char buf[128] = {0};
  Socket::ptr socket = STATIC_CAST(Socket, ev->_es);
  socket->move_r_data(buf, sizeof(buf));
  
  ZLOG_INFO(__FILE__, __LINE__, __func__, id, buf);
  socket->add_w_data(buf, sizeof(buf));
  
  return socket->esend();
}

#include "left_udp_handler.h"

#include "data_buffer.h"
#include "event.h"
#include "socket.h"
#include "utils.h"
#include "zlog.h"

int LeftUdpHandler::handle(EVENTID id, std::shared_ptr<Event> event) {
  Socket::ptr socket = STATIC_CAST(Socket, event->_es);
  char req[128] = {0};
  STATIC_CAST(Socket, event->_es)->get_r_data(req, sizeof(req));
  ZLOG_INFO(__FILE__, __LINE__, __func__, "req: ", req); 

  char res[512] = {0};
  sprintf(res, "%s:%d %s", socket->_line->_main_socket->_ipi._ip.c_str(), socket->_line->_main_socket->_ipi._port, req);
  SEND_TO(event->_es, res, strlen(res));
  return 0;
}

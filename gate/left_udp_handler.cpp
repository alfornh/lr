#include "left_udp_handler.h"

#include "event.h"
#include "zlog.h"
#include "socket.h"
#include "data_buffer.h"
#include "utils.h"
#include "from_and_to.h"

int LeftUdpHandler::handle(EVENTID id, std::shared_ptr<Event> event) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, id);
  switch (id & EVENT_SUB_TYPE_MASK) {
  case EVENT_SUBTYPE_READ:
    return handle_read(event);
  break;

  case EVENT_SUBTYPE_CLOSE:
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown event id", id);
  }

  return 0;
}

int LeftUdpHandler::handle_read(std::shared_ptr<Event> &ev) {
  ZLOG_INFO(__FILE__, __LINE__, __func__);
  Socket::ptr socket = STATIC_CAST(Socket, ev->_es);

  Socket::ptr client = PFROMANDTO->get_right_by_left_ip(IPAddress(socket->_ipi->_ip, socket->_ipi->_port));
  if (!client) {
    client = PFROMANDTO->make_right_by_left(socket);
    if (client) {
      PFROMANDTO->set_left_ip_and_right_id(socket, client);
    } else {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "drop event", ev->_stype);
      return -1;
    }
  } else {
    PFROMANDTO->update_left_ip_and_right_id(socket, client);
  }

  LOCK_GUARD_MUTEX_BEGIN(socket->_mutex_r_db)
    if (socket->_r_db->_len < 1) {
      return 0;
    }

    LOCK_GUARD_MUTEX_BEGIN(client->_mutex_w_db)

      client->_w_db->add(socket->_r_db);
      socket->_r_db->clear();

    LOCK_GUARD_MUTEX_END
  LOCK_GUARD_MUTEX_END

  return client->esend();
}

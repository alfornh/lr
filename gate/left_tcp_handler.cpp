#include "left_tcp_handler.h"

#include "event.h"
#include "zlog.h"
#include "utils.h"
#include "from_and_to.h"
#include "socket.h"
#include "data_buffer.h"

int LeftTcpHandler::handle(EVENTID id, std::shared_ptr<Event> ev) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, id);
  switch (id & EVENT_SUB_TYPE_MASK) {
  //case EVENT_SUBTYPE_ACCEPT:
  case EVENT_SUBTYPE_READ:
    return handle_read(ev);
  break;
  //case EVENT_SUBTYPE_WRITE: // this case will not be in here
  //break;

  case EVENT_SUBTYPE_CLOSE:
    return handle_close(ev);
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown event id", id);
  }

  return -1;
}

int LeftTcpHandler::handle_read(std::shared_ptr<Event> &ev) {
  ZLOG_INFO(__FILE__, __LINE__, __func__);
  Socket::ptr socket = STATIC_CAST(Socket, ev->_es);

  Socket::ptr client = PFROMANDTO->GET_RIGHT_BY_LEFTID(socket->_id);
  if (!client) {
    client = PFROMANDTO->make_right_by_left(socket);
    if (client) {
      PFROMANDTO->SET_LEFT_AND_RIGHT(socket, client);
    } else {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "drop event", ev->_stype);
      return -1;
    }
  }

  LOCK_GUARD_MUTEX_BEGIN(socket->_mutex_r_db)
    if (socket->_r_db->_len < 1) {
      return 0;
    }

    LOCK_GUARD_MUTEX_BEGIN(client->_mutex_w_db)
      client->_w_db->add(socket->_r_db);
    LOCK_GUARD_MUTEX_END

    socket->_r_db->clear();
  LOCK_GUARD_MUTEX_END

  return client->esend();
}

int LeftTcpHandler::handle_close(std::shared_ptr<Event> &ev) {
  ZLOG_INFO(__FILE__, __LINE__, __func__);
  Socket::ptr socket = STATIC_CAST(Socket, ev->_es);
  PFROMANDTO->DEL_ENTRY_BY_LEFTID(socket->_id);

  return 0;
}

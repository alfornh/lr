#include "right_tcp_handler.h"

#include "data_buffer.h"
#include "event.h"
#include "zlog.h"
#include "utils.h"
#include "from_and_to.h"
#include "socket.h"
#include "timer.h"

int RightTcpHandler::handle(EVENTID id, std::shared_ptr<Event> event) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, id);
  switch (id & EVENT_SUB_TYPE_MASK) {
  case EVENT_SUBTYPE_READ:
    return handle_read(event);
  break;
  case EVENT_SUBTYPE_CLOSE:
    return handle_close(event);
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown id", id);
    return -1;
  }
}

int RightTcpHandler::handle_read(std::shared_ptr<Event> &ev) {

  Socket::ptr right = STATIC_CAST(Socket, ev->_es);

  Socket::ptr left = PFROMANDTO->GET_LEFT_BY_RIGHTID(right->_id);
  if (!left) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "drop event", ev->_stype);
    return -1;
  }

  LOCK_GUARD_MUTEX_BEGIN(right->_mutex_r_db)
    LOCK_GUARD_MUTEX_BEGIN(left->_mutex_w_db)
      left->_w_db->add(right->_r_db);
    LOCK_GUARD_MUTEX_END

    right->_r_db->clear();
  LOCK_GUARD_MUTEX_END

  return left->esend();

}

int RightTcpHandler::handle_close(std::shared_ptr<Event> &ev) {
  Socket::ptr right = STATIC_CAST(Socket, ev->_es);

  PFROMANDTO->DEL_ENTRY_BY_RIGHTID(right->_id);

  return 0;
}


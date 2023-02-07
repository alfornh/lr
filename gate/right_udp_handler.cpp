#include "right_udp_handler.h"

#include "data_buffer.h"
#include "event.h"
#include "zlog.h"
#include "from_and_to.h"
#include "socket.h"
#include "utils.h"

int RightUdpHandler::handle(EVENTID id, std::shared_ptr<Event> event) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, id);

  Socket::ptr right = STATIC_CAST(Socket, event->_es);

  Socket::ptr left = PFROMANDTO->get_left_by_right_id(right->_id);
  if (!left) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "drop event", id);
    return -1;
  }

  //del_entry_by_right_id(right->_id);

  LOCK_GUARD_MUTEX_BEGIN(right->_mutex_r_db)
    LOCK_GUARD_MUTEX_BEGIN(left->_mutex_w_db)

      left->_w_db->add(right->_r_db);
      right->_r_db->clear();

    LOCK_GUARD_MUTEX_END
  LOCK_GUARD_MUTEX_END

  return left->esend();
}

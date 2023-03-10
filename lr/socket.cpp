#include "socket.h"

#include <fcntl.h>
//#include <sys/socket.h>
#include <sys/types.h>
#include <string>
//#include <unistd.h>

#include "plt/net-inc.h"

#include "data_buffer.h"
#include "id_set.h"
#include "zlog.h"
#include "utils.h"
#include "event.h"
#include "event_pool.h"
#include "listener.h"

Socket::Socket() {
  _socket_status = SOCKET_STATUS_NULL;
  _fd = 0;
  _id = INVALIDID;
  _ipi = MAKE_SHARED(IPInfo);
  _r_db = MAKE_SHARED(DataBuffer);
  _w_db = MAKE_SHARED(DataBuffer);
}

Socket::~Socket() {
  _socket_status = SOCKET_STATUS_CLOSE;
  if (_fd > 0) {
    //close(_fd);
    socket_close(_fd);
  }
  PSIDSET->put(_id);
}

SOCKETID Socket::sign_socket_id() {
  return PSIDSET->get();
}

int Socket::vinit(std::shared_ptr<IPInfo> ) { return 0; }
int Socket::vbind() { return 0; }
std::shared_ptr<Socket> Socket::vaccept() { return NULL; }
int Socket::vconnect() { return 0; }
int Socket::vlisten() { return 0; }
int Socket::vrecv() { return 0; }
int Socket::vsend(const char *buf, const int blen) { return 0; }
int Socket::vclose() { return 0;}

int Socket::nonblock(bool nb) {
  return socket_nonblock(_fd, nb);
}

int Socket::reuseaddr(bool ra) {
  return socket_reuse(_fd, ra);
}

int Socket::ssend() {
  int len;
  std::vector<char> d;
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  if (_w_db->_len < 1) {
    return 0;
  }

  if (_w_db->_data.size() == 1) {
    BufferItem::ptr bi = _w_db->_data.front();
    if (bi) {
      len = vsend(bi->_buffer + bi->_spos, bi->_len);
    }

  } else {

    _w_db->get(d);

    len = vsend(&(*d.begin()), d.size());
  }

  _w_db->drop(len);

  //return the number of remained bytes
  return _w_db->_len;

  LOCK_GUARD_MUTEX_END
}

int Socket::esend(int cflag) {
  int ret = _line->notify_w_event(_id, _fd);
  if (cflag) {
    vclose();
  } 
  return ret;
}

int Socket::esend(const char *buf, const int len, int cflag) {
  int ret;
  add_w_data(buf, len);
  ret = _line->notify_w_event(_id, _fd);
  if (cflag) {
    vclose();
  }
  return ret;
}

int Socket::esend(const std::string &str, int cflag) {
  int ret;
  add_w_data(str);
  
  ret = _line->notify_w_event(_id, _fd);
  if (cflag) {
    vclose();
  }
  return ret;
}

int Socket::esend(const DataBuffer &db, int cflag) {
  int ret;
  add_w_data(db);
  ret = _line->notify_w_event(_id, _fd);
  if (cflag) {
    vclose();
  }
  return ret;
}

int Socket::esend(const std::shared_ptr<DataBuffer> &db, int cflag) {
  int ret;
  add_w_data(db);
  ret = _line->notify_w_event(_id, _fd);
  if (cflag) {
    vclose();
  }
  return ret;
}

int Socket::esend(const std::shared_ptr<BufferItem> &bi, int cflag) {
  int ret;
  add_w_data(bi);
  ret = _line->notify_w_event(_id, _fd);
  if (cflag) {
    vclose();
  }
  return ret;
}

int Socket::add_w_data(const char *buf, const int len) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  _w_db->add(buf, len);
  LOCK_GUARD_MUTEX_END
  return len;
}

int Socket::add_w_data(const std::string &str) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  _w_db->add(str);
  LOCK_GUARD_MUTEX_END
  return str.size();
}

int Socket::add_w_data(const DataBuffer &db) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  _w_db->add(db);
  LOCK_GUARD_MUTEX_END
  return db._len;
}

int Socket::add_w_data(const std::shared_ptr<DataBuffer> &db) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  _w_db->add(db);
  LOCK_GUARD_MUTEX_END
  return db->_len;
}

int Socket::add_w_data(const std::shared_ptr<BufferItem> &bi) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  _w_db->add(bi);
  LOCK_GUARD_MUTEX_END
  return bi->_len;
}

int Socket::get_w_data(char *buf, const int len) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  return _w_db->get(buf, len);
  LOCK_GUARD_MUTEX_END
}

int Socket::get_w_data(DataBuffer &db) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  return _w_db->get(db);
  LOCK_GUARD_MUTEX_END
}

int Socket::get_w_data(std::shared_ptr<DataBuffer> &db) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  return _w_db->get(db);
  LOCK_GUARD_MUTEX_END
}

int Socket::move_w_data(char *buf, const int len) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  return _w_db->move(buf, len);
  LOCK_GUARD_MUTEX_END
}

int Socket::move_w_data(std::vector<char> &v) {
  int ret;
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  ret = _w_db->get(v);
  _w_db->clear();
  LOCK_GUARD_MUTEX_END
  return ret;
}

void Socket::clear_w_data() {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_db)
  _w_db->clear();
  LOCK_GUARD_MUTEX_END
}

int Socket::move_r_data(char *buf, const int len) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_db)
  return _r_db->move(buf, len);
  LOCK_GUARD_MUTEX_END
}

void Socket::clear_r_data() {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_db)
  _r_db->clear();
  LOCK_GUARD_MUTEX_END
  return;
}

int Socket::get_r_data(char *buf, const int len) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_db)
  return _r_db->get(buf, len);
  LOCK_GUARD_MUTEX_END
}

int Socket::get_r_data(DataBuffer &db) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_db)
  return _r_db->get(db);
  LOCK_GUARD_MUTEX_END
}

int Socket::get_r_data(std::shared_ptr<DataBuffer> &db) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_db)
  return _r_db->get(db);
  LOCK_GUARD_MUTEX_END
}

int Socket::move_r_data(std::vector<char> &v) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_db)
  return _r_db->move(v);
  LOCK_GUARD_MUTEX_END
}

int Socket::move_r_data(DataBuffer &db) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_db)
  return _r_db->move(db);
  LOCK_GUARD_MUTEX_END
}

int Socket::move_r_data(std::shared_ptr<DataBuffer> &db) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_db)
  return _r_db->move(db);
  LOCK_GUARD_MUTEX_END
}

int Socket::add_r_data(const std::shared_ptr<BufferItem> &bi) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_db)
  _r_db->add(bi);
  LOCK_GUARD_MUTEX_END
  return bi->_len;
}

int Socket::add_r_data(const char *buf, const int len) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_db)
  _r_db->add(buf, len);
  LOCK_GUARD_MUTEX_END
  return len;
}

#include "from_and_to.h"

#include <stdlib.h>
#include <time.h>

#include "event_type.h"
#include "from_and_to.h"
#include "right_tcp_end.h"
#include "right_udp_end.h"
#include "right_event_listener.h"
#include "tcp_socket.h"
#include "udp_socket.h"
#include "utils.h"

std::shared_ptr<FromAndTo> FromAndTo::_instance = MAKE_SHARED(FromAndTo);

std::shared_ptr<Socket> FromAndTo::make_right_by_left(std::shared_ptr<Socket> left) {
  Socket::ptr s = Socket::ptr();
  switch (left->_stype) {
  case EVENT_TYPE_SOCKET_TCP:
    return make_tcp_right(left);
  break;

  case EVENT_TYPE_SOCKET_UDP:
    return make_udp_right(left);
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown stype", left->_stype);
  }

  return s;
}

std::shared_ptr<Socket> FromAndTo::make_tcp_right(std::shared_ptr<Socket> left) {
  Socket::ptr s;
  RightTcpEndContainer &tcpends = PRIGHTEVENTLISTENER->_tcp_listeners;

  int len = tcpends.size();
  if (len < 1) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "no right tcp ends available");
    return s;
  }
  int index = random() % len;
  s = tcpends[index]->make_right();
  if (s) {
    return s;
  }

  int i = index + 1;

  while ( (i % len) != index ) {
    s = tcpends[ i % len ]->make_right();
    if ( s ) {
      return s;
    }
    i++;
  }

  ZLOG_ERROR(__FILE__, __LINE__, __func__, "no tcp right end available");
  return s;
}

std::shared_ptr<Socket> FromAndTo::make_udp_right(std::shared_ptr<Socket> left) {
  RightUdpEndContainer &udpends = PRIGHTEVENTLISTENER->_udp_listeners;
  Socket::ptr s;

  int len = udpends.size();
  if (len < 1) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "no right udp end available");
    return s;
  }

  int index = random() % len;
  s = udpends[index]->make_right();
  if (s) {
    return s;
  }

  int i = index + 1;

  while ( (i % len) != index ) {
    s = udpends[ i % len ]->make_right();
    if ( s ) {
      return s;
    }
    i++;
  }

  ZLOG_ERROR(__FILE__, __LINE__, __func__, "no udp right end available");
  return s;
}


Socket::ptr FromAndTo::GET_RIGHT_BY_LEFTID(SOCKETID __lid__) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, __lid__);
  Socket::ptr __right__;
  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_and_right_idx)
  __right__ = _right_by_left_index[(__lid__)].lock();
  LOCK_GUARD_MUTEX_END
  return __right__;
}


std::shared_ptr<Socket> FromAndTo::GET_LEFT_BY_RIGHTID(SOCKETID __rid__) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, __rid__);
  Socket::ptr __left__;
  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_and_right_idx)
  __left__ = _left_by_right_index[(__rid__)].lock();
  LOCK_GUARD_MUTEX_END
  return __left__;
}


void FromAndTo::SET_LEFT_AND_RIGHT(Socket::ptr __left__, Socket::ptr __right__) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_and_right_idx)
  _left_by_right_index[(__right__)->_id] = __left__ ;
  _right_by_left_index[(__left__)->_id] = __right__ ;
  LOCK_GUARD_MUTEX_END
}


void FromAndTo::DEL_ENTRY_BY_RIGHTID(SOCKETID __rid__) {                                                        
  Socket::ptr sk;
  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_and_right_idx)
  RIGHT_LEFT_CONTAINER::iterator it = _left_by_right_index.find((__rid__));
  if (it != _left_by_right_index.end()) {
    sk = it->second.lock();
    _left_by_right_index.erase(it);
  }
  if (sk) {
    _right_by_left_index.erase(sk->_id);
  }
  LOCK_GUARD_MUTEX_END
}                                                                                                   


void FromAndTo::DEL_ENTRY_BY_LEFTID(SOCKETID __lid__) {
  Socket::ptr client;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_and_right_idx);
  RIGHT_LEFT_CONTAINER::iterator it = _right_by_left_index.find((__lid__));
  if (it != _right_by_left_index.end()) {
    client = it->second.lock();
    _right_by_left_index.erase(it);
  }
  if (client) {
    _left_by_right_index.erase(client->_id);
  }

  LOCK_GUARD_MUTEX_END
} 


void FromAndTo::set_left_ip_and_right_id(Socket::ptr left, Socket::ptr right) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_ip_and_right_id)
  _right_by_left_ip_index[IPAddress(left->_ipi->_ip, left->_ipi->_port)] = right;
  _left_by_right_id_index[right->_id] = left;
  LOCK_GUARD_MUTEX_END
}

std::shared_ptr<Socket> FromAndTo::get_right_by_left_ip(IPAddress ip) {
  Socket::ptr sock;
  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_ip_and_right_id)
  IP_LEFT_RIGHT_CONTAINER::iterator it = _right_by_left_ip_index.find(ip);
  if (it == _right_by_left_ip_index.end()) {
    return sock;
  }
  sock = it->second.lock();
  LOCK_GUARD_MUTEX_END

  return sock;
}

std::shared_ptr<Socket> FromAndTo::get_left_by_right_id(SOCKETID id) {
  Socket::ptr sock;
  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_ip_and_right_id)
  ID_RIGHT_LEFT_CONTAINER::iterator it = _left_by_right_id_index.find(id);
  if (it == _left_by_right_id_index.end()) {
    return sock;
  }
  sock = it->second;
  LOCK_GUARD_MUTEX_END
  return sock;
}

void FromAndTo::del_entry_by_left_ip(IPAddress ip) {
  Socket::ptr sock;
  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_ip_and_right_id)
  IP_LEFT_RIGHT_CONTAINER::iterator it = _right_by_left_ip_index.find(ip);
  if (it == _right_by_left_ip_index.end()) {
    return ;
  }
  sock = it->second.lock();
  _right_by_left_ip_index.erase(it);
  _left_by_right_id_index.erase(sock->_id);
  LOCK_GUARD_MUTEX_END
}

void FromAndTo::del_entry_by_right_id(SOCKETID id) {
  Socket::ptr sock;
  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_ip_and_right_id)
  ID_RIGHT_LEFT_CONTAINER::iterator it = _left_by_right_id_index.find(id);
  if (it == _left_by_right_id_index.end()) {
    return;
  }
  sock = it->second;
  _left_by_right_id_index.erase(it);
  _right_by_left_ip_index.erase(IPAddress(sock->_ipi->_ip, sock->_ipi->_port));
  LOCK_GUARD_MUTEX_END
}

void FromAndTo::update_left_ip_and_right_id(Socket::ptr left, Socket::ptr right) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_left_ip_and_right_id)
  _right_by_left_ip_index[IPAddress(left->_ipi->_ip, left->_ipi->_port)] = right;
  _left_by_right_id_index[right->_id] = left;
  LOCK_GUARD_MUTEX_END
}

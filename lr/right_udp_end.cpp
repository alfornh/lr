#include "right_udp_end.h"

#include <string>

#include "plt/net-inc.h"

#include "configure.h"
#include "data_buffer.h"
#include "event.h"
#include "event_pool.h"
#include "reactor.h"
//#include "reactor_epoll.h"
#include "reactor_select.h"
#include "thread_manager.h"
#include "timer.h"
#include "udp_socket.h"
#include "utils.h"
#include "zlog.h"

int RightUdpEnd::init() {
  _stop_flag = false;

  switch (_ipi->_protocal) {
  case PROTOCAL_UDP:
    _stype = EVENT_TYPE_SOCKET_UDP;
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown source type ", _stype);
    return -1;
  }

  _r_event_pool_id = EventPool::reserve_event_queue();

  if (PCONFIGURE->is_key_equal_value("right_reactor", "async")) {
    _reactor = MAKE_SHARED(AsyncIOMultiplex, Reactor::PROTOCOL_UDP);
  } else {
    _reactor = MAKE_SHARED(SelectReactor, Reactor::PROTOCOL_UDP);
  }

  _reactor->_line = shared_from_this();

  if (_reactor->_init(_ipi) < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "_reactor init");
    return -1;
  }

  return 0;
}

int RightUdpEnd::notify_w_event(SOCKETID id, int fd) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, id, fd);
  return _reactor->notify_w_event(id, fd);
}

int RightUdpEnd::listen() {
  return _reactor->_listen();
}

int RightUdpEnd::stop() {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  _stop_flag = false;
  _reactor->_stop();
  return 0;
}

int RightUdpEnd::l_recv(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  int ret;

  UdpSocket::ptr sock;//= _main_socket->vurecv();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  SocketContainer::iterator it = _sockets.find(sid);
  if ( it == _sockets.end()) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket not found", sid);
    return -1;
  }
  sock = it->second;
  LOCK_GUARD_MUTEX_END

  ret = sock->vrecv();
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "udpsocket recv error");
    return -1;
  }

  Event::ptr event = MAKE_SHARED(Event);
  event->_es = sock;
  event->_stype = _stype | EVENT_SUBTYPE_READ;

  ADD_EVENT(_r_event_pool_id, event);
  return ret;
}

int RightUdpEnd::l_recv(SOCKETID sid, std::shared_ptr<BufferItem> bi, void* opt = NULL) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);

  if (!opt) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "null opt");
      return -1;
  }
  sockaddr_in* addrin = (sockaddr_in*)opt;

  UdpSocket::ptr sock = MAKE_SHARED(UdpSocket, _stype);
  sock->_id = Socket::sign_socket_id();
  sock->_fd = 0;//dup(_fd);
  sock->_line = shared_from_this();
  sock->_ipi->_ip = std::string(inet_ntoa(addrin->sin_addr));
  sock->_ipi->_port = ntohs(addrin->sin_port);
  sock->_socket_status = SOCKET_STATUS_ACTIVE;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  _sockets[sock->_id] = sock;
  LOCK_GUARD_MUTEX_END

  int ret = sock->add_r_data(bi);

  Event::ptr event = MAKE_SHARED(Event);
  event->_es = sock;
  event->_stype = _stype | EVENT_SUBTYPE_READ;

  ADD_EVENT(_r_event_pool_id, event);
  return ret;
}

int RightUdpEnd::l_write(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  UdpSocket::ptr sock = UdpSocket::ptr();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)

  SocketContainer::iterator it = _sockets.find(sid);
  if ( it == _sockets.end()) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket not found", sid);
    return -1;
  }
  sock = it->second;

  LOCK_GUARD_MUTEX_END

  int ret = sock->ssend();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "udp socket ssend");
  }

  return ret;
}

int RightUdpEnd::l_close(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);

  Socket::ptr sock;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)

  SocketContainer::iterator it = _sockets.find(sid);
  if ( it == _sockets.end()) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket not found", sid);
    return -1;
  }
  sock = it->second;
  _sockets.erase(it);

  LOCK_GUARD_MUTEX_END

  _reactor->_del(sid, sock->_fd);
  sock->vclose();

  Event::ptr event = std::make_shared<Event>();
  event->_es = sock;
  event->_stype = _stype | EVENT_SUBTYPE_CLOSE;
  ADD_EVENT(_r_event_pool_id, event);

  return 0;
}

std::shared_ptr<UdpSocket> RightUdpEnd::make_right() {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  UdpSocket::ptr c;
  int ret;

  c = MAKE_SHARED(UdpSocket, EVENT_TYPE_SOCKET_UDP);
  c->_id = Socket::sign_socket_id();
  c->_line = shared_from_this();
  if (c->_id < 1) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "no socket id available");
    return c;
  }

  if (c->vinit(_ipi) < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket init");
    return UdpSocket::ptr();
  }

  ret = c->nonblock(false);
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "nonblock");
    return UdpSocket::ptr();
  }

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)

  SocketContainer::iterator it = _sockets.find(c->_id);
  if (it != _sockets.end()) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "repeated socket id ", c->_id);
    return UdpSocket::ptr();
  }
  _sockets[c->_id] = c;

  LOCK_GUARD_MUTEX_END

  _reactor->_add(c->_id, c->_fd);

  return c; 
}

//void RightUdpEnd::start_heart_beat() {
//  Event::ptr e = MAKE_SHARED(Event);
//  e->_es = _main_socket;
//  e->_stype = EVENT_TYPE_TIMER | EVENT_SUBTYPE_HEART_BEAT;
//  PTIMER->add(time(0) + HEART_BEAT_SPAN, e);
//}


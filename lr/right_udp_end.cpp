#include "right_udp_end.h"

#include <string>

#include "configure.h"
#include "data_buffer.h"
#include "event.h"
#include "event_pool.h"
#include "reactor.h"
#include "reactor_epoll.h"
#include "reactor_select.h"
#include "tcp_socket.h"
#include "thread_manager.h"
#include "timer.h"
#include "udp_socket.h"
#include "utils.h"
#include "zlog.h"

int RightUdpEnd::init() {
  _stop_flag = false;

  switch (__ipi._protocal) {
  case PROTOCAL_UDP:
    _stype = EVENT_TYPE_SOCKET_UDP;
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown source type ", _stype);
    return -1;
  }

  _r_event_pool_id = EventPool::reserve_event_queue();

  Value::ptr v = PCONFIGURE->get_value("right_reactor");
  if (v && v->_v == "epoll") {
    _reactor = MAKE_SHARED(EpollReactor);
  } else {
    _reactor = MAKE_SHARED(SelectReactor);
  }

  _reactor->_line = shared_from_this();

  if (_reactor->_init(__ipi) < 0) {
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
  if (c->_id < 1) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "no socket id available");
    return c;
  }

  if (c->vinit(__ipi) < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket init");
    return UdpSocket::ptr();
  }
 
  c->_line = shared_from_this();

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


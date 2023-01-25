#include "left_udp_end.h"

#include "configure.h"
#include "event.h"
#include "event_pool.h"
#include "ipinfo.h"
#include "reactor.h"
#include "reactor_epoll.h"
#include "reactor_select.h"
#include "socket.h"
#include "thread_manager.h"
#include "udp_socket.h"
#include "zlog.h"

int LeftUdpEnd::init() {
  _stop_flag = false;
  switch (__ipi._protocal) {
  case PROTOCAL_UDP:
    _stype = EVENT_TYPE_SOCKET_UDP;
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown protocal type", __ipi._protocal);
    return -1;
  }

  _r_event_pool_id = EventPool::reserve_event_queue();
  _main_socket = MAKE_SHARED(UdpSocket, _stype);
  _main_socket->_id = Socket::sign_socket_id();
  _main_socket->_line = shared_from_this();

  if (_main_socket->vinit(__ipi) < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "main socket init error");
    return -1;
  }

  //if (m == UDP_LINE_BLOCK) {
  _main_socket->nonblock(false);
  //} else {
  //  _main_socket->nonblock(true);
  //}

  if (_main_socket->vbind() < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "main socket bind error");
    return -1;
  }

  Value::ptr v = PCONFIGURE->get_value("left_reactor");
  if (v && v->_v == "epoll") {
    _reactor = MAKE_SHARED(EpollReactor);
  } else {
    _reactor = MAKE_SHARED(SelectReactor);
  }

  _reactor->_line = shared_from_this();

  if (_reactor->_init(__ipi) < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "reactor init error");
    return -1;
  }

  return 0;
}

int LeftUdpEnd::listen() {
  int ret = _reactor->_add(_main_socket->_id, _main_socket->_fd);
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "reactor add error");
    return ret;
  }

  return _reactor->_listen();
}

int LeftUdpEnd::notify_w_event(SOCKETID id, int fd) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, id, fd);
  return _reactor->notify_w_event(id, fd);
}

int LeftUdpEnd::l_close(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);

  if ( sid != _main_socket->_id ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__);
    return -1;
  }

  _main_socket->vclose();
  _reactor->_del(sid, _main_socket->_fd);

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  _sockets.clear();
  LOCK_GUARD_MUTEX_END

  Event::ptr event = std::make_shared<Event>();
  event->_es = _main_socket;
  event->_stype = _stype | EVENT_SUBTYPE_CLOSE;
  ADD_EVENT(_r_event_pool_id, event);

  return 0;
}

int LeftUdpEnd::l_recv(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  if (sid != _main_socket->_id) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__);
    return -1;
  }

  UdpSocket::ptr sock = STATIC_CAST(UdpSocket, _main_socket)->lrecv();
  if ( !sock ) {
    return -1;
  }

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  _sockets.insert(std::make_pair(sock->_id, sock));
  LOCK_GUARD_MUTEX_END

  Event::ptr event = MAKE_SHARED(Event);
  event->_es = sock;
  event->_stype = _stype | EVENT_SUBTYPE_READ;

  ADD_EVENT(_r_event_pool_id, event);
  return 0;
}

int LeftUdpEnd::l_write(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  UdpSocket::ptr sock = UdpSocket::ptr();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)

  SocketContainer::iterator it = _sockets.find(sid);
  if ( it == _sockets.end()) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket not found", sid);
    return -1;
  }
  sock = it->second;
  _sockets.erase(it);

  LOCK_GUARD_MUTEX_END

  int ret = STATIC_CAST(UdpSocket, _main_socket)->lsend(sock);
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket vusend");
  }

  return ret;
}

int LeftUdpEnd::stop() {
  _stop_flag = true;
  return _main_socket->vclose();
}

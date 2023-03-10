#include "left_tcp_end.h"

#include <string>

#include "plt/net-inc.h"

#include "configure.h"
#include "data_buffer.h"
#include "event.h"
#include "event_pool.h"
#include "ipinfo.h"
#include "reactor.h"
#include "reactor_select.h"
#include "tcp_socket.h"
#include "thread_manager.h"
#include "utils.h"
#include "zlog.h"

int LeftTcpEnd::init() {
  _stop_flag = false;
  switch (_ipi->_protocal) {
  case PROTOCAL_TCP:
    _stype = EVENT_TYPE_SOCKET_TCP;
  break;

  case PROTOCAL_WEBSOCKET:
    _stype = EVENT_TYPE_WEBSOCKET;
  break;

  case PROTOCAL_HTTP:
    _stype = EVENT_TYPE_HTTPSOCKET;
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown protocal type", _ipi->_protocal);
    return -1;
  }

  _r_event_pool_id = EventPool::reserve_event_queue();

  if (PCONFIGURE->is_key_equal_value("left_reactor", "async")) {
      _reactor = MAKE_SHARED(AsyncIOMultiplex, Reactor::PROTOCOL_TCP);
  } else {
      _reactor = MAKE_SHARED(SelectReactor, Reactor::PROTOCOL_TCP);
  }

  _reactor->_line = shared_from_this();

  if (_reactor->_init(_ipi) < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "reactor init error");
      return -1;
  }

  _main_socket = MAKE_SHARED(TcpSocket, _stype);
  _main_socket->_id = Socket::sign_socket_id();

  _main_socket->_line = shared_from_this();

  //_main_socket->_r_event_pool_id = _r_event_pool_id;

  if (_main_socket->vinit(_ipi) < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "vinit");
    return -1;
  }

  _main_socket->nonblock(false);
  _main_socket->reuseaddr(true);

  if (_main_socket->vbind() < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "vbind");
    return -1;
  }

  if (_main_socket->vlisten() < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "vlisten");
    return -1;
  }

  return 0;
}

int LeftTcpEnd::listen() {
  int ret = _reactor->_add(_main_socket->_id, _main_socket->_fd);
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "reactor add error");
    return ret;
  }

  return _reactor->_listen();
}

int LeftTcpEnd::notify_w_event(SOCKETID id, int fd) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, id, fd);
  return _reactor->notify_w_event(id, fd);
}

int LeftTcpEnd::l_accept(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  if (sid != _main_socket->_id) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__);
    return -1;
  }

  Socket::ptr sock = _main_socket->vaccept();
  if ( !sock ) {
    return -1;
  }

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  _sockets.insert(std::make_pair(sock->_id, sock));
  LOCK_GUARD_MUTEX_END

  _reactor->_add(sock->_id, sock->_fd);

  if (PCONFIGURE->_tcp_accept_event_flag) {
    Event::ptr event = MAKE_SHARED(Event);
    event->_es = sock;
    event->_stype = _stype | EVENT_SUBTYPE_ACCEPT;
    ADD_EVENT(_r_event_pool_id, event);
  }

  return 0;
}

int LeftTcpEnd::l_close(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  Socket::ptr socket;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  SocketContainer::iterator it = _sockets.find(sid);
  if ( it !=  _sockets.end() && sid != _main_socket->_id ) {
    socket = it->second;
    _sockets.erase(it);
  }
  LOCK_GUARD_MUTEX_END

  if ( !socket ) {
    return -1;
  }

  socket->vclose();

  _reactor->_del(sid, socket->_fd);

  Event::ptr event = std::make_shared<Event>();
  event->_es = socket;
  event->_stype = _stype | EVENT_SUBTYPE_CLOSE;

  ADD_EVENT(_r_event_pool_id, event);

  return 0;
}

int LeftTcpEnd::l_recv(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  Socket::ptr sock = Socket::ptr();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  SocketContainer::iterator it = _sockets.find(sid);
  if ( it == _sockets.end() ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket not found", sid);
    return -1;
  }

  sock = it->second;
  if (!sock || sock->_socket_status == SOCKET_STATUS_CLOSE) {
    _sockets.erase(it);
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket closed", sid);
    return -1;
  }

  LOCK_GUARD_MUTEX_END

  int ret = sock->vrecv();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket recv");
    return ret;
  }

  if (ret > 0) {
    Event::ptr event = std::make_shared<Event>();
    event->_es = sock;
    event->_stype = _stype | EVENT_SUBTYPE_READ;
    
    ADD_EVENT(_r_event_pool_id, event);
  }

  return ret;
}

int LeftTcpEnd::l_recv(SOCKETID sid, std::shared_ptr<BufferItem> bi, void *opt = NULL) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  Socket::ptr sock = Socket::ptr();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  SocketContainer::iterator it = _sockets.find(sid);
  if ( it == _sockets.end() ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket not found", sid);
    return -1;
  }

  sock = it->second;
  if (!sock || sock->_socket_status == SOCKET_STATUS_CLOSE) {
    _sockets.erase(it);
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket closed", sid);
    return -1;
  }

  LOCK_GUARD_MUTEX_END

  int ret = sock->add_r_data(bi);

  Event::ptr event = std::make_shared<Event>();
  event->_es = sock;
  event->_stype = _stype | EVENT_SUBTYPE_READ;

  ADD_EVENT(_r_event_pool_id, event);

  return ret;
}

int LeftTcpEnd::l_write(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  Socket::ptr sock = Socket::ptr();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  SocketContainer::iterator it = _sockets.find(sid);
  if ( it == _sockets.end()) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket not found", sid);
    return -1;
  }
  sock = it->second;

  if (!sock || sock->_socket_status == SOCKET_STATUS_CLOSE) {
    _sockets.erase(it);
    return -1;
  }
  LOCK_GUARD_MUTEX_END

  int ret = sock->ssend();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket ssend");
  }

  return ret;
}

int LeftTcpEnd::stop() {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  _stop_flag = true;
  _reactor->_stop();
  _main_socket->vclose();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  _sockets.clear();
  LOCK_GUARD_MUTEX_END

  return 0;
}

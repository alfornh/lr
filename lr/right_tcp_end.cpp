#include "right_tcp_end.h"

#include <string>

#include "plt/net-inc.h"

#include "configure.h"
#include "event.h"
#include "event_pool.h"
#include "reactor.h"
#include "reactor_select.h"
#include "tcp_socket.h"
#include "udp_socket.h"
#include "utils.h"
#include "zlog.h"
#include "data_buffer.h"
#include "ipinfo.h"
#include "thread_manager.h"
#include "configure_item.h"
#include "timer.h"

int RightTcpEnd::init() {
  _stop_flag = false;

  switch (_ipi->_protocal) {
  case PROTOCAL_TCP:
    //_main_socket = MAKE_SHARED(TcpSocket, EVENT_TYPE_SOCKET_TCP);
    _stype = EVENT_TYPE_SOCKET_TCP;
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown source type ", _stype);
    return -1;
  }

  _r_event_pool_id = EventPool::reserve_event_queue();

  //_main_socket->_id = Socket::sign_socket_id();
  //_main_socket->_line = shared_from_this();

  if (PCONFIGURE->is_key_equal_value("right_reactor", "async")) {
    _reactor = MAKE_SHARED(AsyncIOMultiplex, Reactor::PROTOCOL_TCP);
  } else {
    _reactor = MAKE_SHARED(SelectReactor, Reactor::PROTOCOL_TCP);
  }

  _reactor->_line = shared_from_this();

  if (_reactor->_init(_ipi) < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "_reactor init");
    return -1;
  }

  return 0;
}

int RightTcpEnd::notify_w_event(SOCKETID id, int fd) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, id, fd);
  return _reactor->notify_w_event(id, fd);
}

int RightTcpEnd::listen() {
  return _reactor->_listen();
}

int RightTcpEnd::stop() {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  _stop_flag = false;
  _reactor->_stop();
  return 0;
}

int RightTcpEnd::l_recv(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  Socket::ptr socket = Socket::ptr();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  SocketContainer::iterator it = _sockets.find(sid);
  if ( it == _sockets.end() ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket not found", sid);
    return -1;
  }

  socket = it->second;
  if (!socket || socket->_socket_status == SOCKET_STATUS_CLOSE) {
    _sockets.erase(it);
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket closed", sid);
    return -1;
  }

  LOCK_GUARD_MUTEX_END

  int ret = socket->vrecv();
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket recv");
    return ret;
  }

  Event::ptr event = MAKE_SHARED(Event);
  event->_es = socket;
  event->_stype = _stype | EVENT_SUBTYPE_READ;

  ADD_EVENT(_r_event_pool_id, event);

  return ret;
}

int RightTcpEnd::l_recv(SOCKETID sid, std::shared_ptr<BufferItem> bi){
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  Socket::ptr socket = Socket::ptr();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  SocketContainer::iterator it = _sockets.find(sid);
  if ( it == _sockets.end() ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket not found", sid);
    return -1;
  }

  socket = it->second;
  if (!socket || socket->_socket_status == SOCKET_STATUS_CLOSE) {
    _sockets.erase(it);
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket closed", sid);
    return -1;
  }

  LOCK_GUARD_MUTEX_END

  int ret = socket->add_r_data(bi);

  Event::ptr event = MAKE_SHARED(Event);
  event->_es = socket;
  event->_stype = _stype | EVENT_SUBTYPE_READ;

  ADD_EVENT(_r_event_pool_id, event);

  return ret;
}

int RightTcpEnd::l_write(SOCKETID sid) {
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

int RightTcpEnd::l_close(SOCKETID sid) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  Socket::ptr socket;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)
  SocketContainer::iterator it = _sockets.find(sid);
  //if ( it !=  _sockets.end() && sid != _main_socket->_id ) {
  if ( it !=  _sockets.end() ) {
    socket = it->second;
    _sockets.erase(it);
  }
  LOCK_GUARD_MUTEX_END

  if ( !socket ) {
    return -1;
  }

  socket->vclose();
  _reactor->_del(sid, socket->_fd);

  Event::ptr event = MAKE_SHARED(Event);
  event->_es = socket;
  event->_stype = _stype | EVENT_SUBTYPE_CLOSE;

  ADD_EVENT(_r_event_pool_id, event);

  return 0;
}

std::shared_ptr<Socket> RightTcpEnd::make_right() {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  Socket::ptr c;
  int ret;

  c = MAKE_SHARED(TcpSocket, EVENT_TYPE_SOCKET_TCP);
  c->_id = Socket::sign_socket_id();
  if (c->_id < 1) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "no socket available");
    return Socket::ptr();
  }

  if (c->vinit(_ipi) < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket init");
    return Socket::ptr();
  }

  ret = c->vconnect();
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "connect");
    return Socket::ptr();
  }

  ret = c->nonblock(false);
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "nonblock");
    return Socket::ptr();
  }
 
  c->_line = shared_from_this();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_sockets)

  SocketContainer::iterator it = _sockets.find(c->_id);
  if (it != _sockets.end()) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "repeated socket id ", c->_id);
    return Socket::ptr();
  }
  _sockets[c->_id] = c;
  _reactor->_add(c->_id, c->_fd);
  LOCK_GUARD_MUTEX_END

  return c; 
}

//void RightTcpEnd::start_heart_beat() {
//  Event::ptr e = MAKE_SHARED(Event);
//  e->_es = _main_socket;
//  e->_stype = EVENT_TYPE_TIMER | EVENT_SUBTYPE_HEART_BEAT;
//  PTIMER->add(time(0) + HEART_BEAT_SPAN, e);
//}


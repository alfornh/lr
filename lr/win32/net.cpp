#include "plt/net-inc.h"

#include <stdlib.h>
#include <time.h>

#include "event_type.h"
#include "socket.h"
#include "zlog.h"
#include "thread_manager.h"
#include "utils.h"
#include "listener.h"

MULTIPLEX_CONTEXT::MULTIPLEX_CONTEXT(SOCKETID id, int fd) {
  _id = id;
  _fd = fd;
  _io_type = IO_TYPE_NULL;
  memzero(&_overlapped, sizeof(_overlapped));
  memzero(&_addrin, sizeof(_addrin));
  _bi = MAKE_SHARED(BufferItem);
}

AsyncIOMultiplex::~AsyncIOMultiplex() {
  //if (_handler > 0) {
  //  //::close(_handler);
  //}
  //_handler = NULL;
}

int AsyncIOMultiplex::_init(std::shared_ptr<IPInfo> ipi) {
  _ipi = ipi;
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);

  _handler = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
  if (_handler == NULL) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "CreateIoCompletionPort");
    return -1;
  }

  _listen_i_proc_thread_group_id = PTHREADMANAGER->reserve_thread_group();
  if (_listen_i_proc_thread_group_id < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
    return -1;
  }
  PTHREADMANAGER->start(_listen_i_proc_thread_group_id, _ipi->_reactor_thread_num);

  _listen_o_proc_thread_group_id = PTHREADMANAGER->reserve_thread_group();
  if (_listen_o_proc_thread_group_id < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
    return -1;
  }
  PTHREADMANAGER->start(_listen_o_proc_thread_group_id, _ipi->_io_thread_num);

  _stop_flag = false;

  return 0;
}

void AsyncIOMultiplex::_stop() {
  _stop_flag = true;
  PostQueuedCompletionStatus(_handler, 0, (DWORD) NULL, NULL);
}

int AsyncIOMultiplex::_listen() {
  for (int i = 0; i < _ipi->_reactor_thread_num; ++i) {
    RUN_TASK(_listen_i_proc_thread_group_id,
      MAKE_SHARED(Task, std::bind(&AsyncIOMultiplex::listen_i_proc, this))
    );
  }
  for (int i = 0; i < _ipi->_io_thread_num; ++i) {
    RUN_TASK(_listen_o_proc_thread_group_id,
      MAKE_SHARED(Task, std::bind(&AsyncIOMultiplex::listen_o_proc, this))
    );
  }

  return 0;
}

void AsyncIOMultiplex::listen_i_proc() {
  bool bret;
  int i;

  if (_protocol != Reactor::PROTOCOL_TCP) {
    return ;
  }

  while (!_stop_flag ) {
    ZLOG_DEBUG(__FILE__, __LINE__, __func__);

    LOCK_GUARD_MUTEX_BEGIN(_mutex_handler)
    if (_stop_flag) {
      return ;
    }
    if (_line->_main_socket) {
      _line->l_accept(_line->_main_socket->_id);
    } else {
      //right end has no main_socket, so it does not need accept procedure.
      return ;
    }

    LOCK_GUARD_MUTEX_END
  }
}

void AsyncIOMultiplex::listen_o_proc() {
  DWORD   socketid;
  DWORD      numberOfBytesTransferred;
  LPWSAOVERLAPPED lpoverlapped = NULL;
  bool iostatus;

  while (!_stop_flag) {
    iostatus = GetQueuedCompletionStatus(_handler,  &numberOfBytesTransferred, (PULONG_PTR)&socketid, &lpoverlapped, INFINITE);
    if (!iostatus || !socketid ) {
      ZLOG_WARN(__FILE__, __LINE__, __func__, "GetQueuedCompletionStatus");
      continue;
    }
    if ( _stop_flag ) {
      ZLOG_WARN(__FILE__, __LINE__, __func__, "_stop_flag", _stop_flag);
      return ;
    }

    if (numberOfBytesTransferred == 0 ) {
      _line->l_close(socketid);
      LOCK_GUARD_MUTEX_BEGIN(_mutex_contexts)
        _contexts.erase(socketid);
      LOCK_GUARD_MUTEX_END
      continue;
    }

    MULTIPLEX_CONTEXT::ptr ctx;

    LOCK_GUARD_MUTEX_BEGIN(_mutex_contexts)
      MULTIPLEX_CONTEXT_CONTAINER::iterator it = _contexts.find(socketid);
      if ( it == _contexts.end() ) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "can not find context for socket id", socketid);
        continue;
      }
      ctx = it->second;
    LOCK_GUARD_MUTEX_END

    ctx->_bi->_len = numberOfBytesTransferred;

    _line->l_recv(socketid, ctx->_bi, &(ctx->_addrin));

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, ctx->_bi->_buffer);

    _mod(ctx);
  }
}

int AsyncIOMultiplex::_add(SOCKETID sid, int fd) {
  MULTIPLEX_CONTEXT::ptr ctx = MAKE_SHARED(MULTIPLEX_CONTEXT, sid, fd);
  HANDLE h = CreateIoCompletionPort((HANDLE)fd, _handler, (DWORD_PTR)(ctx->_id), 0);
  if ( h == NULL ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, WSAGetLastError());
    return -1;
  }

  LOCK_GUARD_MUTEX_BEGIN(_mutex_contexts)
  _contexts[sid] = ctx;
  LOCK_GUARD_MUTEX_END

  _mod(ctx);

  return 0;
}

int AsyncIOMultiplex::_del(SOCKETID sid, int fd) {
  return 0;
}

int AsyncIOMultiplex::_mod(SOCKETID sid, int fd) {
  return 0;
}

int AsyncIOMultiplex::_mod(MULTIPLEX_CONTEXT::ptr mc) {
  DWORD flags = 0;
  DWORD recv = 0;
  WSABUF wsabuf;
  int ret;
  int addrlen;

  memzero(&wsabuf, sizeof(wsabuf));
  wsabuf.buf = mc->_bi->_buffer;
  wsabuf.len = BufferItem::buffer_item_capacity;
  memzero(wsabuf.buf, wsabuf.len);

  addrlen = sizeof(mc->_addrin);

  if (_protocol == Reactor::PROTOCOL_TCP) {
    ret = WSARecv(mc->_fd, &wsabuf, 1, &recv, &flags, &mc->_overlapped, NULL);
  } else {
    ret = ::WSARecvFrom(mc->_fd, &wsabuf, 1, &recv, &flags, (struct sockaddr*)&mc->_addrin, &addrlen, &(mc->_overlapped), NULL);
  }

  if (SOCKET_ERROR == ret) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "WSARecv", GetLastError());
    return -1;
  }
  return 0;
}

int AsyncIOMultiplex::notify_w_event(SOCKETID sid, int fd) {
  return _line->l_write(sid);
}

extern "C" {

static bool init_network_flag = false;
void init_network() {
  WSADATA wsd;
  if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "WSAStartup error");
  }
}

int socket_create(void *param) {
  if (!init_network_flag) {
    init_network();
    init_network_flag = true;
  }
  SOCKET sock;
  struct socket_create_param *scp = (struct socket_create_param *)param;
  if ( !scp ) {
    return -1;
  }

  switch ( scp->protocol ) {
  case Reactor::PROTOCOL_TCP:
    if (scp->overlapped) {
      sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    } else {
      sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
  break;

  case Reactor::PROTOCOL_UDP:
    if (scp->overlapped) {
      sock = WSASocket(AF_INET, SOCK_DGRAM,  IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
    } else {
      sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
  break;

  default:
    return -1;
  }
  return sock;
}

int socket_bind(int socket, void *param) {
  struct socket_bind_param *sbp = (struct socket_bind_param *)param;
  if ( !sbp ) {
    return -1;
  }

  sockaddr_in addrin;
  addrin.sin_family = AF_INET;
  addrin.sin_addr.s_addr = inet_addr(sbp->ip);
  addrin.sin_port = htons(sbp->port);
  if ( 0 != ::bind(socket, (struct sockaddr *)&addrin, sizeof(addrin)) ) {
    return -1;
  }

  return 0;
}

int socket_connect(int socket, void *param) {
  struct socket_connect_param *scp = (struct socket_connect_param *)param;
  if ( !scp || scp->protocol != Reactor::PROTOCOL_TCP) {
    return -1;
  }
  sockaddr_in addrin;
  addrin.sin_family = AF_INET;
  addrin.sin_addr.s_addr = inet_addr(scp->ip);
  addrin.sin_port = htons(scp->port);
  if ( 0 != ::connect(socket, (struct sockaddr *)&addrin, sizeof(addrin)) ) {
    return -1;
  }

  return 0;
}

int socket_accept(int socket, void *param) {
  struct sockaddr_in addrin;
  int addrlen = sizeof(addrin);
  SOCKET s;
  struct socket_accept_param *sap = (struct socket_accept_param *)param;
  while ( true ) {
    s = ::WSAAccept(socket, (struct sockaddr *)&addrin, &addrlen, NULL, NULL);
    if ( param ) {
      memzero(sap->ip, sizeof(sap->ip));
      strcpy(sap->ip, inet_ntoa(addrin.sin_addr));
      sap->port = ntohs(addrin.sin_port);
      sap->socket = s;
    }

    if ( s == INVALID_SOCKET ) {
      if (WSA_IO_PENDING == WSAGetLastError()) {
        continue;
      }
      return -1;
    }
    return s;
  }
}

int socket_listen(int socket, void *param) {
  if ( !param ) {
    return -1;
  }
  if ( 0 != ::listen(socket, ((struct socket_listen_param *)param)->number)) {
    return -1;
  }

  return 0;
}

int _tcp_socket_recv(int socket, struct socket_recv_param *srp) {
  DWORD recvlen = 0;
  DWORD flags = 0;
  WSABUF wsabuf;
  wsabuf.buf = srp->buf;
  wsabuf.len = srp->blen;
  int ret;
  while ( true ) {
    ret = ::WSARecv(socket, &wsabuf, 1, &recvlen, &flags, srp->poverlapped, NULL);
    if ( ret == SOCKET_ERROR) {
      if ( WSA_IO_PENDING == WSAGetLastError()) {
        continue;
      }
      return -1;
    }
    break;
  }
  return recvlen;
}

int _udp_socket_recv(int socket, struct socket_recv_param *srp) {
  DWORD flags = 0;
  DWORD recvlen = 0;
  WSABUF wsabuf;

  struct sockaddr_in addrin;
  int addrlen = sizeof(addrin);
  memzero(&addrin, sizeof(addrin));
  int ret;
  while ( true ) {
    memzero(&wsabuf, sizeof(wsabuf));
    wsabuf.buf = srp->buf;
    wsabuf.len = srp->blen;
    ret = ::WSARecvFrom(socket, &wsabuf, 1, &recvlen, &flags, (struct sockaddr *)&addrin, &addrlen, srp->poverlapped, NULL);
    if ( ret == SOCKET_ERROR) {
      if ( WSA_IO_PENDING == WSAGetLastError() ) {
        continue;
      }
      return -1;
    }
    break;
  }

  memzero(srp->ip, sizeof(srp->ip));
  strcpy(srp->ip, inet_ntoa(addrin.sin_addr));
  srp->port = ntohs(addrin.sin_port);
  srp->blen = recvlen;
  return recvlen;
}

int socket_recv(int socket, void *param) {
  struct socket_recv_param *srp = (struct socket_recv_param *)param;
  if ( !srp ) {
    return 0;
  }
  switch (srp->protocol) {
  case Reactor::PROTOCOL_TCP:
    return _tcp_socket_recv(socket, srp);
  break;

  case Reactor::PROTOCOL_UDP:
    return _udp_socket_recv(socket, srp);
  break;
  }

  return -1;
}

int _tcp_socket_send(int socket, struct socket_send_param *ssp) {
  DWORD sent = 0;
  WSABUF wsabuf;
  int ret;

  memzero(&wsabuf, sizeof(wsabuf));
  wsabuf.buf = ssp->buf;
  wsabuf.len = ssp->blen;

  while ( true ) {
    ret = ::WSASend(socket, &wsabuf, 1, &sent, 0, ssp->poverlapped, NULL);
    if ( ret == SOCKET_ERROR ) {
      if (WSA_IO_PENDING == WSAGetLastError()) {
        continue;
      }
      return -1;
    }

    break;
  }
  return sent;
}

int _udp_socket_send(int socket, struct socket_send_param *ssp) {
  DWORD sent;
  struct sockaddr_in addrin;
  WSABUF wsabuf;
  int ret;

  memzero(&addrin, sizeof(addrin));
  addrin.sin_family = AF_INET;
  addrin.sin_addr.s_addr = inet_addr(ssp->ip);
  addrin.sin_port = htons(ssp->port);

  wsabuf.buf = ssp->buf;
  wsabuf.len = ssp->blen;

  while ( true ) {
    ret = ::WSASendTo(socket, &wsabuf, 1, &sent, 0, (struct sockaddr *)&addrin, sizeof(addrin), ssp->poverlapped, NULL);
    if ( ret == SOCKET_ERROR) {
      if (WSA_IO_PENDING == WSAGetLastError()) {
        continue;
      }
      return -1;
    }
    break;
  }
  return sent;
}

int socket_send(int socket, void *param) {
  struct socket_send_param *ssp = (struct socket_send_param *)param;
  if ( !ssp || (ssp->buf == NULL) || (ssp->blen < 1)) {
    return 0;
  }
  switch (ssp->protocol) {
  case Reactor::PROTOCOL_TCP:
    return _tcp_socket_send(socket, ssp);
  break;

  case Reactor::PROTOCOL_UDP:
    return _udp_socket_send(socket, ssp);
  break;
  }

  return -1;
}

int socket_nonblock(int socket, bool nb) {
  unsigned long nonblock = nb ? 1: 0;
  return ::ioctlsocket(socket, FIONBIO, &nonblock);
}

int socket_reuse(int socket, bool ru) {
  int val = ru ? 1 : 0;
  return ::setsockopt(socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&val, sizeof(val));
}

int socket_close(int socket) {
  int ret = ::closesocket(socket);
  if ( ret == SOCKET_ERROR) {
    return WSAGetLastError();
  }
  return 0;
}

}

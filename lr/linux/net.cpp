#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "plt/net-inc.h"

#include "ipinfo.h"
#include "listener.h"
#include "socket.h"
#include "thread_manager.h"
#include "utils.h"
#include "zlog.h"


AsyncIOMultiplex::~AsyncIOMultiplex() {
  if (_epollfd > 0) {
    ::close(_epollfd);
  }
  _epollfd = -1;
}

int AsyncIOMultiplex::_init(std::shared_ptr<IPInfo> ipi) {
  _ipi = ipi;
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);

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
  _epollfd = epoll_create1(EPOLL_CLOEXEC);
  if (_epollfd < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "epoll_create1", errno);
    return -1;
  }

  return 0;
}

void AsyncIOMultiplex::_stop() {
  _stop_flag = true;

  _cv_o_sockets.notify_all();
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
  BufferItem::ptr bi = MAKE_SHARED(BufferItem);
  int i, what;
  SOCKETID id;
  struct epoll_event *ee;

  while (!_stop_flag) {
    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "epoll wait");

    LOCK_GUARD_MUTEX_BEGIN(_mutex_epollfd)
    if (_stop_flag) {
      return ;
    }
    bi->_len = epoll_wait(
      _epollfd,
      (struct epoll_event *)(bi->_buffer),
      EPOLL_WAIT_EVENTS, //BufferItem::buffer_item_capacity/sizeof(struct epoll_event),
      30 * 1000
    );
    LOCK_GUARD_MUTEX_END
    if (bi->_len == 0) {
      ZLOG_DEBUG(__FILE__, __LINE__, __func__, "epoll wait timeout", errno);
      continue;
    }

    if (_stop_flag) {
      return ;
    }
    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "aft epoll wait", bi->_len, errno);

    if (bi->_len < 0) {
      if (errno != EINTR) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "epoll_wait errno", errno);
        return ;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, "epoll_wait EINTR");
      continue;
    }

    ee = (struct epoll_event *)(bi->_buffer);
    for (i = 0; i < bi->_len; ++i) {
      what = ee[i].events;
      id = ee[i].data.u64;

      if (_line->_main_socket) {//right end has no main_socket
        if (_line->_main_socket->_stype != EVENT_TYPE_SOCKET_UDP && id == _line->_main_socket->_id) {
          _line->l_accept(id);
          continue;
        }
      }

      if (what & (EPOLLIN | EPOLLPRI)) {

        _line->l_recv(id);
      }

      if (what & (EPOLLRDHUP | EPOLLERR)) {

        _line->l_close(id);
      }
    }
  }
}

void AsyncIOMultiplex::listen_o_proc() {
  SOCKETID id;
  while (!_stop_flag) {
    {
      std::unique_lock<std::mutex> l(_mutex_o_sockets);
      _cv_o_sockets.wait(l, [this]{
        return (_stop_flag || _o_sockets.size() > 0);
      });

      if (_stop_flag) {
        return ;
      }

      if (_o_sockets.empty()) {
        continue;
      }

      id = _o_sockets.front();
      _o_sockets.pop_front();
    }

    _line->l_write(id);
  }
}

int AsyncIOMultiplex::_add(SOCKETID sid, int fd) {
  struct epoll_event ee;
  memset(&ee, 0x0, sizeof(ee));
  ee.data.u64 = sid;
  ee.events = EPOLLPRI | EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET;

  int ret = epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ee);
  if (EEXIST == ret) {
    return epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ee);
  }

  return ret;
}

int AsyncIOMultiplex::_del(SOCKETID sid, int fd) {
  struct epoll_event ee;
  memset(&ee, 0x0, sizeof(ee));
  ee.data.u64 = sid;
  return epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, &ee);
}

int AsyncIOMultiplex::_mod(SOCKETID sid, int fd) {
  struct epoll_event ee;
  memset(&ee, 0x0, sizeof(ee));
  ee.data.u64 = sid;
  ee.events = EPOLLPRI | EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET ;
  
  int ret = epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ee);
  if (ENOENT == ret) {
    return epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ee);
  }

  return ret;
}

int AsyncIOMultiplex::notify_w_event(SOCKETID sid, int fd) {
  if (_stop_flag) {
    return 0;
  }

  {
    std::unique_lock<std::mutex> l(_mutex_o_sockets);
    _o_sockets.emplace_back(sid);
  }

  _cv_o_sockets.notify_one();

  return 1;
}


extern "C" {

int socket_create(void *param) {
  int socket;
  struct socket_create_param *scp = (struct socket_create_param *)param;
  if ( !scp ) {
    return -1;
  }

  switch ( scp->protocol ) {
  case Reactor::PROTOCOL_TCP:
    socket = ::socket(AF_INET, SOCK_STREAM, 0);
  break;
  
  case Reactor::PROTOCOL_UDP:
    socket = ::socket(AF_INET, SOCK_DGRAM, 0);
  break;

  default:
    return -1;
  }
  return socket;
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
  return ::bind(socket, (struct sockaddr *)&addrin, sizeof(addrin));
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
  return ::connect(socket, (struct sockaddr *)&addrin, sizeof(addrin));
}

int socket_accept(int socket, void *param) {
  struct sockaddr_in addrin;
  socklen_t addrlen = sizeof(addrin);
  int s;
  struct socket_accept_param *sap = (struct socket_accept_param *)param;
  while ( true ) {
    s = ::accept(socket, (struct sockaddr*)&addrin, &addrlen);
    if ( param ) {
      memzero(sap->ip, sizeof(sap->ip));
      strcpy(sap->ip, inet_ntoa(addrin.sin_addr));
      sap->port = ntohs(addrin.sin_port);
      sap->socket = s;
    }

    if ( s < 0 ) {
      if (errno == EINTR) {
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
  return ::listen(socket, ((struct socket_listen_param *)param)->number);
}

int _tcp_socket_recv(int socket, struct socket_recv_param *srp) {
  return ::recv(socket, srp->buf, srp->blen, 0);
}

int _udp_socket_recv(int socket, struct socket_recv_param *srp) {
  struct sockaddr_in addrin;
  socklen_t addrlen = sizeof(addrin);
  addrin.sin_family = AF_INET;
  addrin.sin_addr.s_addr = inet_addr(srp->ip);
  addrin.sin_port = htons(srp->port);
  return ::recvfrom(socket, srp->buf, srp->blen, 0, (struct sockaddr*)&addrin, &addrlen);
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
  return ::send(socket, ssp->buf, ssp->blen, MSG_NOSIGNAL);
}

int _udp_socket_send(int socket, struct socket_send_param *ssp) {
  struct sockaddr_in addrin;
  addrin.sin_family = AF_INET;
  addrin.sin_addr.s_addr = inet_addr(ssp->ip);
  addrin.sin_port = htons(ssp->port);
  return ::sendto(socket, ssp->buf, ssp->blen, MSG_NOSIGNAL, (struct sockaddr*)&addrin, sizeof(addrin));
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
  int flags = ::fcntl(socket, F_GETFL, 0);
  if (flags < 0) {
    return -1;
  }

  if (nb) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }

  return ::fcntl(socket, F_SETFL, flags);
}

int socket_reuse(int socket, bool ru) {
  int val = 0;
  if (ru) {
    val = 1;
  }
  return ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
}

int socket_close(int socket) {
  return ::close(socket);
}

}

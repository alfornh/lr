#include "udp_socket.h"

#include <errno.h>
#include <sys/types.h>

#include "plt/net-inc.h"

#include "configure.h"
#include "data_buffer.h"
#include "event.h"
#include "event_pool.h"
#include "reactor.h"
#include "listener.h"
#include "ipinfo.h"
#include "listener.h"
#include "utils.h"
#include "zlog.h"

UdpSocket::UdpSocket(EVENTID mtype) {
  _stype  = mtype;
}

int UdpSocket::vinit(std::shared_ptr<IPInfo> ipi) {
  struct socket_create_param scp;
  _ipi = ipi;
  memzero(&scp, sizeof(scp));
  scp.protocol = Reactor::PROTOCOL_UDP;
  scp.overlapped = (_line->_reactor->_name == Reactor::REACTOR_ASYNC);

  _fd = socket_create(&scp);
  if (_fd < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket");
    return -1;
  }

  _socket_status = SOCKET_STATUS_INIT;
  return _fd;
}

int UdpSocket::vbind() {
  struct socket_bind_param sbp;
  memzero(&sbp, sizeof(sbp));
  sbp.protocol = Reactor::PROTOCOL_UDP;
  sbp.port = _ipi->_port;
  strncpy(sbp.ip, _ipi->_ip.c_str(), _ipi->_ip.size());

  int ret = socket_bind(_fd, &sbp);
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "bind with", _fd, _ipi->_ip, _ipi->_port);
    return -1;
  }

  _socket_status = SOCKET_STATUS_ACTIVE;

  return 0;
}

int UdpSocket::vsend(const char *buf, const int blen) {
  if ( blen < 1 || !buf ) {
    return 0;
  }

  struct socket_send_param ssp;
  memzero(&ssp, sizeof(ssp));
  ssp.protocol = Reactor::PROTOCOL_UDP;
  ssp.socket = _fd;
  ssp.port = _ipi->_port;
  strncpy(ssp.ip, _ipi->_ip.c_str(), _ipi->_ip.size());

  int sent = 0;
  int ret;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_fd)

  do {
    ssp.buf = (char *)buf + sent;
    ssp.blen = blen - sent;
    ret = socket_send(_fd, &ssp);
    if (ret < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket_send");
      break;
    }

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "sendto", _ipi->_ip, _ipi->_port, buf);

    sent += ret;

    if (sent < blen) {
      continue;
    }

    break;
  } while ( true );

  LOCK_GUARD_MUTEX_END

  return sent;
}

int UdpSocket::vrecv() {
  BufferItem::ptr bi = MAKE_SHARED(BufferItem);
  struct socket_recv_param srp;
  memzero(&srp, sizeof(srp));
  srp.protocol = Reactor::PROTOCOL_UDP;
  srp.buf = bi->_buffer;
  srp.blen = BufferItem::buffer_item_capacity;

  int ret;
  int recv = 0;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_fd)
  do {
    ret = socket_recv(_fd, &srp);
    if (ret < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket_recv");
      break;
    }

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, bi->_len, bi->_buffer);

    bi->_len = ret;
    recv += ret;

    add_r_data(bi);

    bi->clear();

    if (ret >= BufferItem::buffer_item_capacity) {
      continue;
    }

    break;
  } while ( true );
  LOCK_GUARD_MUTEX_END

  return recv;
}

std::shared_ptr<UdpSocket> UdpSocket::lrecv() {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  struct socket_recv_param srp;
  memzero(&srp, sizeof(srp));

  UdpSocket::ptr socket = MAKE_SHARED(UdpSocket, _stype);
  BufferItem::ptr bi = MAKE_SHARED(BufferItem);

  srp.protocol = Reactor::PROTOCOL_UDP;
  srp.buf = bi->_buffer;
  srp.blen = BufferItem::buffer_item_capacity;
  int ret;
  int recv = 0;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_fd)
  do {
    ret = socket_recv(_fd, &srp);
    if (ret < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket_recv");
      break;
    }

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, ret, bi->_buffer);

    bi->_len = ret;
    socket->add_r_data(bi);
    bi->clear();

    recv += ret;
    if (ret >= BufferItem::buffer_item_capacity) {
      continue;
    }

    break;
  } while ( true );
  LOCK_GUARD_MUTEX_END

  if (socket->_r_db->_len < 1) {
    return NULL;
  }

  socket->_id = Socket::sign_socket_id();
  socket->_fd = 0;//dup(_fd);
  socket->_line = _line;
  socket->_ipi->_ip = std::string(srp.ip);//inet_ntoa(addr.sin_addr);
  socket->_ipi->_port = srp.port;//ntoh16(addr.sin_port);
  socket->_socket_status = SOCKET_STATUS_ACTIVE;

  return socket;
}

int UdpSocket::lsend(const std::shared_ptr<Socket> socket) {
  struct socket_send_param ssp;
  memzero(&ssp, sizeof(ssp));
  ssp.protocol = Reactor::PROTOCOL_UDP;
  ssp.port = socket->_ipi->_port;
  strncpy(ssp.ip, socket->_ipi->_ip.c_str(), socket->_ipi->_ip.size());

  int sent = 0;
  int ret;
  int blen = socket->_w_db->_len;
  char *buf;
  std::vector<char> d;

  if (blen < 1) {
    return 0;
  }

  if (socket->_w_db->_data.size() == 1) {
    BufferItem::ptr bi = socket->_w_db->_data.front();
    buf = bi->_buffer;
  } else {
    socket->move_w_data(d);
    buf = &(*d.begin());
  }

  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_fd)
  do {
    ssp.buf = buf + sent;
    ssp.blen = blen - sent;
    ret = socket_send(_line->_main_socket->_fd, &ssp);
    if (ret < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket_send");
      break;
    }

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "sendto", socket->_ipi->_ip, socket->_ipi->_port, buf);

    sent += ret;

    if (sent < blen) {
      continue;
    }

    break;

  } while ( true );
  LOCK_GUARD_MUTEX_END

  return 0;
  //return blen - sent;
}

int UdpSocket::vclose() {
  _socket_status = SOCKET_STATUS_CLOSE;
  return 0;
}

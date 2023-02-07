#include "tcp_socket.h"

#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>

#include "plt/net-inc.h"

#include "socket.h"
#include "event.h"
#include "event_type.h"
#include "reactor.h"
#include "listener.h"
#include "zlog.h"
#include "utils.h"
#include "data_buffer.h"
#include "event_pool.h"
#include "configure.h"

TcpSocket::TcpSocket(EVENTID stype) {
  _stype  = stype;
}

int TcpSocket::vinit(std::shared_ptr<IPInfo> ipi) {
  _ipi = ipi;
  struct socket_create_param scp;
  scp.protocol = Reactor::PROTOCOL_TCP;
  _fd = socket_create(&scp);
  if (_fd < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket error", _fd);
    return -1;
  }
  
  _socket_status = SOCKET_STATUS_INIT;
  return 0;
}

std::shared_ptr<Socket> TcpSocket::vaccept() {
  //struct sockaddr_in addr;
  struct socket_accept_param sap;
  memzero(&sap, sizeof(sap));
  //memset(&addr, 0x0, sizeof(addr));
  //socklen_t socklen = sizeof(addr);
  TcpSocket::ptr sock = MAKE_SHARED(TcpSocket, _stype);
  sock->_id = Socket::sign_socket_id();
  sock->_line = _line;//DYNAMIC_CAST(EventSource, sock);
  //sock->_r_event_pool_id = _r_event_pool_id;
  //sock->_w_event_pool_id = _w_event_pool_id;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_a_fd)
  //sock->_fd = ::accept(_fd, (struct sockaddr *)&addr, &socklen);
  sock->_fd = socket_accept(_fd, &sap);
  if (sock->_fd < 0) {
    ZLOG_WARN(__FILE__, __LINE__, __func__, "accept errno", errno);
    return Socket::ptr();
  }
  LOCK_GUARD_MUTEX_END

  sock->nonblock(true);
  sock->_time_stamp = time(0);
  sock->_ipi->_ip = std::string(sap.ip);//std::string(inet_ntoa(addr.sin_addr));
  sock->_ipi->_port = sap.port;//ntoh16(addr.sin_port);
  sock->_socket_status = SOCKET_STATUS_ACTIVE;

  ZLOG_INFO(__FILE__, __LINE__, __func__, sock->_ipi->_ip, sock->_ipi->_port);

  return DYNAMIC_CAST(Socket, sock);
}

int TcpSocket::vbind() {
  struct socket_bind_param sbp;
  memzero(&sbp, sizeof(sbp));
  sbp.protocol = Reactor::PROTOCOL_TCP;
  strcpy(sbp.ip, _ipi->_ip.c_str());
  sbp.port = _ipi->_port;

  //struct sockaddr_in addr;
  //memset(&addr, 0x0, sizeof(addr));
  //addr.sin_family = AF_INET;
  //addr.sin_port  = hton16(_ipi._port);
  //addr.sin_addr.s_addr = htonl(INADDR_ANY);
  //int ret = ::bind(_fd, (struct sockaddr *)&addr, sizeof(addr));
  int ret = socket_bind(_fd, &sbp);
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "bind with", _fd, _ipi->_ip, _ipi->_port);
    return -1;
  }

  return 0;
}

int TcpSocket::vconnect() {
  struct socket_connect_param scp;
  memzero(&scp, sizeof(scp));
  scp.protocol = Reactor::PROTOCOL_TCP;
  strncpy(scp.ip, _ipi->_ip.c_str(), _ipi->_ip.size());
  scp.port = _ipi->_port;

  //struct sockaddr_in server;
  //
  //memset(&server, 0x0, sizeof(server));
  //
  //server.sin_family = AF_INET;
  //server.sin_port   = hton16(_ipi._port);
  //server.sin_addr.s_addr = inet_addr(_ipi._ip.c_str());
  //
  //int ret = ::connect(_fd, (struct sockaddr *) &server, sizeof(server));
  int ret = socket_connect(_fd, &scp);
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, _fd, _ipi->_ip, _ipi->_port, errno);
    return -1;
  }

  ZLOG_DEBUG(__FILE__, __LINE__, __func__, _fd, _ipi->_ip, _ipi->_port);

  _socket_status = SOCKET_STATUS_ACTIVE;

  return 0;
}

int TcpSocket::vlisten() {
  struct socket_listen_param slp;
  slp.number = 10;
  //int ret = ::listen(_fd, 65535);
  int ret = socket_listen(_fd, &slp);
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "listen with fd", _fd);
    return ret;
  }

  return 0;
}

int TcpSocket::vrecv() {
  struct socket_recv_param srp;
  memzero(&srp, sizeof(srp));
  srp.protocol = Reactor::PROTOCOL_TCP;

  int ret;
  int lrecv = 0;
  BufferItem::ptr bi = std::make_shared<BufferItem>();

  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_fd)
  while ( _socket_status == SOCKET_STATUS_ACTIVE || _socket_status == SOCKET_STATUS_WEBSOCKET_ACTIVE) {
    srp.buf = bi->_buffer + bi->_len;
    srp.blen = bi->available();
    ret = socket_recv(_fd, &srp);
    if ( ret < 0 ) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket_recv errno", errno);
      break;
    }

    if (ret == 0) {
      //ZLOG_WARN(__FILE__, __LINE__, __func__, "peer closed");
      return -1;
    }

    bi->_len += ret;
    lrecv += ret;

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, bi->_len, bi->_buffer);

    add_r_data(bi);

    if (bi->available() < 1) {
      bi->clear();
      continue;
    }

    break;
  }
  LOCK_GUARD_MUTEX_END

  return lrecv;
}

int TcpSocket::vsend(const char *buf, const int blen) {
  int sent = 0;
  int ret;
  struct socket_send_param ssp;
  memzero(&ssp, sizeof(ssp));
  ssp.protocol = Reactor::PROTOCOL_TCP;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_fd)
  while (sent < blen && (_socket_status == SOCKET_STATUS_ACTIVE || _socket_status == SOCKET_STATUS_WEBSOCKET_ACTIVE)) {
    ssp.buf = (char *)buf + sent;
    ssp.blen = blen - sent;
    //ret = ::send(_fd, buf + sent, blen - sent, MSG_NOSIGNAL);
    ret = socket_send(_fd, &ssp);
    if (ret < 0) {
      //if (errno == EINTR) {
      //  ZLOG_WARN(__FILE__, __LINE__, __func__, "errno", errno);
      //  continue;
      //}

      ZLOG_ERROR(__FILE__, __LINE__, __func__, _ipi->_ip, _ipi->_port, errno, buf);
      break;
    }

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, buf);
    sent += ret;
  }
  LOCK_GUARD_MUTEX_END
  return sent;
}

int TcpSocket::vclose() {
  _socket_status = SOCKET_STATUS_CLOSE;

  return 0;
}

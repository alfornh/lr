#include "udp_socket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "data_buffer.h"
#include "event.h"
#include "event_pool.h"
#include "ipinfo.h"
#include "listener.h"
#include "utils.h"
#include "zlog.h"

UdpSocket::UdpSocket(EVENTID mtype) {
  _stype  = mtype;
}

int UdpSocket::vinit(IPInfo &ipi) {
  _fd = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (_fd < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "socket");
    return -1;
  }

  _ipi = ipi;

  _socket_status = SOCKET_STATUS_INIT;
  return _fd;
}

int UdpSocket::vbind() {
  struct sockaddr_in addr;
  memset(&addr, 0x0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port  = htons(_ipi._port);
  if (_stype == EVENT_TYPE_CLIENT_SOCKET_UDP) {
    addr.sin_addr.s_addr = inet_addr(_ipi._ip.c_str());
  } else {
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
  }

  int ret = ::bind(this->_fd, (struct sockaddr *)&addr, sizeof(addr));
  if ( ret < 0 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "bind with", _fd, _ipi._ip, _ipi._port);
    return -1;
  }

  _socket_status = SOCKET_STATUS_ACTIVE;

  return 0;
}

int UdpSocket::vsend(const char *buf, const int blen) {
  int sent = 0;
  int ret;
  struct sockaddr_in addr;
  socklen_t socklen = sizeof(addr);

  if (blen < 1) {
    return 0;
  }

  memset(&addr, 0x0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port  = htons(_ipi._port);
  addr.sin_addr.s_addr = inet_addr(_ipi._ip.c_str());

  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_fd)

  do {
    ret = ::sendto(_fd, buf + sent, blen - sent, MSG_NOSIGNAL, (struct sockaddr *)&addr, socklen);
    if (ret < 0) {
      if (errno == EINTR) {
        ZLOG_WARN(__FILE__, __LINE__, __func__, "EINTR");
        continue;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, "sendto errno", errno);
      break;
    }

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "sendto", _ipi._ip, _ipi._port, buf);

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
  int ret;
  int recv = 0;
  struct sockaddr_in addr;
  memset(&addr, 0x0, sizeof(addr));
  socklen_t socklen = sizeof(addr);

  BufferItem::ptr bi = MAKE_SHARED(BufferItem);
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_fd)
  do {
    ret = ::recvfrom(_fd, bi->_buffer + bi->_len, bi->available(), 0, (struct sockaddr *)&addr, &socklen);
    if (ret < 0) {
      if (errno == EINTR) {
        ZLOG_WARN(__FILE__, __LINE__, __func__, "recvfrom EINTR");
        continue;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, "recvfrom error", errno);
      return -1;
    }

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, bi->_len, bi->_buffer);

    bi->_len += ret;
    recv += ret;

    add_r_data(bi);

    if (bi->available() < 1) {
      bi->clear();
      continue;
    }

    break;
  } while ( true );
  LOCK_GUARD_MUTEX_END

  return recv;
}

std::shared_ptr<UdpSocket> UdpSocket::lrecv() {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  int ret;
  int recv = 0;
  struct sockaddr_in addr;
  memset(&addr, 0x0, sizeof(addr));

  UdpSocket::ptr socket = MAKE_SHARED(UdpSocket, _stype);

  socklen_t socklen = sizeof(addr);

  BufferItem::ptr bi = MAKE_SHARED(BufferItem);
  LOCK_GUARD_MUTEX_BEGIN(_mutex_r_fd)
  do {
    ret = ::recvfrom(_fd, bi->_buffer + bi->_len, bi->available(), 0, (struct sockaddr *)&addr, &socklen);
    if (ret < 0) {
      if (errno == EINTR) {
        ZLOG_WARN(__FILE__, __LINE__, __func__, "recvfrom EINTR");
        continue;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, "recvfrom error", errno);
      return NULL;
    }

    if (ret == 0) {
      break;
    }
    ZLOG_DEBUG(__FILE__, __LINE__, __func__, ret, bi->_buffer);

    bi->_len += ret;
    recv += ret;

    socket->add_r_data(bi);

    if (bi->available() < 1) {
      bi->clear();
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
  socket->_ipi._ip = inet_ntoa(addr.sin_addr);
  socket->_ipi._port = ntohs(addr.sin_port);
  socket->_socket_status = SOCKET_STATUS_ACTIVE;

  return socket;
}

int UdpSocket::lsend(const std::shared_ptr<Socket> socket) {
  int sent = 0;
  int ret;
  struct sockaddr_in addr;
  int blen = socket->_w_db->_len;
  char *buf;
  std::vector<char> d;
  socklen_t socklen = sizeof(addr);

  if (blen < 1) {
    return 0;
  }

  memset(&addr, 0x0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port  = htons(socket->_ipi._port);
  addr.sin_addr.s_addr = inet_addr(socket->_ipi._ip.c_str());

  if (socket->_w_db->_data.size() == 1) {
    BufferItem::ptr bi = socket->_w_db->_data.front();
    buf = bi->_buffer;
  } else {
    socket->move_w_data(d);
    buf = &(*d.begin());
  }

  LOCK_GUARD_MUTEX_BEGIN(_mutex_w_fd)
  do {
    ret = ::sendto(_line->_main_socket->_fd, buf + sent, blen - sent, MSG_NOSIGNAL, (struct sockaddr *)&addr, socklen);
    if (ret < 0) {
      if (errno == EINTR) {
        ZLOG_WARN(__FILE__, __LINE__, __func__, "EINTR");
        continue;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, "sendto errno", errno);
      break;
    }

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "sendto", socket->_ipi._ip, socket->_ipi._port, buf);

    sent += ret;

    if (sent < blen) {
      continue;
    }

    break;

  } while ( true );
  LOCK_GUARD_MUTEX_END

  return sent;
}

int UdpSocket::vclose() {
  _socket_status = SOCKET_STATUS_CLOSE;
  return 0;
}

#ifndef _UDP_SOCKET_H__
#define _UDP_SOCKET_H__

#include <memory>

#include "socket.h"

class UdpSocket: public Socket, public std::enable_shared_from_this<UdpSocket> {
public:
  typedef std::shared_ptr<UdpSocket> ptr;

  UdpSocket(EVENTID mtype);

  virtual ~UdpSocket() { }

public:
  virtual int vinit(IPInfo &);
  virtual int vbind();
  virtual int vrecv();
  virtual int vsend(const char *buf, const int blen);
  virtual int vclose();

public:
  std::shared_ptr<UdpSocket> lrecv();
  int lsend(const std::shared_ptr<Socket> socket);
};

#endif//_UDP_SOCKET_H__

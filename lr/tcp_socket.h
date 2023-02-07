#ifndef _TCP_SOCKET_H__
#define _TCP_SOCKET_H__

#include <memory>

#include "socket.h"
#include "ipinfo.h"

class TcpSocket: public Socket, public std::enable_shared_from_this<TcpSocket> {
public:
  typedef std::shared_ptr<TcpSocket> ptr;

  TcpSocket(EVENTID stype);

  virtual ~TcpSocket() { }

public:
  virtual int vinit(std::shared_ptr<IPInfo> );
  virtual std::shared_ptr<Socket> vaccept();
  virtual int vbind();
  virtual int vconnect();
  virtual int vlisten();
  virtual int vrecv();
  virtual int vsend(const char *buf, const int blen);
  virtual int vclose();
};

#endif //_TCP_SOCKET_H__

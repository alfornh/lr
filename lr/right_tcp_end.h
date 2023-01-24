#ifndef _RIGHT_TCP_END_H__
#define _RIGHT_TCP_END_H__

#include <memory>
#include <mutex>
#include <unordered_map>

#include "ipinfo.h"
#include "listener.h"
#include "base_type.h"
#include "reactor.h"
#include "socket.h"

class Socket;

class RightTcpEnd: public Listener, public std::enable_shared_from_this<RightTcpEnd> {
public:
  typedef std::shared_ptr<RightTcpEnd> ptr;

  RightTcpEnd(IPInfo &ipi) { __ipi = ipi; }
  virtual ~RightTcpEnd() { }

  virtual int init();
  virtual int listen();
  virtual int notify_w_event(SOCKETID id, int fd);
  virtual int stop();

  virtual int l_recv(SOCKETID sid);
  virtual int l_write(SOCKETID sid);
  virtual int l_close(SOCKETID sid);

public:
  std::shared_ptr<Socket> make_right();
  //void start_heart_beat();

public:

  typedef std::unordered_map<SOCKETID, std::shared_ptr<Socket>> SocketContainer;
  SocketContainer _sockets;
  std::mutex _mutex_sockets;
};

#endif //_RIGHT_TCP_END_H__

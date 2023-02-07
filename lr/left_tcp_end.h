#ifndef _LEFT_TCP_END_H__
#define _LEFT_TCP_END_H__

#include <memory>
#include <mutex>
#include <unordered_map>

#include "plt/type-inc.h"

#include "ipinfo.h"
#include "listener.h"

class Socket;
class TcpSocket;
class LeftTcpEnd: public Listener, public std::enable_shared_from_this<LeftTcpEnd> {
public:
  typedef std::shared_ptr<LeftTcpEnd> ptr;

  LeftTcpEnd(std::shared_ptr<IPInfo> ipi) { _ipi = ipi; }

  virtual ~LeftTcpEnd() { }

  virtual int init();
  virtual int listen();
  virtual int notify_w_event(SOCKETID id, int fd);

  virtual int l_accept(SOCKETID sid);
  virtual int l_recv(SOCKETID sid);
  virtual int l_recv(SOCKETID sid, std::shared_ptr<BufferItem> bi);
  virtual int l_write(SOCKETID sid);
  virtual int l_close(SOCKETID sid);

  virtual int stop();

public:
  typedef std::unordered_map<SOCKETID, std::shared_ptr<Socket>> SocketContainer;
  SocketContainer _sockets;
  std::mutex _mutex_sockets;
};

#endif// _LEFT_TCP_END_H__

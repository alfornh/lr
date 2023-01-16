#ifndef _LEFT_UDP_END_H__
#define _LEFT_UDP_END_H__

#include <memory>
#include <mutex>
#include <unordered_map>

#include "ipinfo.h"
#include "base_type.h"
#include "listener.h"

class UdpSocket;
class LeftUdpEnd: public Listener, public std::enable_shared_from_this<LeftUdpEnd> {
public:
  typedef std::shared_ptr<LeftUdpEnd> ptr;

  LeftUdpEnd(IPInfo &ipi) { __ipi = ipi; }

  virtual ~LeftUdpEnd() {}


  virtual int init();
  virtual int listen();
  virtual int notify_w_event(SOCKETID id, int fd);

  //virtual int l_accept(SOCKETID sid);
  virtual int l_recv(SOCKETID sid);
  virtual int l_write(SOCKETID sid);
  virtual int l_close(SOCKETID sid);

  virtual int stop();

public:
  typedef std::unordered_map<SOCKETID, std::shared_ptr<UdpSocket> > SocketContainer;
  //typedef std::unordered_map<IPAddress, std::shared_ptr<UdpSocket>, HashIPAddress, EqualIPAddress > IPSocketContainer;

  SocketContainer _sockets;
  //IPSocketContainer _ip_sockets;

  std::mutex _mutex_sockets;
};

#endif//_LEFT_UDP_END_H__

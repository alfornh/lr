#ifndef _RIGHT_UDP_END_H__
#define _RIGHT_UDP_END_H__

#include <memory>
#include <mutex>
#include <unordered_map>

#include "plt/type-inc.h"

#include "ipinfo.h"
#include "listener.h"

class UdpSocket;
class RightUdpEnd: public Listener, public std::enable_shared_from_this<RightUdpEnd> {
public:
  typedef std::shared_ptr<RightUdpEnd> ptr;

  RightUdpEnd(std::shared_ptr<IPInfo> ipi) { _ipi = ipi; }
  virtual ~RightUdpEnd() { }

  virtual int init();
  virtual int listen();
  virtual int notify_w_event(SOCKETID id, int fd);
  virtual int stop();

  virtual int l_recv(SOCKETID sid);
  virtual int l_recv(SOCKETID sid, std::shared_ptr<BufferItem> bi);
  virtual int l_write(SOCKETID sid);
  virtual int l_close(SOCKETID sid);

public:
  std::shared_ptr<UdpSocket> make_right();
  //void start_heart_beat();

public:

  typedef std::unordered_map<SOCKETID, std::shared_ptr<UdpSocket> > SocketContainer;
  SocketContainer _sockets;
  std::mutex _mutex_sockets;
};

#endif //_RIGHT_UDP_END_H__

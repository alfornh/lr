#ifndef _LISTENER_H__
#define _LISTENER_H__

#include <memory>

#include "base_type.h"
#include "event_type.h"
#include "ipinfo.h"

class EpollReactor;
class Socket;
class Event;
class Listener {
public:
  Listener() { }

  virtual ~Listener() { }

  virtual int init() { return 0; }
  virtual int listen() { return 0; }
  virtual int notify_w_event(SOCKETID id, int fd) { return 0; }
  virtual int l_accept(SOCKETID sid) { return 0; }
  virtual int l_recv(SOCKETID sid) { return 0; }
  virtual int l_write(SOCKETID sid) { return 0; }
  virtual int l_close(SOCKETID sid) { return 0; }

  virtual int stop() { return 0; }

public:
  EVENTID _stype;

  IPInfo __ipi;
  std::shared_ptr<Socket> _main_socket;
  bool _stop_flag;
  std::shared_ptr<EpollReactor> _reactor;

  int _r_event_pool_id;
  //int _w_event_pool_id;

  int _lhbt;
};

#endif//_LISTENER_H__

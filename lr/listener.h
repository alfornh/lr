#ifndef _LISTENER_H__
#define _LISTENER_H__

#include <memory>

#include "plt/type-inc.h"

#include "event_type.h"
#include "ipinfo.h"

class BufferItem;
class Reactor;
class Socket;
class Event;
class Listener {
public:
  Listener() {
    _lhbt = 0;
    _stype = EVENT_TYPE_NULL;
    _stop_flag = true;
    _r_event_pool_id = 0;
  }

  virtual ~Listener() { }

  virtual int init() { return 0; }
  virtual int listen() { return 0; }
  virtual int notify_w_event(SOCKETID id, int fd) { return 0; }
  virtual int l_accept(SOCKETID sid) { return 0; }
  virtual int l_recv(SOCKETID sid) { return 0; }
  virtual int l_recv(SOCKETID sid, std::shared_ptr<BufferItem> bi) { return 0; }
  virtual int l_write(SOCKETID sid) { return 0; }
  virtual int l_close(SOCKETID sid) { return 0; }

  virtual int stop() { return 0; }

public:
  EVENTID _stype;
  bool _stop_flag;
  std::shared_ptr<IPInfo> _ipi;
  std::shared_ptr<Socket> _main_socket;
  std::shared_ptr<Reactor> _reactor;

  int _r_event_pool_id;
  //int _w_event_pool_id;

  int _lhbt;
};

#endif//_LISTENER_H__

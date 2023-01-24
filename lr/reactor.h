#ifndef _REACTOR_H__
#define _REACTOR_H__

#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>

#include "socket.h"
#include "ipinfo.h"

#define EPOLL_WAIT_EVENTS 16

enum {
  REACTOR_NULL,
  REACTOR_EPOLL,
  REACTOR_SELECT,
};

class Listener;
class Reactor {
public:
  Reactor() { }

  virtual ~Reactor() {}

  virtual int _init(IPInfo &) { return 0; }
  virtual int _listen() { return 0; }

  virtual int _add(SOCKETID sid, int fd) { return 0; }
  virtual int _del(SOCKETID sid, int fd) { return 0; }
  virtual int _mod(SOCKETID sid, int fd) { return 0; }

  virtual int notify_w_event(SOCKETID sid, int fd) { return 0; }

  virtual void _stop() { };

  virtual void listen_i_proc() { return ; }
  virtual void listen_o_proc() { return ; }

public:
  int _listen_i_proc_thread_group_id;
  int _listen_o_proc_thread_group_id;
  IPInfo _ipi;
  std::shared_ptr<Listener> _line;
  bool _stop_flag;

public:
  std::list<SOCKETID> _o_sockets;
  std::mutex _mutex_o_sockets;
  std::condition_variable _cv_o_sockets;
};

#endif //_REACTOR_H__


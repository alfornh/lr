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



class Listener;
class Reactor {
public:
  enum {
    PROTOCOL_NULL, PROTOCOL_TCP, PROTOCOL_UDP
  };
  enum {
    REACTOR_NULL, REACTOR_ASYNC, REACTOR_SELECT
  };
public:
  Reactor() {
    _name = REACTOR_NULL;
    _listen_i_proc_thread_group_id = 0;
    _listen_o_proc_thread_group_id = 0;
    _stop_flag = false;
    _protocol = PROTOCOL_NULL;
  }

  virtual ~Reactor() {}

  virtual int _init(std::shared_ptr<IPInfo> ) { return 0; }
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
  std::shared_ptr<IPInfo> _ipi;
  std::shared_ptr<Listener> _line;
  bool _stop_flag;
  int _name;

public:
  std::list<SOCKETID> _o_sockets;
  std::mutex _mutex_o_sockets;
  std::condition_variable _cv_o_sockets;
  int _protocol;
};

#endif //_REACTOR_H__


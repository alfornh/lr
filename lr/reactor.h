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
  Reactor() {
  }
  virtual ~Reactor() {}

  virtual int _init(IPInfo &) { return 0; }
  virtual int _listen() { return 0; }

  virtual int _add(SOCKETID sid, int fd) { return 0; }
  virtual int _del(SOCKETID sid, int fd) { return 0; }
  virtual int _mod(SOCKETID sid, int fd) { return 0; }

  virtual int notify_w_event(SOCKETID sid, int fd) { return 0; }

  virtual void _stop() { };

public:

  std::shared_ptr<Listener> _line;
  bool _stop_flag;
};

class EpollReactor : public Reactor {
public:
  EpollReactor() { }

  virtual ~EpollReactor();

  virtual int _init(IPInfo &);
  virtual int _listen();

  virtual int _add(SOCKETID sid, int fd);
  virtual int _del(SOCKETID sid, int fd);
  virtual int _mod(SOCKETID sid, int fd);

  virtual int notify_w_event(SOCKETID sid, int fd);

  virtual void _stop();

public:
  void listen_i_proc();
  void listen_o_proc();

public:
  int _epollfd;
  std::mutex _mutex_epollfd;

  int _listen_i_proc_thread_group_id;
  int _listen_o_proc_thread_group_id;
  IPInfo _ipi;

  std::list<SOCKETID> _o_sockets;
  std::mutex _mutex_o_sockets;
  std::condition_variable _cv_o_sockets;
};

#endif //_REACTOR_H__


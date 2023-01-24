#ifndef _REACTOR_EPOLL_H__
#define _REACTOR_EPOLL_H__

#include "reactor.h"

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
  virtual void listen_i_proc();
  virtual void listen_o_proc();

public:
  int _epollfd;
  std::mutex _mutex_epollfd;
};

#endif//_REACTOR_EPOLL_H__

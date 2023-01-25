#ifndef _REACTOR_SELECT_H__
#define _REACTOR_SELECT_H__

#include <map>
#include <memory>
#include <mutex>

#include "reactor.h"

class SelectReactor: public Reactor {
public:
  typedef std::shared_ptr<SelectReactor> ptr;

  SelectReactor() {}

  virtual ~SelectReactor() {}

  virtual int _init(IPInfo &);
  virtual int _listen();

  virtual int _add(SOCKETID sid, int fd);
  virtual int _del(SOCKETID sid, int fd);
  virtual int _mod(SOCKETID sid, int fd);

  virtual int notify_w_event(SOCKETID sid, int fd);

  virtual void _stop();

  virtual void listen_i_proc();
  virtual void listen_o_proc();

public:
  typedef std::map<int, SOCKETID> MAP_FD_SOCKETID;

  MAP_FD_SOCKETID _fd_sids;

  std::mutex _mutex_fd_sids;
};

#endif //_REACTOR_SELECT_H__

#ifndef _REACTOR_SELECT_H__
#define _REACTOR_SELECT_H__

#include <map>
#include <memory>
#include <mutex>

#include "reactor.h"

class SelectReactor: public Reactor {
public:
  typedef std::shared_ptr<SelectReactor> ptr;

  SelectReactor(int protocol) {
    _protocol = protocol;
    _name = Reactor::REACTOR_SELECT;
  }

  virtual ~SelectReactor() {}

  virtual int _init(std::shared_ptr<IPInfo> );
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
  std::mutex _mutex_fd_sids;
  MAP_FD_SOCKETID _fd_sids;
  std::condition_variable _cv_fd_sids;

private:
  void wait_before_select(MAP_FD_SOCKETID &fds);
  void signal_before_select(MAP_FD_SOCKETID &fds);

};

#endif //_REACTOR_SELECT_H__

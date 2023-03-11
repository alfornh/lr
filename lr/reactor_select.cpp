#include "plt/type-inc.h"
#include "plt/net-inc.h"

#include "ipinfo.h"
#include "listener.h"
#include "reactor_select.h"
#include "thread_manager.h"
#include "utils.h"
#include "zlog.h"

int SelectReactor::_init(std::shared_ptr<IPInfo> ipi) {
  _ipi = ipi;
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);

  _stop_flag = false;

  _listen_i_proc_thread_group_id = PTHREADMANAGER->reserve_thread_group();
  if (_listen_i_proc_thread_group_id < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
    return -1;
  }
  PTHREADMANAGER->start(_listen_i_proc_thread_group_id, _ipi->_reactor_thread_num);

  _listen_o_proc_thread_group_id = PTHREADMANAGER->reserve_thread_group();
  if (_listen_o_proc_thread_group_id < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
    return -1;
  }
  PTHREADMANAGER->start(_listen_o_proc_thread_group_id, _ipi->_io_thread_num);

  return 0;
}

void SelectReactor::_stop() {
  _stop_flag = true;
  _cv_fd_sids.notify_all();
  _cv_o_sockets.notify_all();
}

int SelectReactor::_listen() {
  for (int i = 0; i < _ipi->_reactor_thread_num; ++i) {
    RUN_TASK(_listen_i_proc_thread_group_id,
      MAKE_SHARED(Task, std::bind(&SelectReactor::listen_i_proc, this))
    );
  }

  for (int i = 0; i < _ipi->_io_thread_num; ++i) {
    RUN_TASK(_listen_o_proc_thread_group_id,
      MAKE_SHARED(Task, std::bind(&SelectReactor::listen_o_proc, this))
    );
  }

  return 0;
}

void SelectReactor::wait_before_select(MAP_FD_SOCKETID &fds) {
  while ( !_stop_flag ) {
    std::unique_lock<std::mutex> l(_mutex_fd_sids);
    _cv_fd_sids.wait(l, [this]{
      return (_stop_flag || _fd_sids.size() > 0);
    });

    if (_stop_flag) {
      return ;
    }

    if (_fd_sids.empty()) {
      continue;
    }

    fds.swap(_fd_sids);

    return;
  }
}

void SelectReactor::signal_before_select(MAP_FD_SOCKETID &fds) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_fd_sids)
  for (const auto &e: fds) {
    _fd_sids.insert(e);
  }
  LOCK_GUARD_MUTEX_END

  _cv_fd_sids.notify_one();
}

void SelectReactor::listen_i_proc() {
  int ret;
  int maxfd;
  int fd;
  int nfds;
  struct timeval tv;
  fd_set rfdset;

  MAP_FD_SOCKETID ifds;
  MAP_FD_SOCKETID afds;

  while ( !_stop_flag ) {

    maxfd = 0;

    tv.tv_sec = 10;
    tv.tv_usec = 0;

    FD_ZERO(&rfdset);

    ifds.clear();
    wait_before_select(ifds);

    for (const auto &e: ifds) {
      fd = e.first;//tit->first;
      if (fd > maxfd) {
        maxfd = fd;
      }
      FD_SET(fd, &rfdset);
    }

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "select");
    nfds = select(maxfd + 1, &rfdset, NULL, NULL, &tv);
    if (_stop_flag) {
      return ;
    }
    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "aft select", maxfd, nfds, errno);

    if (nfds == 0) {
      signal_before_select(ifds);
      continue;
    }
    if (nfds < 0) {
      if (errno != EINTR) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "select errno", errno);
        return ;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, "select EINTR");

      signal_before_select(ifds);
      continue;
    }

    if (_line->_main_socket) {//right end has no main_socket
      fd = _line->_main_socket->_fd;

      if (_protocol == Reactor::PROTOCOL_TCP && FD_ISSET(fd, &rfdset)) {

        _line->l_accept(_line->_main_socket->_id);

        FD_CLR(fd, &rfdset);

        --nfds;
      }
    }

    if (nfds < 1) {
      signal_before_select(ifds);
      continue;
    }

    afds.clear();
    MAP_FD_SOCKETID::iterator it = ifds.begin();
    while (it != ifds.end()) {
      if (FD_ISSET(it->first, &rfdset)) {
        afds[it->first] = it->second;
        it = ifds.erase(it);
      } else {
        ++it;
      }
    }

    signal_before_select(ifds);

    it = afds.begin();
    while ( it != afds.end() ) {
      ret = _line->l_recv(it->second);
      if (ret < 1) {
        _line->l_close(it->second);
        it = afds.erase(it);
      } else {
        ++it;
      }
    }

    signal_before_select(afds);
  }
}

void SelectReactor::listen_o_proc() {
  SOCKETID id;
  int ret;
  while (!_stop_flag) {
    {
      std::unique_lock<std::mutex> l(_mutex_o_sockets);
      _cv_o_sockets.wait(l, [this]{
        return (_stop_flag || _o_sockets.size() > 0);
      });

      if (_stop_flag) {
        return ;
      }

      if (_o_sockets.empty()) {
        continue;
      }

      id = _o_sockets.front();
      _o_sockets.pop_front();
    }

    ret = _line->l_write(id);

    if (ret > 0) {
      std::unique_lock<std::mutex> l(_mutex_o_sockets);
      _o_sockets.emplace_back(id);
    }
  }
}

int SelectReactor::_add(SOCKETID sid, int fd) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_fd_sids)
  _fd_sids[fd] = sid;
  LOCK_GUARD_MUTEX_END
  return 0;
}

int SelectReactor::_del(SOCKETID sid, int fd) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_fd_sids)
  _fd_sids.erase(fd);
  LOCK_GUARD_MUTEX_END
  return 0;
}

int SelectReactor::_mod(SOCKETID sid, int fd) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_fd_sids)
  _fd_sids[fd] = sid;
  LOCK_GUARD_MUTEX_END
  return 0;
}

int SelectReactor::notify_w_event(SOCKETID sid, int fd) {
  if (_stop_flag) {
    return 0;
  }

  {
    std::unique_lock<std::mutex> l(_mutex_o_sockets);
    _o_sockets.emplace_back(sid);
  }

  _cv_o_sockets.notify_one();

  return 1;
}

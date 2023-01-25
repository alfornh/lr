#include <sys/select.h>

#include "ipinfo.h"
#include "listener.h"
#include "reactor_select.h"
#include "thread_manager.h"
#include "utils.h"
#include "zlog.h"

int SelectReactor::_init(IPInfo &ipi) {
  _ipi = ipi;
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);
  _listen_i_proc_thread_group_id = PTHREADMANAGER->reserve_thread_group();
  if (_listen_i_proc_thread_group_id < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
    return -1;
  }
  PTHREADMANAGER->start(_listen_i_proc_thread_group_id, _ipi._reactor_thread_num);

  _listen_o_proc_thread_group_id = PTHREADMANAGER->reserve_thread_group();
  if (_listen_o_proc_thread_group_id < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "reserve_thread_group");
    return -1;
  }
  PTHREADMANAGER->start(_listen_o_proc_thread_group_id, _ipi._reactor_thread_num);

  _stop_flag = false;

  return 0;
}

void SelectReactor::_stop() {
  _stop_flag = true;
  _cv_o_sockets.notify_all();
}

int SelectReactor::_listen() {
  for (int i = 0; i < _ipi._reactor_thread_num; ++i) {
    RUN_TASK(_listen_i_proc_thread_group_id,
      MAKE_SHARED(Task, std::bind(&SelectReactor::listen_i_proc, this))
    );
  }
  for (int i = 0; i < _ipi._io_thread_num; ++i) {
    RUN_TASK(_listen_o_proc_thread_group_id,
      MAKE_SHARED(Task, std::bind(&SelectReactor::listen_o_proc, this))
    );
  }

  return 0;
}

void SelectReactor::listen_i_proc() {
  int ret;
  int maxfd;
  int fd;
  struct timeval tv;
  fd_set _rfds;
  fd_set _efds;
  MAP_FD_SOCKETID fd_sids;

  while ( !_stop_flag ) {
    maxfd = 0;
    tv.tv_sec = 30;
    tv.tv_usec = 0;

    FD_ZERO(&_rfds);
    FD_ZERO(&_efds);

    LOCK_GUARD_MUTEX_BEGIN(_mutex_fd_sids)
    fd_sids = _fd_sids;
    LOCK_GUARD_MUTEX_END

    MAP_FD_SOCKETID::reverse_iterator tit = fd_sids.rbegin();
    while (tit != fd_sids.rend()) {
      fd = tit->first;
      if (fd > maxfd) {
        maxfd = fd;
      }
      FD_SET(fd, &_rfds);
      FD_SET(fd, &_efds);

      tit++;
    }

    if (_stop_flag) {
      return ;
    }
    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "select");
    ret = select(maxfd + 1, &_rfds, NULL, &_efds, &tv);

    if (_stop_flag) {
      return ;
    }
    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "aft select", maxfd, ret, errno);

    if (ret < 0) {
      if (errno != EINTR) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "select errno", errno);
        return ;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, "select EINTR");
      continue;
    }

    if (_line->_main_socket) {//right end has no main_socket
      if (_line->_main_socket->_stype != EVENT_TYPE_SOCKET_UDP
        && (FD_ISSET(_line->_main_socket->_fd, &_rfds))) {

        _line->l_accept(_line->_main_socket->_id);
        FD_CLR(_line->_main_socket->_fd, &_rfds);

        ret--;
      }
    }

    if (ret < 1) {
      continue;
    }

    for (const auto &e: fd_sids) {
      if (FD_ISSET(e.first, &_rfds)) {
        ret = _line->l_recv(e.second);
        if (ret < 0) {
          _line->l_close(e.second);
        }
        --ret;
      }

      if (FD_ISSET(e.first, &_efds)) {
        _line->l_close(e.second);
        --ret;
      }

      if (ret < 1) {
        break;
      }
    }

  }
}

void SelectReactor::listen_o_proc() {
  SOCKETID id;
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

    _line->l_write(id);
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

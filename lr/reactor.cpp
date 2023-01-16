#include "reactor.h"

#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#include "socket.h"
#include "zlog.h"
#include "thread_manager.h"
#include "utils.h"
#include "listener.h"

EpollReactor::~EpollReactor() {
  close(_epollfd);
}

int EpollReactor::_init(IPInfo &ipi) {
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
  _epollfd = epoll_create1(EPOLL_CLOEXEC);

  return 0;

}

void EpollReactor::_stop() {
  _stop_flag = true;
  Reactor::_stop();

  //_epoll_event_buffer->stop();

  static int tfd = -1;
  if ( tfd < 0) {
    tfd = timerfd_create(CLOCK_REALTIME, 0);
  }

  if (tfd < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "timerfd_create error");
    return ;
  }

  struct itimerspec iti;
  iti.it_value.tv_sec = 1;
  iti.it_value.tv_nsec= 0;
  iti.it_interval.tv_sec = 0;
  iti.it_interval.tv_nsec = 0;
  int ret = timerfd_settime(tfd, TFD_TIMER_ABSTIME, &iti, NULL);
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "timerfd_settime error");
    return ;
  }
  

  _add(0, tfd);

  _cv_o_sockets.notify_all();
}

int EpollReactor::_listen() {
  for (int i = 0; i < _ipi._reactor_thread_num; ++i) {
    RUN_TASK(_listen_i_proc_thread_group_id,
      MAKE_SHARED(Task, std::bind(&EpollReactor::listen_i_proc, this))
    );
  }
  for (int i = 0; i < _ipi._io_thread_num; ++i) {
    RUN_TASK(_listen_o_proc_thread_group_id,
      MAKE_SHARED(Task, std::bind(&EpollReactor::listen_o_proc, this))
    );
  }

  return 0;
}

void EpollReactor::listen_i_proc() {
  BufferItem::ptr bi = MAKE_SHARED(BufferItem);
  int i, what;
  SOCKETID id;
  struct epoll_event *ee;

  while (!_stop_flag) {
    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "epoll wait");

    LOCK_GUARD_MUTEX_BEGIN(_mutex_epollfd)
    if (_stop_flag) {
      return ;
    }
    bi->_len = epoll_wait(
      _epollfd,
      (struct epoll_event *)(bi->_buffer),
      EPOLL_WAIT_EVENTS, //BufferItem::buffer_item_capacity/sizeof(struct epoll_event),
      -1
    );
    LOCK_GUARD_MUTEX_END
    if (_stop_flag) {
      return ;
    }
    ZLOG_DEBUG(__FILE__, __LINE__, __func__, "aft epoll wait", bi->_len, errno);

    if (bi->_len < 0) {
      if (errno != EINTR) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "epoll_wait errno", errno);
        return ;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, "epoll_wait EINTR");
      continue;
    }

    ee = (struct epoll_event *)(bi->_buffer);
    for (i = 0; i < bi->_len; ++i) {
      what = ee[i].events;
      id = ee[i].data.u64;

      if (_line->_main_socket) {//right end has no main_socket
        if (_line->_main_socket->_stype != EVENT_TYPE_SOCKET_UDP && id == _line->_main_socket->_id) {
          _line->l_accept(id);
          continue;
        }
      }

      if (what & (EPOLLIN | EPOLLPRI)) {

        _line->l_recv(id);
      }

      if (what & (EPOLLRDHUP | EPOLLERR)) {

        _line->l_close(id);
      }
    }
  }
}

void EpollReactor::EpollReactor::listen_o_proc() {
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

int EpollReactor::_add(SOCKETID sid, int fd) {
  struct epoll_event ee;
  memset(&ee, 0x0, sizeof(ee));
  ee.data.u64 = sid;
  ee.events = EPOLLPRI | EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET;

  int ret = epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ee);
  if (EEXIST == ret) {
    return epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ee);
  }

  return ret;
}

int EpollReactor::_del(SOCKETID sid, int fd) {
  struct epoll_event ee;
  memset(&ee, 0x0, sizeof(ee));
  ee.data.u64 = sid;
  return epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, &ee);
}

int EpollReactor::_mod(SOCKETID sid, int fd) {
  struct epoll_event ee;
  memset(&ee, 0x0, sizeof(ee));
  ee.data.u64 = sid;
  ee.events = EPOLLPRI | EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET ;
  
  int ret = epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ee);
  if (ENOENT == ret) {
    return epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ee);
  }

  return ret;
}

int EpollReactor::notify_w_event(SOCKETID sid, int fd) {
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


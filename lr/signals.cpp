#include "signals.h"

#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>

#include "plt/oth-inc.h"

#include "event_type.h"
#include "event.h"
#include "event_pool.h"
#include "zlog.h"
#include "left_object.h"
#include "thread_manager.h"

std::shared_ptr<Signals> Signals::_instance = std::make_shared<Signals>();
bool Signals::_stop_flag = false;

Signals::Signals() {
  _stype = EVENT_TYPE_SIGNAL;
}

int Signals::init() {

  Signals::_stop_flag = false;

  int ret = atexit(&Signals::clean_up);
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "atexit setting error");
    return ret;
  }

  set_signal_handler(SIGHUP,   &Signals::signal_handler);
  set_signal_handler(SIGINT,   &Signals::signal_handler);
  set_signal_handler(SIGQUIT,  &Signals::signal_handler);
  set_signal_handler(SIGBUS,   &Signals::signal_handler);
  set_signal_handler(SIGKILL,  &Signals::signal_handler);
  set_signal_handler(SIGSEGV,  &Signals::signal_handler);
  set_signal_handler(SIGALRM,  &Signals::signal_handler);
  set_signal_handler(SIGILL,   &Signals::signal_handler);
  set_signal_handler(SIGFPE,   &Signals::signal_handler);
  set_signal_handler(SIGTERM,  &Signals::signal_handler);
  //set_signal_handler(SIGBREAK , &Signals::signal_handler);
  //set_signal_handler(SIGABRT,  &Signals::signal_handler);
  
  _r_event_pool_id = EventPool::reserve_event_queue();
  _thread_group_id = PTHREADMANAGER->reserve_thread_group();
  PTHREADMANAGER->start(_thread_group_id, 1);
  // this thread used by dispatcher ..

  return 0;
}

void Signals::signal_handler(int sig) {
  Event::ptr e = MAKE_SHARED(Event);

  if (Signals::_stop_flag) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "caught signal:", sig);
    return ;
  }

  switch (sig) {
  case SIGBUS:
    ZLOG_WARN(__FILE__, __LINE__, __func__, "SIGBUS ", sig, " caught");
    dump_stack();
  break;
  case SIGSEGV:
    ZLOG_WARN(__FILE__, __LINE__, __func__, "SIGSEGV ", sig, " caught");
    dump_stack();
  break;
  case SIGQUIT:
    ZLOG_WARN(__FILE__, __LINE__, __func__, "SIGQUIT ", sig, " caught");
    dump_stack();
  break;
  case SIGHUP:
    e->_stype = EVENT_TYPE_SIGNAL_SIGHUP;
    ZLOG_WARN(__FILE__, __LINE__, __func__, "SIGHUP ", sig, " caught");
  break;
  case SIGKILL:
    e->_stype = EVENT_TYPE_SIGNAL_SIGKILL;
    ZLOG_WARN(__FILE__, __LINE__, __func__, "SIGKILL ", sig, " caught");
  break;
  case SIGINT:
    e->_stype = EVENT_TYPE_SIGNAL_SIGINT;
    ZLOG_WARN(__FILE__, __LINE__, __func__, "SIGINT ", sig, " caught");
  break;

  case SIGALRM:
    e->_stype = EVENT_TYPE_SIGNAL_SIGALRM;
    ZLOG_WARN(__FILE__, __LINE__, __func__, "SIGALRM ", sig, " caught");
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown sig caught ", sig);
  }

  ADD_EVENT(PSIGNALS->_r_event_pool_id, e);

  return ;
}

void Signals::dump_stack(void) {
  char gdbcommand[160] = {0};

  sprintf(gdbcommand, "gdb -q  --pid %d --batch -ex generate-core-file -ex kill -ex quit", getpid());

  system(gdbcommand);

  return;
}


void Signals::clean_up(void) {
  ZLOG_WARN(__FILE__, __LINE__, __func__, "server stopped");
  /* Do any cleaning up chores here */

  return ;
}

void Signals::stop() {
  ZLOG_WARN(__FILE__, __LINE__, __func__);
  Signals::_stop_flag = true;

}


#include "signals.h"

#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>

#include "event_type.h"
#include "event.h"
#include "event_pool.h"
#include "zlog.h"
#include "left_object.h"
#include "thread_manager.h"

std::shared_ptr<Signals> Signals::_instance = std::make_shared<Signals>();
bool Signals::_stop_flag = false;

static struct sigaction _sigact;

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

  sigset_t set, oldset;
  
  // initialize set (first add all signals, then remove SIGINT)
  sigfillset(&set);
  sigdelset(&set, SIGINT);
  sigdelset(&set, SIGSEGV);
  sigdelset(&set, SIGBUS);
  sigdelset(&set, SIGQUIT);
  sigdelset(&set, SIGHUP);
  sigdelset(&set, SIGKILL);
  sigdelset(&set, SIGALRM);

  // install new signal mask and get previously installed signal mask
  // all signals are blocked except for SIGINT SIGSEGV SIGBUS SIGQUIT SIGHUP SIGKILL SIGALRM
  sigprocmask(SIG_SETMASK, &set, &oldset);

  sigemptyset(&_sigact.sa_mask);
  _sigact.sa_handler = &Signals::signal_handler;
  _sigact.sa_flags = SA_RESTART;


  sigaddset(&_sigact.sa_mask, SIGINT);
  sigaction(SIGINT, &_sigact, (struct sigaction *)NULL);

  sigaddset(&_sigact.sa_mask, SIGSEGV);
  sigaction(SIGSEGV, &_sigact, (struct sigaction *)NULL);

  sigaddset(&_sigact.sa_mask, SIGBUS);
  sigaction(SIGBUS, &_sigact, (struct sigaction *)NULL);

  sigaddset(&_sigact.sa_mask, SIGQUIT);
  sigaction(SIGQUIT, &_sigact, (struct sigaction *)NULL);

  sigaddset(&_sigact.sa_mask, SIGHUP);
  sigaction(SIGHUP, &_sigact, (struct sigaction *)NULL);

  sigaddset(&_sigact.sa_mask, SIGKILL);
  sigaction(SIGKILL, &_sigact, (struct sigaction *)NULL);

  sigaddset(&_sigact.sa_mask, SIGALRM);
  sigaction(SIGALRM, &_sigact, (struct sigaction *)NULL);

  _r_event_pool_id = EventPool::reserve_event_queue();
  _thread_group_id = PTHREADMANAGER->reserve_thread_group();
  PTHREADMANAGER->start(_thread_group_id, 1);
  // this thread used by dispatcher ..

  return 0;
}

void Signals::signal_handler(int sig) {
  Event::ptr e = std::make_shared<Event>();

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
  sigemptyset(&_sigact.sa_mask);
  /* Do any cleaning up chores here */

  return ;
}

void Signals::stop() {
  ZLOG_WARN(__FILE__, __LINE__, __func__);
  Signals::_stop_flag = true;

  sigset_t set, oldset;
  sigfillset(&set);
  sigprocmask(SIG_SETMASK, &set, &oldset);
}


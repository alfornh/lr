#ifndef _OTH_INC_H__
#define _OTH_INC_H__

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "plt/oth-inc.h"
#include "plt/type-inc.h"

extern "C" {

void init_signals() {
  sigset_t set;
  // initialize set (first add all signals, then remove SIGINT,  SIGINT SIGSEGV SIGBUS SIGQUIT SIGHUP SIGKILL SIGALRM
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
  sigprocmask(SIG_SETMASK, &set, NULL);
}

void set_signal_handler(int sig, SignalHandler sh) {
  static bool init = false;
  if ( !init ) {
    init_signals();
    init = true;
  }

  struct sigaction _sigact;
  sigemptyset(&_sigact.sa_mask);
  _sigact.sa_handler = sh;
  _sigact.sa_flags = SA_RESTART;

  switch (sig) {
  case SIGHUP:
  case SIGINT:
  case SIGQUIT:
  case SIGBUS:
  case SIGKILL:
  case SIGSEGV:
  case SIGALRM:
    sigaddset(&_sigact.sa_mask, sig);
    sigaction(sig, &_sigact, (struct sigaction *)NULL);
  break;
  }
}


THREADID get_thread_id() {
  return pthread_self();
}

}

#endif//_OTH_INC_H__

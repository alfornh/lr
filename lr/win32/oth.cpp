
#include <Windows.h>
#include <signal.h>

#include "plt/oth-inc.h"
#include "plt/type-inc.h"

extern "C" {

void set_signal_handler(int sig, SignalHandler sh) {
  switch (sig) {
  case SIGINT:
  case SIGILL:
  case SIGFPE:
  case SIGSEGV:
  case SIGTERM:
  case SIGBREAK:
  case SIGABRT:
    signal(sig, sh);
  break;
  }
}

THREADID get_thread_id() {
  return GetCurrentThreadId();
}

}
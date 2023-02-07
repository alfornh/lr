#ifndef _OTH_INC_H__
#define _OTH_INC_H__

#include "type-inc.h"

extern "C" {

#ifdef SIGHUP
  #undef SIGHUP
  #define SIGHUP         1
#endif //

#ifdef SIGINT
  #undef SIGINT
  #define SIGINT         2
#endif

#ifdef SIGQUIT
  #undef SIGQUIT
  #define SIGQUIT        3
#endif

#ifdef SIGBUS
  #undef SIGBUS
  #define SIGBUS         7
#endif

#ifdef SIGKILL
  #undef SIGKILL
  #define SIGKILL        9
#endif

#ifdef SIGSEGV
  #undef SIGSEGV
  #define SIGSEGV        11
  // segment violation
#endif


#ifdef SIGALRM
  #undef SIGALRM
  #define SIGALRM        14
#endif

//#define SIGINT       2   // interrupt
#ifdef SIGILL
  #undef SIGILL
  #define SIGILL         4
  // illegal instruction - invalid function image
#endif

#ifdef SIGFPE
  #undef SIGFPE
  #define SIGFPE         8
  // floating point exception
#endif

//#define SIGSEGV      11  // segment violation

#ifdef SIGTERM
  #undef SIGTERM
  #define SIGTERM        15
  // Software termination signal from kill
#endif

#ifdef SIGBREAK
  #undef SIGBREAK
  #define SIGBREAK 21
  // Ctrl-Break sequence
#endif

#ifdef SIGABRT
  #undef SIGABRT
  #define SIGABRT
#endif

int get_thread_id();

void set_signal_handler(int sig, SignalHandler sh);

}

#endif//_OTH_INC_H__

#ifndef _OTH_INC_H__
#define _OTH_INC_H__

//#ifdef _WIN32

#include "type-inc.h"

extern "C" {

#define SIGHUP        1
#define SIGINT         2
#define SIGQUIT        3
#define SIGBUS         7
#define SIGKILL        9
#define SIGSEGV        11  // segment violation
#define SIGALRM        14

//#define SIGINT          2   // interrupt
#define SIGILL         4   // illegal instruction - invalid function image
#define SIGFPE         8   // floating point exception
//#define SIGSEGV        11  // segment violation
#define SIGTERM        15  // Software termination signal from kill
#define SIGBREAK       21  // Ctrl-Break sequence
#define SIGABRT        22  // abnormal termination triggered by abort call

THREADID get_thread_id();

void set_signal_handler(int sig, SignalHandler sh);


}

//#endif//_WIN32

#endif//_OTH_INC_H__
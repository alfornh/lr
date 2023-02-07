#ifndef _TYPE_INC_H__
#define _TYPE_INC_H__

#include <limits.h>
#include <stdint.h>

typedef unsigned int   TIME_T;
typedef unsigned int   SOCKETID;
typedef uint32_t       EVENTID;
typedef int            PORTID;
typedef unsigned int   ID;
typedef unsigned long  THREADID;


typedef void (*SignalHandler)(int);

#define ID_MAX     INT_MAX

#endif//_TYPE_INC_H__
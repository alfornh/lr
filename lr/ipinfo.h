#ifndef _IPINFO_H__
#define _IPINFO_H__

#include <string>

typedef struct _IPInfo {
  _IPInfo():_protocal(0), _ip(""), _port(0), _reactor_thread_num(0), _io_thread_num(0) {}

  int _protocal;
  std::string _ip;
  int _port;

  int _reactor_thread_num;
  int _io_thread_num;
} IPInfo;

#endif//_IPINFO_H__

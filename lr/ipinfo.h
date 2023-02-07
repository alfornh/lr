#ifndef _IPINFO_H__
#define _IPINFO_H__

#include <memory>
#include <string>

struct IPInfo {
  typedef std::shared_ptr<IPInfo> ptr;

  IPInfo() {
    _protocal = 0;
    _port = 0;
    _reactor_thread_num = 0;
    _io_thread_num = 0;
    _ip = "";
  }

  int _protocal;
  int _port;
  int _reactor_thread_num;
  int _io_thread_num;
  std::string _ip;
};

#endif//_IPINFO_H__

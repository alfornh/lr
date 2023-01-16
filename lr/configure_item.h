#ifndef _CONFIGURE_ITEM_H__
#define _CONFIGURE_ITEM_H__

#include <memory>
#include <vector>
#include <string>

#include "ipinfo.h"

enum {
  PROTOCAL_NULL,
  PROTOCAL_TCP,
  PROTOCAL_UDP,
  PROTOCAL_WEBSOCKET,
  PROTOCAL_HTTP,
};

enum {
  FLAG_NULL,
  FLAG_READ,
  FLAG_WRITE,
};
enum {
  CONFIGURE_NULL               = 0x0000,
  CONFIGURE_TYPE_MASK          = 0xFF00,


  CONFIGURE_SERVER_INFO        = 0x0101,
  CONFIGURE_TIMER              = 0x0201,

  CONFIGURE_RIGHT_END          = 0x0300,
  CONFIGURE_RIGHT_END_TCP      = 0x0301,
  CONFIGURE_RIGHT_END_UDP      = 0x0302,

  CONFIGURE_LEFT_END           = 0x0400,
  CONFIGURE_LEFT_END_TCP       = 0x0401,
  CONFIGURE_LEFT_END_WEBSOCKET = 0x0402,
  CONFIGURE_LEFT_END_HTTP      = 0x0403,
  CONFIGURE_LEFT_END_UDP       = 0x0404,
};

class ConfigureItem {
public:
  typedef std::shared_ptr<ConfigureItem> ptr;

  ConfigureItem() {
    _id = CONFIGURE_NULL;
    _enable = false;
  }
  virtual ~ConfigureItem() {}


  bool _enable;
  int _id;
};

class ConfigureNetworkLine: public ConfigureItem {
public:
  typedef std::shared_ptr<ConfigureNetworkLine> ptr;

  ConfigureNetworkLine() {
    _id = CONFIGURE_NULL;
    _max_worker_thread_num = 0;
  }
  virtual ~ConfigureNetworkLine() {}

  int _max_worker_thread_num;
  std::vector<IPInfo> _lines;
};

class ConfigureServerInfo: public ConfigureItem {
public:
  typedef std::shared_ptr<ConfigureServerInfo> ptr;

  ConfigureServerInfo() {
    _id = CONFIGURE_SERVER_INFO;
  }
  virtual ~ConfigureServerInfo() {}

  std::string _server_id;
  std::string _server_type;
  std::string _server_name;
  std::string _notes;
  int _max_concurrency;
};

//class ConfigureSignal: public ConfigureItem {
//public:
//  typedef std::shared_ptr<ConfigureSignal> ptr;
//
//  ConfigureSignal() {
//    _id = CONFIGURE_SIGNAL;
//    _max_worker_thread_num = 2;
//  }
//  virtual ~ConfigureSignal() {}
//
//  int _max_worker_thread_num;
//};

class ConfigureTimer: public ConfigureItem {
public:
  typedef std::shared_ptr<ConfigureTimer> ptr;

  ConfigureTimer() {
    _id = CONFIGURE_TIMER;
    _max_worker_thread_num = 2;
  }
  virtual ~ConfigureTimer() {}

  int _max_worker_thread_num;
};

class ConfigureRightEnd: public ConfigureItem {
public:
  typedef std::shared_ptr<ConfigureRightEnd> ptr;

  ConfigureRightEnd() {
    _id  = CONFIGURE_RIGHT_END;
  }

  std::shared_ptr<ConfigureNetworkLine> _tcp;
  std::shared_ptr<ConfigureNetworkLine> _udp;
};

class ConfigureLeftEnd: public ConfigureItem {
public:
  typedef std::shared_ptr<ConfigureLeftEnd> ptr;

  ConfigureLeftEnd() {
    _id = CONFIGURE_LEFT_END;
  }

  std::shared_ptr<ConfigureNetworkLine> _tcp;
  std::shared_ptr<ConfigureNetworkLine> _websocket;
  std::shared_ptr<ConfigureNetworkLine> _http;
  std::shared_ptr<ConfigureNetworkLine> _udp;
};

#endif//_CONFIGURE_ITEM_H__

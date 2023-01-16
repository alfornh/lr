#ifndef _LEFT_EVENT_DISPATCHER_H__
#define _LEFT_EVENT_DISPATCHER_H__

#include <memory>

#define PLEFTEVENTDISPATCHER LeftEventDispatcher::_instance

class LeftEventHandler;
class LeftEventDispatcher {
public:
  typedef std::shared_ptr<LeftEventDispatcher> ptr;

  LeftEventDispatcher();

  int init();

  int dispatch();

  void stop();

private:
  void dispatch_tcp(int);
  void dispatch_websocket(int);
  void dispatch_http(int);
  void dispatch_udp(int);
  void dispatch_timer(int);

private:
  bool _stop_flag;
  std::shared_ptr<LeftEventHandler> _handler;

  int _tcp_thread_group_id;
  int _websocket_thread_group_id;
  int _http_thread_group_id;
  int _udp_thread_group_id;
  int _timer_thread_group_id;

public:
  static std::shared_ptr<LeftEventDispatcher> _instance;
};

#endif// _LEFT_EVENT_DISPATCHER_H__

#ifndef _LEFT_EVENT_LISTENER_H__
#define _LEFT_EVENT_LISTENER_H__

#include <memory>
#include <vector>

#define PLEFTEVENTLISTENER LeftEventListener::_instance

class LeftTcpEnd;
class LeftUdpEnd;
class LeftEventListener {
public:
  typedef std::shared_ptr<LeftEventListener> ptr;

  LeftEventListener();

  int init();

  int listen();

  void stop();


public:
  std::vector<std::shared_ptr<LeftTcpEnd>> _tcp_listeners;
  std::vector<std::shared_ptr<LeftTcpEnd>> _websocket_listeners;
  std::vector<std::shared_ptr<LeftTcpEnd>> _http_listeners;
  std::vector<std::shared_ptr<LeftUdpEnd>> _udp_listeners;

  bool _stop_flag;

public:
  static std::shared_ptr<LeftEventListener> _instance;
};

#endif //_LEFT_EVENT_LISTENER_H__

#ifndef _RIGHT_EVENT_LISTENER_H__
#define _RIGHT_EVENT_LISTENER_H__

#include <memory>
#include <vector>

#define PRIGHTEVENTLISTENER RightEventListener::_instance


class Socket;
class RightTcpEnd;
class RightUdpEnd;

typedef std::vector<std::shared_ptr<RightTcpEnd>> RightTcpEndContainer;
typedef std::vector<std::shared_ptr<RightUdpEnd>> RightUdpEndContainer;

class RightEventListener {
public:
  typedef std::shared_ptr<RightEventListener> ptr;

  RightEventListener();

  int init();

  int listen();

  void stop();

public:
  RightTcpEndContainer _tcp_listeners;
  RightUdpEndContainer _udp_listeners;

  bool _stop_flag;

public:
  static std::shared_ptr<RightEventListener> _instance;
};

#endif //_RIGHT_EVENT_LISTENER_H__

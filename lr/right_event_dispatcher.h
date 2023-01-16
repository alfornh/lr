#ifndef _RIGHT_EVENT_DISPATCHER_H__
#define _RIGHT_EVENT_DISPATCHER_H__

#include <memory>

#define PRIGHTEVENTDISPATCHER RightEventDispatcher::_instance

class RightEventHandler;
class RightEventDispatcher {
public:
  typedef std::shared_ptr<RightEventDispatcher> ptr;

  RightEventDispatcher() { }

  int init();

  int dispatch();

  void stop();

private:
  int dispatch_tcp(int);
  int dispatch_udp(int);

private:
  bool _stop_flag;
  std::shared_ptr<RightEventHandler> _handler;

  int _tcp_thread_group_id;
  int _udp_thread_group_id;

public:
  static std::shared_ptr<RightEventDispatcher> _instance;
};



#endif //_RIGHT_EVENT_DISPATCHER_H__

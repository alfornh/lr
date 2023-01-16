#ifndef _LEFT_OBJECT_H__
#define _LEFT_OBJECT_H__

#include <memory>

#define PLEFTOBJECT LeftObject::_instance

class Handler;
class LeftObject {
public:
  LeftObject();

  int init();

  void set_event_handlers(
    std::shared_ptr<Handler> tcp_handler,
    std::shared_ptr<Handler> http_handler,
    std::shared_ptr<Handler> websocket_handler,
    std::shared_ptr<Handler> udp_handler,
    std::shared_ptr<Handler> timer_handler,
    std::shared_ptr<Handler> signal_handler
  );

  int run();

  int wait();

  void stop();


private:
  int dispatch_signal();

public:
  static std::shared_ptr<LeftObject> _instance;
  bool _stop_flag;
};

#endif //_LEFT_OBJECT_H__

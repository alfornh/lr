#ifndef _GATE_OBJECT_H__
#define _GATE_OBJECT_H__

#include <memory>

#define PRIGHTOBJECT RightObject::_instance

class Handler;
class RightObject {
public:
  typedef std::shared_ptr<RightObject> ptr;

  int init();

  void set_event_handlers(
    std::shared_ptr<Handler> tcp_handler,
    std::shared_ptr<Handler> udp_handler
  );

  int run();

  void stop();

public:
  static std::shared_ptr<RightObject> _instance;
};

#endif//_GATE_OBJECT_H__

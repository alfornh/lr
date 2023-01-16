#ifndef _SIGNALS_H__
#define _SIGNALS_H__

#include <memory>

#include "event.h"

#define PSIGNALS Signals::_instance

class Signals: public ESource {
public:

  Signals();
  virtual ~Signals() {}

  int init();

  static void signal_handler(int sig);
  static void dump_stack(void);
  static void clean_up(void);

  void stop();
public:
  static std::shared_ptr<Signals> _instance;
private:
  static bool _stop_flag;

public:
  int _r_event_pool_id;
  int _thread_group_id;
};

#endif //_SIGNALS_H__

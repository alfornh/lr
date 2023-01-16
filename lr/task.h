#ifndef _TASK_H__
#define _TASK_H__

#include <memory>
#include <functional>

#include "event.h"

class Task {
public:
  typedef std::shared_ptr<Task> ptr;

  #define FUNC_1_ std::function<void()>
  #define FUNC_2_ std::function<void(int)>
  #define FUNC_3_ std::function<void(std::shared_ptr<Event>)>

  enum {
    _FUNC_NULL__,
    _FUNC_1__,
    _FUNC_2__,
    _FUNC_3__,
  };

  Task(FUNC_1_ f) {
    _task_index = _FUNC_1__;
    _1 = f;
  }

  Task(FUNC_2_ f, int i) {
    _task_index = _FUNC_2__;
    _2 = f;
    _p_f_2 = i;
  }

  Task(FUNC_3_ f, std::shared_ptr<Event> ev) {
    _task_index = _FUNC_3__;
    _3 = f;
    _p_f_3 = ev;
  }

  void operator ()() {
    switch (_task_index) {
    case _FUNC_1__:
      _1();
    break;

    case _FUNC_2__:
      _2(_p_f_2);
    break;

    case _FUNC_3__:
      _3(_p_f_3);
    break;
    }
  }

  int _p_f_2;
  std::shared_ptr<Event> _p_f_3;

  FUNC_1_ _1;
  FUNC_2_ _2;
  FUNC_3_ _3;

  int _task_index;
};

#endif//_TASK_H__

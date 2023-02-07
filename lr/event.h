#ifndef _EVENT_H__
#define _EVENT_H__

#include <memory>

#include "plt/type-inc.h"

#include "data_buffer.h"
#include "event_type.h"


class ESource {
public:
  ESource() {
    _stype = EVENT_TYPE_NULL;
  }
  virtual ~ESource() { }

public:

  EVENTID _stype;
};


class Event {
public:
  typedef std::shared_ptr<Event> ptr;

  Event() {
    _stype = EVENT_TYPE_NULL;
  }

  //std::shared_ptr<DataBuffer> _db;
  std::shared_ptr<ESource> _es;
  EVENTID _stype;
};

#endif//_EVENT_H__

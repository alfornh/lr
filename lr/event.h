#ifndef _EVENT_H__
#define _EVENT_H__

#include <memory>

#include "base_type.h"
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
  //int _r_event_pool_id;
  //int _w_event_pool_id;
};


class Event {
public:
  typedef std::shared_ptr<Event> ptr;

  Event() { }

  //std::shared_ptr<DataBuffer> _db;
  std::shared_ptr<ESource> _es;
  EVENTID _stype;
};

#endif//_EVENT_H__

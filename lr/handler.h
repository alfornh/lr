#ifndef _HANDLER_H__
#define _HANDLER_H__

#include <memory>

#include "plt/type-inc.h"
#include "event_type.h"

class Event;
class Handler {
public:
  typedef std::shared_ptr<Handler> ptr;

  Handler() { _stype = EVENT_TYPE_NULL; }

  virtual ~Handler() {}

  virtual int handle(EVENTID id, std::shared_ptr<Event>) { return 0; }

public:
  EVENTID _stype;
};




#endif //_HANDLER_H__

#ifndef _EVENT_POOL_H__
#define _EVENT_POOL_H__

#include <memory>
#include <unordered_map>

#include "event_queue.h"

#define GET_EVENT EventPool::get_event
#define ADD_EVENT EventPool::add_event

#define GET_EVENT_NO_SYNC EventPool::get_event_no_sync
#define ADD_EVENT_NO_SYNC EventPool::add_event_no_sync

class Event;
class EventPool {
public:
  typedef std::shared_ptr<EventPool> ptr;

  EventPool();

  //return -1, or qid of type queue
  static int reserve_event_queue();

  static void stop();

public:
  static int add_event(int sid, std::shared_ptr<Event> event);
  static std::shared_ptr<Event> get_event(int sid);

  static int add_event_no_sync(int sid, std::shared_ptr<Event> event);
  static std::shared_ptr<Event> get_event_no_sync(int sid);
};

#endif //_EVENT_POOL_H__

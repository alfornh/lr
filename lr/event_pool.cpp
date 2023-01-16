#include "event_pool.h"

#include "zlog.h"
#include "event.h"
#include "event_queue.h"

static EventQueue _event_queue;

EventPool::EventPool() { }

void EventPool::stop() {
  ZLOG_WARN(__FILE__, __LINE__, __func__);

  _event_queue.release();
}

int EventPool::reserve_event_queue() {
  ZLOG_INFO(__FILE__, __LINE__, __func__);

  return _event_queue.reserve_slot();
}

int EventPool::add_event(int sid, std::shared_ptr<Event> event) {
  return _event_queue.add_event_async(sid, event);
}

std::shared_ptr<Event> EventPool::get_event(int sid) {
  return _event_queue.get_event_sync(sid);
}

int EventPool::add_event_no_sync(int sid, std::shared_ptr<Event> event) {
  return _event_queue.add_event_no_sync(sid, event);
}

std::shared_ptr<Event> EventPool::get_event_no_sync(int sid) {
  return _event_queue.get_event_no_sync(sid);
}


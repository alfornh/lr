#ifndef _MESSAGE_QUEUE_H__
#define _MESSAGE_QUEUE_H__

#include <memory>
#include <mutex>
#include <list>
#include <vector>
#include <condition_variable>

class Event;

class EventSlot {
public:
  typedef std::shared_ptr<EventSlot> ptr;

  EventSlot(){
    _events_number = 0; 
  }

  std::list<std::shared_ptr<Event>> _events;
  int _events_number;
  std::condition_variable _cv_events;
  std::mutex _mutex_events;
};

typedef std::vector<std::shared_ptr<EventSlot>> EventSlots;

class EventQueue {
public:
  typedef std::shared_ptr<EventQueue> ptr;

  EventQueue();

  int init();

  //return sid, or -1 failure
  int reserve_slot();

  int add_event_async(int sid, std::shared_ptr<Event>);

  //sid 0 randomly, or from slot id
  std::shared_ptr<Event> get_event_sync(int sid);

  int add_event_no_sync(int sid, std::shared_ptr<Event>);

  //sid 0 randomly, or from slot id
  std::shared_ptr<Event> get_event_no_sync(int sid);

  void release() {
    _release_flag = true;
    for (EventSlot::ptr &es: _slots) {
      if ( !es ) {
        continue;
      }
      es->_cv_events.notify_all();
    }
  }

private:
  EventSlots _slots;

  int _slot_number;
  int _slot_size;

  bool _release_flag;
};

#endif //_MESSAGE_QUEUE_H__

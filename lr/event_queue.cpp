#include "event_queue.h"

#include <stdlib.h>

#include "event.h"

//#include <errno.h>
//#include <time.h>
#include "zlog.h"

EventQueue::EventQueue() {
  init();
}

int EventQueue::init() {
  _slot_size   = 4;
  _slots.resize(_slot_size);
  _slot_number = 0;
  _release_flag = false;

  return 0;
}

int EventQueue::reserve_slot() {

  if (_slot_number >= _slot_size) {
    _slot_size = _slot_size + _slot_size;
    _slots.resize(_slot_size);
    _slots.shrink_to_fit();
  }

  _slots[_slot_number] = std::make_shared<EventSlot>();
  ++_slot_number;

  return _slot_number;
}

int EventQueue::add_event_async(int sid, std::shared_ptr<Event> event) {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, sid, event->_stype);
  if (_release_flag) {
    return -1;
  }
  {
    std::unique_lock<std::mutex> l(_slots[sid - 1]->_mutex_events);
    _slots[sid - 1]->_events.emplace_back(event);
    ++_slots[sid - 1]->_events_number;
  }
  _slots[sid - 1]->_cv_events.notify_one();

  return 0;
}

std::shared_ptr<Event> EventQueue::get_event_sync(int sid) {
  Event::ptr event = Event::ptr();
  if (_release_flag) {
    return event;
  }

  do {
    std::unique_lock<std::mutex> l(_slots[sid - 1]->_mutex_events);
    _slots[sid - 1]->_cv_events.wait(l, [sid, this]{ 
      return (_slots[sid - 1]->_events.size() > 0) || _release_flag;
    });

    if (_release_flag) {
      return event;
    }

    if (_slots[sid - 1]->_events.empty()) {
      continue;
    }

    event = _slots[sid - 1]->_events.front();
    _slots[sid - 1]->_events.pop_front();
    --_slots[sid - 1]->_events_number;

    break;

  } while ( true );

  ZLOG_DEBUG(__FILE__, __LINE__, __func__, sid, event->_stype);
  return event;
}

int EventQueue::add_event_no_sync(int sid, std::shared_ptr<Event> event) {
  if (_release_flag) {
    return -1;
  }
  _slots[sid - 1]->_events.emplace_back(event);
  ++_slots[sid - 1]->_events_number;
  return 0;
}

std::shared_ptr<Event> EventQueue::get_event_no_sync(int sid) {
  Event::ptr e = Event::ptr();
  //int sindexbeg = rand() % _slot_size;
  //int sindex;

  if (_release_flag) {
    return e;
  }

//  if ( sid ) {
  if (_slots[sid - 1]->_events.size() < 1) {
    return e;
  }

  e = _slots[sid - 1]->_events.front();
  _slots[sid - 1]->_events.pop_front();
  --_slots[sid - 1]->_events_number;
//
//  } else {
//
//    for (int index = 0; index < _slot_size; ++index) {
//      sindex = (sindexbeg + index) % _slot_size;
//
//      if (_slots[sindex]._events.size() < 1) {
//        continue;
//      }
//
//      e = _slots[sindex]._events.front();
//      _slots[sindex]._events.pop_front();
//      --_slots[sindex]._events_number;
//      return e;
//    }
//  }

  return e;
}

//int MessageQueue::_message_key = time(0);
//int MessageQueue::new_message_key() {
//  return _message_key++;
//}

//int MessageQueue::init() {
//  _message_queue_id = msgget(new_message_key(), IPC_CREAT | 0666);
//  if ( _message_queue_id < 0 ) {
//    LOG_ERROR("{}:{} {}", __FILE__, __LINE__, __func__);
//    return -1;
//  }
//
//  return 0;
//}


//int MessageQueue::add_message_async(EventMessage &msg) {
//  if (msgsnd(_message_queue_id, &msg, sizeof(msg._event), IPC_NOWAIT) < 0) {
//    LOG_ERROR("{}:{} {} msgsnd error", __FILE__, __LINE__, __func__);
//    return -1;
//  }
//
//  return 0;
//}

//int MessageQueue::get_message_sync(EventMessage &msg) {
//  do {
//    int ret = msgrcv(_message_queue_id, &msg, sizeof(msg._event), 0, MSG_NOERROR);
//    if (ret < 0) {
//      if (errno != ENOMSG) {
//        LOG_ERROR("{}:{} {} message queue error", __FILE__, __LINE__, __func__);
//        return ret;
//      }
//
//      LOG_INFO("{}:{} {} no message available", __FILE__, __LINE__, __func__);
//      continue;
//    }
//
//    break;
//
//  } while ( true );
//
//  return 0;
//}

#include "thread_manager.h"

#include "base_type.h"
#include "configure.h"
#include "event_type.h"

std::shared_ptr<ThreadManager> ThreadManager::_instance = std::make_shared<ThreadManager>();

ThreadGroup::ThreadGroup() {
}

ThreadGroup::ThreadGroup(EVENTID tg_type) {
  _tg_type = tg_type;
}

int ThreadGroup::start(int thread_num) {
  _stop_flag = false;
  _threads.clear();
  _expect_thread_num = thread_num;

  for (int i = 0; i < _expect_thread_num; ++i) {
    std::shared_ptr<std::thread> th = std::make_shared<std::thread>(std::bind(&ThreadGroup::_thread_proc, this));
    th->detach();

    _threads.emplace_back(th);
  }

  return 0;
}

int ThreadGroup::add_task(const std::shared_ptr<Task> t) {
  {
    std::lock_guard<std::mutex> l(_mutex_tasks);

    if (_stop_flag) {
      ZLOG_WARN(__FILE__, __LINE__, __func__, "thread group stoped", _tg_type);
      return -1;
    }
 
    _tasks.emplace(t);
  }

  _cv_tasks.notify_one();

  return 0;
}

void ThreadGroup::stop() {
  _stop_flag = true;
  _cv_tasks.notify_all();
}

void ThreadGroup::_thread_proc() {
  while ( !_stop_flag ) {
    ZLOG_DEBUG(__FILE__, __LINE__, __func__);
    Task::ptr t;

    {
      std::unique_lock<std::mutex> lock(this->_mutex_tasks);

      _cv_tasks.wait(lock, [this] {
        return this->_stop_flag || !this->_tasks.empty();
      });

      if (_stop_flag) {
        ZLOG_WARN(__FILE__, __LINE__, __func__, "_stop_flag", _stop_flag);
        return ;
      }

      if (_tasks.empty()) {
        continue;
      }

      t = std::move(_tasks.front());
      _tasks.pop();
    }

    (*t)();
  }
}

ThreadManager::ThreadManager() {
  _thread_group_id = 0;
}

int ThreadManager::init() {
  _thread_groups.resize(MAX_THREAD_GROUP_NUM);
  _thread_groups.shrink_to_fit();
 
  return 0;
}

int ThreadManager::reserve_thread_group() {
  _thread_groups[_thread_group_id] = std::make_shared<ThreadGroup>();

  ++_thread_group_id;

  return _thread_group_id;
}


int ThreadManager::start(int thread_group_id, int th_num) {
  return _thread_groups[--thread_group_id]->start(th_num);
}

void ThreadManager::stop() {
  for (int i = 0; i < _thread_group_id; ++i) {
    if (_thread_groups[i]) {
      _thread_groups[i]->stop();
    }
  }

  //for (ThreadGroup::ptr &g: _thread_groups) {
  //  if (g) {
  //    g->stop();
  //  }
  //}
}

ThreadManager::~ThreadManager() {
  //{
  //  std::unique_lock<std::mutex> lock(_mutex_tasks);
  //  _stop_flag = true;
  //}

  //_cv_tasks.notify_all();

  //for (std::thread &t: _threads) {
  //  t.join();
  //}
}

int ThreadManager::run_task(int thread_group_id, std::shared_ptr<Task> t) {
  return _thread_groups[--thread_group_id]->add_task(t);
}


#ifndef _THREAD_MANAGER_H__
#define _THREAD_MANAGER_H__

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <future>
#include <unordered_map>

#include "base_type.h"
#include "utils.h"
#include "zlog.h"
#include "task.h"

#define MAX_THREAD_GROUP_NUM 32
#define DEFAULT_WORKER_THREAD_NUM 10

#define PTHREADMANAGER ThreadManager::_instance
#define RUN_TASK ThreadManager::_instance->run_task


class Task;
class ThreadGroup {
public:
  typedef std::shared_ptr<ThreadGroup> ptr;

  ThreadGroup();
  ThreadGroup(EVENTID tg_type);

  int start(int thread_num);

  int add_task(const std::shared_ptr<Task> t);

  void stop();

public: //private
  void _thread_proc();

  typedef std::vector<std::shared_ptr<std::thread>> ThreadsContainer;
  typedef std::queue<std::shared_ptr<Task> > TaskQueue;

  ThreadsContainer _threads;
  TaskQueue _tasks;
  std::mutex _mutex_tasks;
  std::condition_variable _cv_tasks;
  //std::vector<std::shared_ptr<std::thread>> _threads;
  //std::queue<std::function<void()>> _tasks;
  EVENTID _tg_type;
  bool _stop_flag;

  int _expect_thread_num;
};

class ThreadManager {
public:
  typedef std::shared_ptr<ThreadManager> ptr;

  ThreadManager();
  ~ThreadManager();

  int init();

  int run_task(int thread_group_id, std::shared_ptr<Task> t);

  void stop();

public://private:
  int reserve_thread_group();

  int start(int thread_group_id, int th_num);

private:
  typedef std::vector<std::shared_ptr<ThreadGroup>> ThreadGroupContainer;
  
  ThreadGroupContainer _thread_groups;
  int _thread_group_id;

public:
  static std::shared_ptr<ThreadManager> _instance;
};

#endif //_THREAD_MANAGER_H__

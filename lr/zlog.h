#ifndef _Z_LOG_H__
#define _Z_LOG_H__

//#include <condition_variable>
#include <iostream>
//#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "plt/oth-inc.h"

#define DEFAULT_LOG_PATH "logs"
#define DEFAULT_LOG_FILE_PREFIX "lr"
#define DEFAULT_ROTATE_SIZE 4 * 1024 * 1024

#define PZLOG ZLog::_instance

class File;

class Record {
public:
  typedef std::shared_ptr<Record> ptr;

  Record(int s, int l, std::string m) {
    _sections = s;
    _level = l;
    _msg = m;
    _tid = get_thread_id();
    _time = time(0);
  }

  std::string to_string();

  TIME_T _time;
  int _level;
  int _sections;
  THREADID _tid;
  std::string _msg;
};

class ZLog {
public:
enum log_level {
  debug     = 0x01,
  info      = 0x02,
  warn      = 0x03,
  error     = 0x04,
};

enum log_sink {
  file      = 0x01,
  terminal  = 0x02,
};

enum log_section {
  stime        = 0x01,
  sthread_id   = 0x02,
  slevel       = 0x04,
  smessage     = 0x08,
  sall         = 0xFF,
};

public:
  typedef std::shared_ptr<ZLog> ptr;

  ZLog() {
    _level = log_level::info;
    _sinks = log_sink::terminal;
    _sections = log_section::sall;
    _rotate_size = DEFAULT_ROTATE_SIZE;
  }
  ~ZLog();

  int init();

  void set_log_file_prefix(std::string pre) {
    _prefix = pre;
  }

  void set_log_level(int ll) {
    _level = ll;
  }

  void set_log_sink(int ls) {
    _sinks = ls;
  }

  void set_log_sections(int ls) {
    _sections = ls;
  }

  void set_log_rotate_size(int size) {
    _rotate_size = size;
  }

  void record(const int lev, std::string msg);

  //void stop();

private:
  bool fexist(const std::string f);

  int rotate();

public:
  //void _flush_proc();

public:
  std::shared_ptr<File> _file;

  std::string _path;
  std::string _prefix;
  int _level;
  int _sinks;
  int _sections;
  int _rotate_size;

  //std::list<std::shared_ptr<Record> > _records;
  std::mutex _mutex_records;
  //std::condition_variable _cv_records;

  //bool _stop_flag;

  //std::shared_ptr<std::thread> _thread;
public:
  static std::shared_ptr<ZLog> _instance;
};

#define ZLOG_DEBUG(...) ZLOG(ZLog::log_level::debug, ##__VA_ARGS__)
#define ZLOG_INFO(...)  ZLOG(ZLog::log_level::info,  ##__VA_ARGS__)
#define ZLOG_WARN(...)  ZLOG(ZLog::log_level::warn,  ##__VA_ARGS__)
#define ZLOG_ERROR(...) ZLOG(ZLog::log_level::error, ##__VA_ARGS__)

inline std::string v_to_string(const char * const a) {
  return std::string(a);
}

inline std::string v_to_string(const unsigned char * const a) {
  return std::string((const char *)a);
}

inline std::string v_to_string(const std::string a) {
  return a;
}

inline std::string v_to_string(const int a) {
  return std::to_string(a);
}

inline std::string v_to_string(const unsigned a) {
  return std::to_string(a);
}

inline std::string v_to_string(const long a) {
  return std::to_string(a);
}

inline std::string v_to_string(const unsigned long a) {
  return std::to_string(a);
}

inline std::string v_to_string(const long long a) {
  return std::to_string(a);
}
 
inline std::string v_to_string(const unsigned long long a) {
  return std::to_string(a);
}
 
inline std::string v_to_string(const float a) {
  return std::to_string(a);
}
 
inline std::string v_to_string(const double a) {
  return std::to_string(a);
}

template<typename A>
void ZLOG(const int lev, const A a) {
  std::string msg = v_to_string(a);

  PZLOG->record(lev, msg);
}

template<typename A, typename B>
void ZLOG(const int lev, const A a, const B b) {
  std::string msg = v_to_string(a) + " ";
  msg += v_to_string(b);

  PZLOG->record(lev, msg);
}

template<typename A, typename B, typename C>
void ZLOG(const int lev, const A a, const B b, const C c) {
  std::string msg = v_to_string(a) + " ";
  msg += v_to_string(b) + " ";
  msg += v_to_string(c);

  PZLOG->record(lev, msg);
}

template<typename A, typename B, typename C, typename D>
void ZLOG(const int lev, const A a, const B b, const C c, const D d) {
  std::string msg = v_to_string(a) + " ";
  msg += v_to_string(b) + " ";
  msg += v_to_string(c) + " ";
  msg += v_to_string(d);

  PZLOG->record(lev, msg);
}

template<typename A, typename B, typename C, typename D, typename E>
void ZLOG(const int lev, const A a, const B b, const C c, const D d, const E e) {
  std::string msg = v_to_string(a) + " ";
  msg += v_to_string(b) + " ";
  msg += v_to_string(c) + " ";
  msg += v_to_string(d) + " ";
  msg += v_to_string(e);

  PZLOG->record(lev, msg);
}

template<typename A, typename B, typename C, typename D, typename E, typename F>
void ZLOG(const int lev, const A a, const B b, const C c, const D d, const E e, const F f) {
  std::string msg = v_to_string(a) + " ";
  msg += v_to_string(b) + " ";
  msg += v_to_string(c) + " ";
  msg += v_to_string(d) + " ";
  msg += v_to_string(e) + " ";
  msg += v_to_string(f);

  PZLOG->record(lev, msg);
}

template<typename A, typename B, typename C, typename D, typename E, typename F, typename G>
void ZLOG(const int lev, const A a, const B b, const C c, const D d, const E e, const F f, const G g) {
  std::string msg = v_to_string(a) + " ";
  msg += v_to_string(b) + " ";
  msg += v_to_string(c) + " ";
  msg += v_to_string(d) + " ";
  msg += v_to_string(e) + " ";
  msg += v_to_string(f) + " ";
  msg += v_to_string(g);

  PZLOG->record(lev, msg);
}

#endif //_Z_LOG_H__

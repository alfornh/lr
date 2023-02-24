#ifndef _CONFIGURE_H__
#define _CONFIGURE_H__

#include <memory>
#include <string>
#include <unordered_map>

#include "data_buffer.h"
#include "configure_item.h"

typedef std::string Key;

#define PCONFIGURE Configure::_instance

class Value {
public:
  typedef std::shared_ptr<Value> ptr;

  Value();
  Value(const char *);

  bool valid();
  int _int();
  double _double();
  std::string _string();

public:
  std::string _v;
};

class File;
class Configure {
public:
  typedef std::shared_ptr<Configure> ptr;

  int init();

  std::shared_ptr<Value> get_value(const Key &key);

  int get_int(const Key &key, int *val);
  int get_double(const Key &key, double *val);
  int get_string(const Key &key, std::string &val);

  int get_log_level();

  std::shared_ptr<ConfigureItem> get_ci(int);

  bool line_enable(int);
  int get_max_thread_number(int);

private:
  int parse_configure();


  std::shared_ptr<ConfigureItem> parse_lr_tu(const int ciid);
  void parse_left_end();
  void parse_right_end();

  std::shared_ptr<ConfigureItem> parse_server_info();
  std::shared_ptr<ConfigureItem> parse_timer_line();


private:
  int get_ipi_from_str(const char *str, IPInfo &ipi);

public:
  bool is_key_equal_value(const std::string& key, const std::string& value);

private:
  typedef std::unordered_map<Key, Value::ptr> ConfigureContainer;
  ConfigureContainer _configures;

  typedef std::unordered_map<int, std::shared_ptr<ConfigureItem>> ConfigureItemContainer;
  ConfigureItemContainer _configure_items;

public:
  bool _tcp_accept_event_flag;

  std::shared_ptr<File> _file;

public:
  static std::shared_ptr<Configure> _instance;
};

#endif //_CONFIGURE_H__

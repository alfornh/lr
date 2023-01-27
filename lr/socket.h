#ifndef _SOCKET_H__
#define _SOCKET_H__

#include <memory>
#include <string>
#include <mutex>

#include "event.h"
#include "utils.h"
#include "data_buffer.h"
#include "ipinfo.h"
#include "base_type.h"


#define CLOSE_FLAG true

#define SEND_TO(es, data, args...)        \
  std::static_pointer_cast<Socket>(es)->esend(data, ##args)

enum {
  SOCKET_STATUS_NULL,
  SOCKET_STATUS_INIT,
  SOCKET_STATUS_ACTIVE,
  SOCKET_STATUS_WEBSOCKET_ACTIVE,
  SOCKET_STATUS_CLOSE,
};

class Listener;
class Socket: public ESource {
public:
  typedef std::shared_ptr<Socket> ptr;

  Socket();

  virtual ~Socket();

  virtual int vinit(IPInfo &);
  virtual int vbind();
  virtual std::shared_ptr<Socket> vaccept();
  virtual int vconnect();
  virtual int vlisten();
  virtual int vrecv();
  virtual int vsend(const char *buf, const int blen);
  virtual int vclose();

public:
  int ssend();

public:
  int esend(int cflag = false);
  int esend(const char *buf, const int len, int cflag = false);
  int esend(const std::string &str, int cflag = false);
  int esend(const DataBuffer &db, int cflag = false);
  int esend(const std::shared_ptr<DataBuffer> &db, int cflag = false);
  int esend(const std::shared_ptr<BufferItem> &bi, int cflag = false);

public:
  int add_r_data(const std::shared_ptr<BufferItem> &);
  int add_r_data(const char *buf, const int len);

  int get_r_data(char *buf, const int len);
  int get_r_data(DataBuffer &db);
  int get_r_data(std::shared_ptr<DataBuffer> &db);
  int move_r_data(std::vector<char> &);
  int move_r_data(char *buf, const int len);
  int move_r_data(DataBuffer &db);
  int move_r_data(std::shared_ptr<DataBuffer> &db);
  void clear_r_data();

  int add_w_data(const char *buf, const int len);
  int add_w_data(const std::string &str);
  int add_w_data(const DataBuffer &db);
  int add_w_data(const std::shared_ptr<DataBuffer> &db);
  int add_w_data(const std::shared_ptr<BufferItem> &bi);

  int get_w_data(char *buf, const int len);
  int get_w_data(DataBuffer &db);
  int get_w_data(std::shared_ptr<DataBuffer> &db);
  int move_w_data(char *buf, const int len);
  int move_w_data(std::vector<char> &);
  void clear_w_data();

public:
  int nonblock(bool);
  int reuseaddr(bool);
  static SOCKETID sign_socket_id();

public:
  SOCKETID _id;

  int _fd;
  std::mutex _mutex_a_fd;
  std::mutex _mutex_r_fd;
  std::mutex _mutex_w_fd;

  IPInfo _ipi;
  int _socket_status;

  int _time_stamp;

  std::shared_ptr<DataBuffer> _r_db;
  std::mutex _mutex_r_db;

  std::shared_ptr<DataBuffer> _w_db;
  std::mutex _mutex_w_db;

  //std::list<std::shared_ptr<Event>> _w_events;
  std::mutex _mutex_for_worker_threads;

  std::shared_ptr<Listener> _line;

};

#endif//_SOCKET_H__

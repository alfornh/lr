#ifndef _NET_INC_H__
#define _NET_INC_H__

//#ifdef _WIN32

#include <winSock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <ws2tcpip.h>
#include <mswsock.h>

#include <map>
#include <mutex>

#include "reactor.h"
#include "utils.h"

class MULTIPLEX_CONTEXT {
public:
  enum {
    IO_TYPE_NULL,
    IO_TYPE_READ,
    IO_TYPE_WRITE,
  };
  typedef std::shared_ptr<MULTIPLEX_CONTEXT> ptr;

  MULTIPLEX_CONTEXT(SOCKETID id, int fd);

  SOCKETID _id;
  int      _fd;
  int      _io_type;
  WSAOVERLAPPED  _overlapped;
  std::shared_ptr<BufferItem> _bi;
};

class AsyncIOMultiplex: public Reactor {
public:
  AsyncIOMultiplex(int protocol) {
    _name = Reactor::REACTOR_ASYNC;
    _protocol = protocol;
  }

  virtual ~AsyncIOMultiplex();

  virtual int _init(std::shared_ptr<IPInfo> );
  virtual int _listen();

  virtual int _add(SOCKETID sid, int fd);
  virtual int _del(SOCKETID sid, int fd);
  virtual int _mod(SOCKETID sid, int fd);
  int _mod(MULTIPLEX_CONTEXT::ptr mc);
  virtual int notify_w_event(SOCKETID sid, int fd);

  virtual void _stop();

public:
  virtual void listen_i_proc();
  virtual void listen_o_proc();

public:
  HANDLE _handler;
  std::mutex _mutex_handler;
  typedef std::map<int, std::shared_ptr<MULTIPLEX_CONTEXT> > MULTIPLEX_CONTEXT_CONTAINER;
  MULTIPLEX_CONTEXT_CONTAINER _contexts;
  std::mutex _mutex_contexts;
};

extern "C" {

struct socket_create_param {
  int protocol;
  bool overlapped; // true, false
};
int socket_create(void *param);

struct socket_bind_param {
  int protocol;
  unsigned int port;
  char ip[64];
};
int socket_bind(int socket, void *param);

struct socket_connect_param {
  int protocol;
  unsigned int port;
  char ip[64];
};
int socket_connect(int socket, void *param);

struct socket_accept_param {
  char ip[64];
  unsigned int port;
  int socket;
};
int socket_accept(int socket, void *param);

struct socket_listen_param {
  int number;
};
int socket_listen(int socket, void *param);

struct socket_recv_param {
  int protocol; 
  WSAOVERLAPPED *poverlapped;
  char *buf;
  unsigned int blen;
  unsigned int port;
  char ip[64];
};
int socket_recv(int socket, void *param);

struct socket_send_param {
  int protocol;
  WSAOVERLAPPED *poverlapped;
  char *buf;
  unsigned int blen;
  int socket;
  unsigned int port;
  char ip[64];
};
int socket_send(int socket, void *param);

int socket_nonblock(int socket, bool nb);

int socket_reuse(int socket, bool ru);

int socket_close(int socket);

}

//#endif//_WIN32

#endif//_NET_INC_H__

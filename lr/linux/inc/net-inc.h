#ifndef _NET_INC_H__
#define _NET_INC_H__

#include "reactor.h"

class AsyncIOMultiplex : public Reactor {
public:
  AsyncIOMultiplex(int protocol) {
    _protocol = protocol;
    _name = Reactor::REACTOR_ASYNC;
  }
  virtual ~AsyncIOMultiplex();

  virtual int _init(std::shared_ptr<IPInfo> );
  virtual int _listen();

  virtual int _add(SOCKETID sid, int fd);
  virtual int _del(SOCKETID sid, int fd);
  virtual int _mod(SOCKETID sid, int fd);

  virtual int notify_w_event(SOCKETID sid, int fd);

  virtual void _stop();

public:
  virtual void listen_i_proc();
  virtual void listen_o_proc();

public:
  int _epollfd;
  std::mutex _mutex_epollfd;
};

extern "C" {

struct socket_create_param {
  int protocol;
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
  char *buf;
  unsigned int blen;
  unsigned int port;
  char ip[64];
};
int socket_recv(int socket, void *param);

struct socket_send_param {
  int protocol;

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

#endif//_NET_INC_H__

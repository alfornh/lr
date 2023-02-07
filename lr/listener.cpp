#include "listener.h"

//#include <sys/socket.h>
#include <sys/types.h>

#include "event_type.h"
#include "socket.h"
#include "reactor.h"
#include "zlog.h"

//bool Listener::online() {
//  if (!_main_socket) {
//    return false;
//  }
//
//  if (_main_socket->_socket_status == SOCKET_STATUS_ACTIVE || _main_socket->_socket_status == SOCKET_STATUS_WEBSOCKET_ACTIVE) {
//    return true;
//  }
//
//  if (_main_socket->_socket_status != SOCKET_STATUS_CLOSE) {
//    int err;
//    socklen_t errlen = sizeof(err);
//    
//    int ret = getsockopt(_main_socket->_fd, SOL_SOCKET, SO_ERROR, &err, &errlen); 
//    if (ret < 0) {
//      return false;
//    }
//
//    if (err == 0) {
//      _main_socket->_socket_status = SOCKET_STATUS_ACTIVE;
//      return true;
//    }
//  }
//  return false;
//}
//
//int Listener::reconnect_main_socket() {
//  int ret;
//
//  switch (_main_socket->_stype) {
//  case EVENT_TYPE_SOCKET_TCP:
//    ret = _main_socket->vconnect();
//  break;
//
//  case EVENT_TYPE_SOCKET_UDP:
//    ret = _main_socket->vbind();
//  break;
//  }
//
//  if (ret < 0) {
//    ZLOG_ERROR(__FILE__, __LINE__, __func__, "bind/connect");
//    return -1;
//  }
//
//  _main_socket->_socket_status = SOCKET_STATUS_ACTIVE;
//  //_main_socket->vinit(__ipi);
//  //if (_reactor) {
//  _reactor->_add(_main_socket->_id, _main_socket->_fd);
//  //}
//
//  return ret;
//}

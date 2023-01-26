#include "left_websocket_handler.h"

#include <stdio.h>

#include "event.h"
#include "event_pool.h"
#include "listener.h"
#include "socket.h"
#include "utils.h"
#include "websocketpackage.h"
#include "zlog.h"

int LeftWebSocketHandler::handle(EVENTID id, std::shared_ptr<Event> ev) {
  switch (id & EVENT_SUB_TYPE_MASK) {
  case EVENT_SUBTYPE_READ:
    return echo(STATIC_CAST(Socket, ev->_es));
  break;

  case EVENT_SUBTYPE_CLOSE:
    ZLOG_INFO(__FILE__, __LINE__, __func__, "EVENT_SUBTYPE_CLOSE");
  break;

  default:
    ZLOG_WARN(__FILE__, __LINE__, __func__, "unknown id", id);
    return -1;
  }

  return 0;
}

int LeftWebSocketHandler::echo(std::shared_ptr<Socket> sock) {
  if (sock->_socket_status == SOCKET_STATUS_ACTIVE) {
    int ret = _handshak(sock);
    if ( ret >= 0 ) {
      sock->_socket_status = SOCKET_STATUS_WEBSOCKET_ACTIVE;
    }
    return ret;
  }

  if (sock->_socket_status != SOCKET_STATUS_WEBSOCKET_ACTIVE) {
    ZLOG_WARN(__FILE__, __LINE__, __func__, "unknown socket status", sock->_socket_status);
    return -1;
  }

  DataBuffer::ptr db = MAKE_SHARED(DataBuffer);
  sock->move_r_data(db);
  int stype = 0;
  int ret = _decode_data(db, stype);
  if ( ret < 0 ) {
    return ret;
  }

  char head[32] = {0};
  db->move(head, sizeof(head) - 1);
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, "head", head);
  char echo[128];
  sprintf(echo, "%s %d %d:%s", sock->_line->__ipi._ip.c_str(), sock->_line->__ipi._port, sock->_id, head);
  db->set(echo, strlen(echo));

  _encode_data(db);

  return SEND_TO(sock, db);
}


int LeftWebSocketHandler::_handshak(std::shared_ptr<Socket> &socket) {
  char method[16] = {0};
  std::vector<char> data;
  //Socket::ptr s = STATIC_CAST(Socket, ev->_es);
  socket->move_r_data(data);
  char *beg = &(*data.begin());

  while ( *beg == ' ' ) {
    ++beg;
  }

  int i = 0;
  while ( i < 15 && *beg  != ' ' ) {
    method[i++] = *beg++;
  }

  if (strncmp(method, "GET", 3) != 0) {
    ZLOG_ERROR("{}:{} method {} not GET", __FILE__, __LINE__, method);
    return -1;
  }

  char version[32] = {0};
  int ver_i = 0;
  char *pos = strstr(beg, "HTTP/");
  while ( ver_i < 32 && *pos != '\r' && *pos != '\n') {
    version[ver_i++] = *pos++;
  }

  pos = strstr(pos, "Sec-WebSocket-Key");
  if (!pos) {
    ZLOG_ERROR("{}:{} no websocket key found", __FILE__, __LINE__);
    return -1;
  }
  pos = strchr(pos, ':');
  while (*pos == ':' || *pos == ' ')
    ++pos;

  char key[128] = {0};
  int  key_i = 0;
  while (key_i < 64 && *pos != ' ' && *pos != '\r' && *pos != '\n') {
    key[key_i++] = *pos++;
  }

  strcat(key + key_i, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

  ZLOG_DEBUG("{}:{} sec websocket key [{}]", __FILE__, __LINE__, key);

  char sha1key[128] = { 0 };
  SECKEY_SHA1((const char *)key, strlen(key), (char *)sha1key);
  //int pret = SECKEY_SHA1((const char *)key, strlen(key), (char *)sha1key);
  //if ( pret < 0 ) {
  //  ZLOG_ERROR(__FILE__, __LINE__, "zsha1 failed");
  //  return -1;
  //}

  ZLOG_DEBUG(__FILE__, __LINE__, "sha1 ed sec websocket key", sha1key);

  char basekey[128] = {0};
  ZBASE64(sha1key, 20, basekey);
  ZLOG_DEBUG(__FILE__, __LINE__, __func__, "base64 ed sec websocket key", basekey);

  std::string resp = std::string(version) + " 101 Switching Protocals\r\n";
  resp += "Upgrade: websocket\r\n";
  resp += "Connection: Upgrade\r\n";
  resp += "Sec-WebSocket-Accept: " + std::string(basekey) + "\r\n\r\n";

  //ZLOG_DEBUG("{}:{} send:{}", __FILE__, __LINE__, resp);

  return SEND_TO(socket, resp);
}


int LeftWebSocketHandler::_decode_data(std::shared_ptr<DataBuffer> &db, int &stype) {
  WebsocketPackage _r_socket_package;

  _r_socket_package.from(db);
  _r_socket_package.cache();
  if (!_r_socket_package.is_msg_tail()) {
    return 0;
  }

  db->clear();
  _r_socket_package.get_cache_data(db);
  _r_socket_package.clear();
  
  switch (_r_socket_package._fin_rsv1_rsv2_rsv3_opcode & WebsocketPackage::MASK_OPCODE) {
  case PACKAGE_ADDITIONAL_DATA:
    stype = EVENT_TYPE_WEBSOCKET_ADDITIONAL_DATA;
  break;

  case PACKAGE_TEXT_DATA:
    stype = EVENT_TYPE_WEBSOCKET_TEXT_DATA;
  break;

  case PACKAGE_BINARY_DATA:
    stype = EVENT_TYPE_WEBSOCKET_BINARY_DATA;
  break;

  case PACKAGE_CONNECTION_CLOSE:
    ZLOG_INFO(__FILE__, __LINE__, __func__, "peer closed");
    stype = PACKAGE_CONNECTION_CLOSE;
  break;

  case PACKAGE_PING:
    stype = EVENT_TYPE_WEBSOCKET_PING;
  break;

  case PACKAGE_PONG:
    stype = EVENT_TYPE_WEBSOCKET_PONG;
    return 0;
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown op code");
    return -1;
  }

  return 0;
}

int LeftWebSocketHandler::_encode_data(std::shared_ptr<DataBuffer> &db) {
  WebsocketPackage _w_socket_package;
  _w_socket_package.clear();

  _w_socket_package.set_payload_data(db);
  _w_socket_package.cache();

  //DataBuffer dt;
  _w_socket_package.to(db);

  return 0;
}

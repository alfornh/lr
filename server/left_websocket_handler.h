#ifndef _LEFT_WEB_SOCKET_HANDLER_H__
#define _LEFT_WEB_SOCKET_HANDLER_H__

#include "handler.h"
#include "event_type.h"

class Event;
class Socket;
class DataBuffer;
class LeftWebSocketHandler: public Handler {
public:
  LeftWebSocketHandler() { _stype = EVENT_TYPE_WEBSOCKET; }
  virtual ~LeftWebSocketHandler() {}
  virtual int handle(EVENTID id, std::shared_ptr<Event>);

private:

  int _handshak(std::shared_ptr<Socket> &);
  int _decode_data(std::shared_ptr<DataBuffer> &db, int &stype);
  int _encode_data(std::shared_ptr<DataBuffer> &db);


  int echo(std::shared_ptr<Socket>);

};

#endif //_WEB_SOCKET_HANDLER_H__

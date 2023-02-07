#ifndef _FROM_AND_TO_H__
#define _FROM_AND_TO_H__

#include <unordered_map>
#include <mutex>
#include <memory>

#include "plt/type-inc.h"

#include "ip_addr.h"
#include "socket.h"
#include "zlog.h"

#define PFROMANDTO FromAndTo::_instance

class FromAndTo {
public:

  std::shared_ptr<Socket> make_right_by_left(std::shared_ptr<Socket> left);
  std::shared_ptr<Socket> GET_RIGHT_BY_LEFTID(SOCKETID __sid__);
  std::shared_ptr<Socket> GET_LEFT_BY_RIGHTID(SOCKETID __cid__);
  void SET_LEFT_AND_RIGHT(Socket::ptr __left__, Socket::ptr __right__);
  void DEL_ENTRY_BY_RIGHTID(SOCKETID __cid__);
  void DEL_ENTRY_BY_LEFTID(SOCKETID __sid__);
  
  std::shared_ptr<Socket> get_right_by_left_ip(IPAddress ip);
  std::shared_ptr<Socket> get_left_by_right_id(SOCKETID id);
  void set_left_ip_and_right_id(Socket::ptr left, Socket::ptr right);
  void del_entry_by_left_ip(IPAddress ip);
  void del_entry_by_right_id(SOCKETID id);
  void update_left_ip_and_right_id(Socket::ptr left, Socket::ptr right);


  std::shared_ptr<Socket> make_tcp_right(std::shared_ptr<Socket> left);
  std::shared_ptr<Socket> make_udp_right(std::shared_ptr<Socket> left);
public:

  typedef std::unordered_map<SOCKETID, std::weak_ptr<Socket>> RIGHT_LEFT_CONTAINER;
  typedef std::unordered_map<IPAddress, std::weak_ptr<Socket>, HashIPAddress, EqualIPAddress > IP_LEFT_RIGHT_CONTAINER;
  typedef std::unordered_map<SOCKETID, std::shared_ptr<Socket> > ID_RIGHT_LEFT_CONTAINER;

  RIGHT_LEFT_CONTAINER _left_by_right_index;
  RIGHT_LEFT_CONTAINER _right_by_left_index;

  std::mutex _mutex_left_and_right_idx;


  IP_LEFT_RIGHT_CONTAINER _right_by_left_ip_index;
  ID_RIGHT_LEFT_CONTAINER _left_by_right_id_index;
  std::mutex _mutex_left_ip_and_right_id;

public:
  static std::shared_ptr<FromAndTo> _instance;
};

#endif //_FROM_AND_TO_H__


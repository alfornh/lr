#ifndef _IP_ADDRESS_H__
#define _IP_ADDRESS_H__

#include <string>

struct IPAddress {
  IPAddress():_ip(""), _port(-1){}
  IPAddress(const std::string ip, const int port):_ip(ip), _port(port) { }

  std::string _ip;
  int _port;
};

struct HashIPAddress {
  std::size_t operator()(const struct IPAddress& k) const {
    return std::hash<std::string>()(k._ip) ^
          (std::hash<int>()(k._port) << 1);
  }

};

struct EqualIPAddress {
  bool operator() (const struct IPAddress& lhs, const struct IPAddress& rhs) const {
    return lhs._ip == rhs._ip && lhs._port == rhs._port;
  }
};

#endif//_IP_ADDRESS_H__

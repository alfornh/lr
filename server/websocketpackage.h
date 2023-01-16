#ifndef _WEB_SOCKET_PACKAGE_H__
#define _WEB_SOCKET_PACKAGE_H__

#include <stdint.h>

#include <memory>
#include <vector>

#include "data_buffer.h"

enum {
  PACKAGE_ADDITIONAL_DATA    = 0x0,
  PACKAGE_TEXT_DATA          = 0x1,
  PACKAGE_BINARY_DATA        = 0x2,
  PACKAGE_CONNECTION_CLOSE   = 0x8,
  PACKAGE_PING               = 0x9,
  PACKAGE_PONG               = 0xA,
};

class WebsocketPackage {
public:

  typedef std::shared_ptr<WebsocketPackage> ptr;

  WebsocketPackage();

  static int get_websocketpackage_len(char const *, int);

  int from(const char *data, size_t len);
  int from(std::shared_ptr<DataBuffer>);

  void cache();
  int get_cache_data(std::vector<char>&);
  int get_cache_data(DataBuffer&);
  int get_cache_data(DataBuffer::ptr);
  bool is_msg_tail();

  int  get_package_type();

  int set_payload_data(const char *buf, const int len);
  int set_payload_data(std::vector<char>&);
  int set_payload_data(const std::string&);
  int set_payload_data(DataBuffer&);
  int set_payload_data(DataBuffer::ptr);

  int to(DataBuffer&);
  int to(DataBuffer::ptr);

  void clear();

  enum {
    MASK_FIN         = 0b10000000,
    MASK_RSV1        = 0b01000000,
    MASK_RSV2        = 0b00100000,
    MASK_RSV3        = 0b00010000,
    MASK_OPCODE      = 0b00001111,

    MASK_MASK        = 0b10000000,
    MASK_PAYLOAD_LEN = 0b01111111,
  };

  uint8_t           _fin_rsv1_rsv2_rsv3_opcode;
  uint8_t           _mask_payloadlen;
  uint8_t           _maskinfkey[4];
  uint64_t          _payloadlen; //can be unit16_t, need ntohs

  std::shared_ptr<DataBuffer>     _db_payload;
  std::shared_ptr<DataBuffer>     _db_cache;
};

#endif //_WEB_SOCKET_PACKAGE_H__

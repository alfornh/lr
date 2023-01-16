#include "websocketpackage.h"

#include "zlog.h"
#include "utils.h"
#include "event_type.h"

WebsocketPackage::WebsocketPackage() {
  _fin_rsv1_rsv2_rsv3_opcode = 0;
  _mask_payloadlen           = 0;
  _maskinfkey[4]             = { 0 };
  _payloadlen                = 0;
  _db_payload                = std::make_shared<DataBuffer>();
  _db_cache                  = std::make_shared<DataBuffer>();
}


int WebsocketPackage::get_websocketpackage_len(char const *header, int len) {
  if ( len < EVENT_HEAD_LEN ) {
    ZLOG_INFO(__FILE__, __LINE__, __func__, len, "len less than EVENT_HEAD_LEN");
    return -1;
  }

  uint8_t mask_payloadlen = header[1];
  int ret = mask_payloadlen & MASK_PAYLOAD_LEN;
  if ( ret <= 125 ) {
    return ret;
  }

  if ( ret != 126 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "data package too large, not support");
    return -1;
  }


  const uint16_t *pll = (const uint16_t *)(&(header[2]));

  ret = ntoh16(*pll);
  return ret;
}

int WebsocketPackage::from(std::shared_ptr<DataBuffer> dbuf) {
  BufferItem::ptr bi = dbuf->_data.front();
  if ( !bi ) {
    ZLOG_INFO(__FILE__, __LINE__, __func__, "no data available");
    return 0;
  }

  dbuf->_data.pop_front();
  dbuf->_len -= bi->_len;

  const char *data = bi->_buffer;;
  int len = bi->_len;
  if ( !data  || len < 1 ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, data, len);
    return -1;
  }

  const char *data_beg = NULL;
  int lenused = 0;
  if (lenused + 1 > len) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "imcompelete package");
    return -1;
  }
  _fin_rsv1_rsv2_rsv3_opcode = data[lenused];
  ++lenused;

  if (lenused + 1> len) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "imcompelete package");
    return -1;
  }
  _mask_payloadlen           = data[lenused];
  ++lenused;

  _payloadlen = (_mask_payloadlen & MASK_PAYLOAD_LEN);

  if (_payloadlen <= 125) {
    if ((MASK_MASK & _mask_payloadlen) != MASK_MASK) {
      if (lenused + 1 > len) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__);
        return -1;
      }

      data_beg = data + lenused;
      ++lenused;

    } else {

      if (lenused + 4 > len) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "imcompelete package");
        return -1;
      }

      _maskinfkey[0] = *(data + lenused);
      ++lenused;
      _maskinfkey[1] = *(data + lenused);
      ++lenused;
      _maskinfkey[2] = *(data + lenused);
      ++lenused;
      _maskinfkey[3] = *(data + lenused);
      ++lenused;

      if (lenused + 1 > len) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "imcompelete package");
        return -1;
      }

      data_beg = data + lenused;
      ++lenused;
    }
  } else if ( _payloadlen == 126 ) {
    if (lenused + 2 > len) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "imcompelete package");
      return -1;
    }

    const uint16_t *pll = (const uint16_t *)(data + lenused);

    _payloadlen = ntoh16(*pll);

    lenused += 2;

    if ((int)(MASK_MASK & _mask_payloadlen) == 0) {
      if (lenused + 1 > len) {
        ZLOG_ERROR("{}:{} imcompelete package", __FILE__, __LINE__);
        return -1;
      }
      data_beg = data + lenused;
      ++lenused;

    } else {
      if (lenused + 4 > len) {
        ZLOG_ERROR("{}:{} imcompelete package", __FILE__, __LINE__);
        return -1;
      }

      _maskinfkey[0] = *(data + lenused);
      ++lenused;
      _maskinfkey[1] = *(data + lenused);
      ++lenused;
      _maskinfkey[2] = *(data + lenused);
      ++lenused;
      _maskinfkey[3] = *(data + lenused);
      ++lenused;

      if (lenused + 1 > len) {
        ZLOG_ERROR("{}:{} imcompelete package", __FILE__, __LINE__);
        return -1;
      }

      data_beg = data + lenused;
      ++lenused;
    }
   
  } else if ( _payloadlen == 127 ) {
    if (lenused + 8 > len) {
      ZLOG_ERROR("{}:{} imcompelete package", __FILE__, __LINE__);
      return -1;
    }

    const uint64_t *pu64 = (const uint64_t *)(data + lenused);

    _payloadlen = ntoh64(*pu64);

    lenused += 8;

    if ((int)(MASK_MASK & _mask_payloadlen) == 0) {
      if (lenused + 1 > len) {
        ZLOG_ERROR("{}:{} imcompelete package", __FILE__, __LINE__);
        return -1;
      }
      data_beg = data + lenused;
      ++lenused;

    } else {
      if (lenused + 4 > len) {
        ZLOG_ERROR("{}:{} imcompelete package", __FILE__, __LINE__);
        return -1;
      }

      _maskinfkey[0] = *(data + lenused);
      ++lenused;
      _maskinfkey[1] = *(data + lenused);
      ++lenused;
      _maskinfkey[2] = *(data + lenused);
      ++lenused;
      _maskinfkey[3] = *(data + lenused);
      ++lenused;

      if (lenused + 1 > len) {
        ZLOG_ERROR("{}:{} imcompelete package", __FILE__, __LINE__);
        return -1;
      }

      data_beg = data + lenused;
      ++lenused;
    }
  } else {
    ZLOG_ERROR("{}:{} _mask_payloadlen error", __FILE__, __LINE__);
    return -1;
  }

  --lenused;

  //LOG_DEBUG("{}:{} data len {}, payloadlen {}, lenused {}", __FILE__, __LINE__, len, _payloadlen, lenused);

  //if (_payloadlen + lenused != len) {
  //  LOG_ERROR("{}:{} data len {}, payload len {}, lenused {}", __FILE__, __LINE__, len, _payloadlen, lenused);
  //  return -1;
  //}

  _db_payload->clear();

  if (bi->_len > lenused) {
    BufferItem::ptr nbi = std::make_shared<BufferItem>();
    memcpy(nbi->_buffer, data_beg, len - lenused);
    nbi->_len = len - lenused;
    _db_payload->add(nbi);
  }

  _db_payload->add(dbuf);

  if ((MASK_MASK & _mask_payloadlen) == MASK_MASK) {
    for (BufferItem::ptr &tbi: _db_payload->_data) {
      char *d = tbi->_buffer;
      for (int i = 0; i < tbi->_len; ++i) {
        d[i] = d[i] ^ _maskinfkey[i % 4];
      }
      //LOG_DEBUG("{}:{} payloaddata:{}", __FILE__, __LINE__, d);
    }
  }

  //LOG_DEBUG("{}:{} after decode payloaddata len {}", __FILE__, __LINE__, _db_payload->_len);

  return 0;

}

int WebsocketPackage::to(DataBuffer &buf) {
  if (_db_cache->size() < 1) {
    ZLOG_INFO(__FILE__, __LINE__, __func__, "no data in data buffer cache");
    return 0;
  }

  char head[32] = { 0 };
  _fin_rsv1_rsv2_rsv3_opcode = (MASK_FIN | PACKAGE_TEXT_DATA);
  head[0] = _fin_rsv1_rsv2_rsv3_opcode;
  int len = 0;
  size_t s = _db_cache->size();

  if (s < 126) {
    _mask_payloadlen = s;
    head[1] = _mask_payloadlen;
    len = 2;
  } else if (s >= 126 && s < 65535) {
    head[1] = 126;
    uint16_t s16 = hton16(s);
    memcpy(&(head[2]), &s16, 2);
    len = 4;
  } else {
    head[1] = 127;
    uint64_t s64 = hton64(s);
    memcpy(&(head[2]), &s64, 8);
    len = 10;
  }

  buf.set(head, len);

  buf.add(_db_cache);

  return s;
}

int WebsocketPackage::to(DataBuffer::ptr db) {
  return to(*db);
  /*
  if (_db_cache->size() < 1) {
    LOG_INFO("{}:{} no data in data buffer cache", __FILE__, __LINE__);
    return 0;
  }

  char head[32] = { 0 };
  _fin_rsv1_rsv2_rsv3_opcode = (MASK_FIN | PACKAGE_TEXT_DATA);
  head[0] = _fin_rsv1_rsv2_rsv3_opcode;

  size_t s = _db_cache->size();

  if (s < 126) {
    _mask_payloadlen = s;
    head[1] = _mask_payloadlen;
  } else if (s >= 126 && s < 65535) {
    head[1] = 126;
    uint16_t s16 = hton16(s);
    memcpy(&(head[2]), &s16, 2);
  } else {
    head[1] = 127;
    uint64_t s64 = hton64(s);
    memcpy(&(head[2]), &s64, 8);
  }

  db->set(head, strlen(head));

  db->add(_db_cache);

  return s;
  */
}


void WebsocketPackage::clear() {
  _db_cache->clear();
  _db_payload->clear();
}

bool WebsocketPackage::is_msg_tail() {
  return (MASK_FIN & _fin_rsv1_rsv2_rsv3_opcode) == MASK_FIN;
}

int  WebsocketPackage::get_package_type() {
  return (MASK_OPCODE & _fin_rsv1_rsv2_rsv3_opcode);
}

int WebsocketPackage::get_cache_data(std::vector<char>& buf) {
  return _db_cache->get(buf);
}

int WebsocketPackage::get_cache_data(DataBuffer& dbuf) {
  return _db_cache->get(dbuf);
}

int WebsocketPackage::get_cache_data(DataBuffer::ptr db) {
 return _db_cache->get(db);
}

int WebsocketPackage::set_payload_data(const char *buf, const int len) {
  _db_payload->set(buf, len);
  return 0;
}

int WebsocketPackage::set_payload_data(std::vector<char> &data) {
  _db_payload->set(data);
  return 0;
}

int WebsocketPackage::set_payload_data(const std::string& data) {
  _db_payload->set(data);
  return 0;
}

int WebsocketPackage::set_payload_data(DataBuffer& db) {
  _db_payload->set(db);
  return 0;
}

int WebsocketPackage::set_payload_data(DataBuffer::ptr db) {
  _db_payload->set(db);
  return 0;
}

void WebsocketPackage::cache() {
  _db_cache->add(_db_payload);
}


#include "data_buffer.h"

#include <memory.h>

#include "zlog.h"

DataBuffer::DataBuffer() {
  //_data = std::vector<char>();
  //_data.resize(data_buffer_capacity_step, '\0');
  //_data.shrink_to_fit();
  _len    = 0;
  _data.clear();
}

void DataBuffer::clear() {
  //_data.resize(data_buffer_capacity_step, '\0');
  //_data.shrink_to_fit();
  //_data = std::vector<char>();
  //_len   = 0;
  _data.clear();
  _len = 0;
}

int DataBuffer::size() {
  return _len;
}

int DataBuffer::drop(const int len) {
  int dlen = 0;
  while (dlen < len) {
    BufferItem::ptr bi = _data.front();
    if ( !bi ) {
      break;
    }

    dlen += bi->drop(len - dlen);
    if (bi->empty()) {
      _data.pop_front();
    }
  }

  _len -= dlen;
  return dlen;
}

int DataBuffer::add(const std::shared_ptr<BufferItem> bi) {
  //_data.push_back(bi);
  return add(bi->_buffer, bi->_len);
}

int DataBuffer::add(const std::string &str) {
  return add(str.c_str(), str.size());
}

int DataBuffer::add(const DataBuffer &db) {
  if (db._len < 1) {
    return 0;
  }

  BufferItemContainer::const_iterator it = db._data.begin();
  for (; it != db._data.end(); ++it) {
    add(*it);
  }

  return db._len;
}

int DataBuffer::add(const std::shared_ptr<DataBuffer> db) {
  return add(*db);
}

int DataBuffer::add(const char *data, const int dlen) {
  BufferItem::ptr bi = BufferItem::ptr();

  if (_data.size() > 0) {
    bi = _data.back();
  }

  int alen = 0;
  while (dlen - alen > 0) {
    if (!bi || bi->available() < 1) {
      bi = std::make_shared<BufferItem>();
      _data.push_back(bi);
    }

    //int cana = bi->available();
    //if (cana > dlen -alen) {
    //  cana = dlen - alen;
    //}
    alen += bi->add(data + alen, dlen - alen);
  }

  _len += alen;

  return dlen;
}

int DataBuffer::move(std::vector<char> &v) {
  get(v);
  _data.clear();
  _len = 0;
  return v.size();
}

int DataBuffer::move(DataBuffer &db) {
  if (&(db._len) == &_len) {
    return _len;
  }

  get(db);
  _data.clear();
  _len = 0;
  return db._len;
}

int DataBuffer::move(std::shared_ptr<DataBuffer> db) {
  get(db);
  _data.clear();
  _len = 0;
  return db->_len;
}

int DataBuffer::move(char *buf, const int len) {
  if (_len < 1) {
    return 0;
  }

  int used = 0;
  memset(buf, 0x0, len);

  do {
    BufferItem::ptr bi = _data.front();

    if (bi->_len <= len - used) {
      memcpy(buf + used, bi->_buffer, bi->_len);
      used += bi->_len;
      _data.pop_front();
    } else {
      bi->move(buf + used, len-used);
      used = len;
      break;
    }

  } while (!_data.empty() && used < len);

  _len -= used;

  return used;
}

int DataBuffer::get(char *buf, int blen) {
  if (blen < _len) {
    return -1;
  }

  int spos = 0;
  std::list<std::shared_ptr<BufferItem>>::iterator beg = _data.begin();
  while (beg != _data.end()) {
    spos += (*beg)->get(buf + spos, blen - spos);
    if (spos >= blen) {
      return blen;
    }
    if (spos >= _len) {
      return _len;
    }

    ++beg;
  }

  return _len;
}

int DataBuffer::get(std::vector<char> &buf) {
  if (_len < 1) {
    return 0;
  }

  buf.resize(_len);
  get(&(*buf.begin()), _len);
  return _len;
}

int DataBuffer::get(DataBuffer &buf) {
  if (&(buf._len) == &_len) {
    return _len;
  }

  if (_len < 1) {
    return 0;
  }

  std::list<std::shared_ptr<BufferItem>>::iterator it = _data.begin();
  for (; it != _data.end(); ++it) {
    BufferItem::ptr bi = std::make_shared<BufferItem>();
    bi->set(*it);
    buf._data.emplace_back(bi);
  }

  buf._len = _len;

  return _len;
}

int DataBuffer::get(std::shared_ptr<DataBuffer> db) {
  return get(*db);
}

int DataBuffer::set(const std::vector<char> &data) {
  return set(&(*data.begin()), data.size());
}

int DataBuffer::set(const std::string &data) {
  return set(data.c_str(), data.size());
}

int DataBuffer::set(const char *data, const int dlen) {
  _data.clear();

  return add(data, dlen);
}

int DataBuffer::set(const DataBuffer &db) {
  if (&(db._len) == &_len) {
    return _len;
  }
  _data.clear();

  return add(db);
  //_data = db._data;
  //_len  = db._len;
}

int DataBuffer::set(const std::shared_ptr<DataBuffer> db) {
  return set(*db);
  //_data = db->_data;
  //_len  = db->_len;
}

void DataBuffer::add_new_buffer_item(const std::shared_ptr<BufferItem> bi) {
  _data.push_back(bi);
}


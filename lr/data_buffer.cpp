#include "data_buffer.h"

#include <memory.h>

#include "zlog.h"
#include "utils.h"

DataBuffer::DataBuffer() {
  _data.clear();
  _len    = 0;
}

void DataBuffer::clear() {
  _data.clear();
  _len    = 0;
}

int DataBuffer::size() {
  return _len;
}

int DataBuffer::drop(const int len) {
  int dlen = 0;
  BufferItem::ptr bi;
  while (dlen < len) {
    if (_data.size() < 1) {
      break;
    }

    bi = _data.front();

    dlen += bi->drop(len - dlen);
    if (bi->empty()) {
      _data.pop_front();
    }
  }

  _len -= dlen;
  return dlen;
}

int DataBuffer::add(const std::shared_ptr<BufferItem> bi) {
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
  BufferItem::ptr bi;

  if (_data.size() > 0) {
    bi = _data.back();
  }

  int alen = 0;
  while (dlen - alen > 0) {
    if (!bi || bi->available() < 1) {
      bi = MAKE_SHARED(BufferItem);
      _data.push_back(bi);
    }

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
  return move(*db);
}

int DataBuffer::move(char *buf, const int len) {
  int used = 0;
  BufferItem::ptr bi;

  if (_len < 1 || !buf) {
    return 0;
  }

  memset(buf, 0x0, len);

  while ( true ) {
    if (_data.size() < 1) {
      break;
    }

    if (used >= len || used >= _len) {
      break;
    }

    bi = _data.front();

    used += bi->move(buf + used, len - used);

    if (bi->empty()) {
      _data.pop_front();
    }
  }

  _len -= used;

  return used;
}

int DataBuffer::get(char *buf, int blen) {
  int used = 0;
  BufferItem::ptr bi;
  if (_len < 1 || !buf) {
    return 0;
  }

  for (const BufferItem::ptr &bi:_data) {
    if (used >= blen || used >= _len) {
      break;
    }

    used += bi->get(buf + used, blen - used);
  }

  return used;
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
  if ((_len < 1) || (&(buf._len) == &_len)) {
    return _len;
  }

  for (const BufferItem::ptr &bi: _data) {
    buf.add(bi);
  }

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
}

int DataBuffer::set(const std::shared_ptr<DataBuffer> db) {
  return set(*db);
}

void DataBuffer::add_new_buffer_item(const std::shared_ptr<BufferItem> bi) {
  _data.push_back(bi);
}

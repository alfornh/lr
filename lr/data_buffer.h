#ifndef _DATA_BUFFER_H__
#define _DATA_BUFFER_H__

#include <string.h>

#include <vector>
#include <string>
#include <memory>
#include <list>

class BufferItem {
public:
  typedef std::shared_ptr<BufferItem> ptr;

  enum {
    buffer_item_capacity = 1024,
  };

  BufferItem() {
    memset(_buffer, 0x0, sizeof(_buffer));
    _len = 0;
  }

  int size() {
    return _len;
  }

  bool empty() {
    return (_len == 0);
  }

  int available() {
    return BufferItem::buffer_item_capacity - _len;
  }

  void clear() {
    _len = 0;
  }

  int set(const char *buf, const int len) {
    if (len > buffer_item_capacity) {
      memmove((void *)_buffer, (const void *)buf, buffer_item_capacity);
      _len = buffer_item_capacity;
      return buffer_item_capacity;
    }
    memmove((void *)_buffer, (const void *)buf, len);
    _len = len;
    return len;
  }

  int set(const std::shared_ptr<BufferItem> bi) {
    return set(*bi);
  }

  int set(const BufferItem &bi) {
    if (bi._buffer == _buffer) {
      return _len;
    }

    memmove((void *)_buffer, (const void *)(bi._buffer), bi._len);
    _len = bi._len;
    return _len;
  }

  int add(const char *buf, const int len) {
    int ava = buffer_item_capacity - _len;
    if (ava < 1) {
      return 0;
    }

    if (len > ava) {
      memmove((void *)&(_buffer[_len]), (const void *)buf, ava);
      _len += ava;
      return ava;
    }

    memmove((void *)&(_buffer[_len]), (const void *)buf, len);
    _len += len;
    return len;
  }

  int move(BufferItem &bi) {
    if (_buffer == bi._buffer) {
      return _len;
    }

    memmove((void *)(bi._buffer), (const void *)_buffer, _len);
    bi._len = _len;
    _len = 0;
    return bi._len;
  }

  int move(char *buf, const int len) {
    int ret;
    if (len >= _len) {
      memmove((void *)buf, (const void *)_buffer, _len);
      ret = _len;
      _len = 0;
      return ret;
    }

    int i = 0;
    for (; i < len; ++i) {
      buf[i] = _buffer[i];
    }

    for (; i < _len; ++i) {
      _buffer[i - len] = _buffer[i];
    }

    _len -= len;

    return len;
  }

  int get(char *buf, const int len) {
    if (len >= _len) {
      memmove((void *)buf, (const void *)_buffer, _len);
      return _len;
    }

    int i = 0;
    for (; i < len; ++i) {
      buf[i] = _buffer[i];
    }

    return len;
  }


  int drop(const int len) {
    int ret = 0;
    if (len >= _len) {
      ret = _len;
      memset(_buffer, 0x0, sizeof(_buffer));
      _len = 0;
      return ret;
    }

    _len -= len;
    memmove((void *)_buffer, (const void *)(_buffer + len), _len);
    memset((_buffer + _len), 0x0, buffer_item_capacity - _len);
    return len;
  }

  char _buffer[BufferItem::buffer_item_capacity];
  int _len;
};

class DataBuffer {
public:
  typedef std::shared_ptr<DataBuffer> ptr;

  DataBuffer();

  void clear();

  int size();

  int set(const std::vector<char> &data);
  int set(const std::string &data);
  int set(const char *data, const int dlen);
  int set(const DataBuffer &db);
  int set(const std::shared_ptr<DataBuffer> db);

  int add(const DataBuffer &db);
  int add(const std::shared_ptr<DataBuffer> db);
  int add(const char *data, const int dlen);
  int add(const std::string &str);
  int add(const std::shared_ptr<BufferItem> bi);

  int get(char *buf, int blen);
  int get(std::vector<char> &buf);
  int get(DataBuffer &buf);
  int get(std::shared_ptr<DataBuffer> db);


  int move(char *buf, const int len);
  int move(DataBuffer &db);
  int move(std::shared_ptr<DataBuffer> db);
  int move(std::vector<char> &v);

  int drop(const int len);
public:
  // this sections functions don't used except you know how to use it.
  void add_new_buffer_item(const std::shared_ptr<BufferItem> bi);

public:
  typedef std::list<std::shared_ptr<BufferItem>> BufferItemContainer;
  BufferItemContainer _data;
  int              _len;
};

#endif//_DATA_BUFFER_H__

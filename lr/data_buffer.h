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
    _spos = 0;
  }

  int size() {
    return _len;
  }

  bool empty() {
    return (_len <= 0);
  }

  int available() {
    return BufferItem::buffer_item_capacity - (_len + _spos);
  }

  void clear() {
    _len = 0;
    _spos = 0;
    memset(_buffer, 0x0, sizeof(_buffer));
  }

  int set(const char *buf, const int len) {
    int l = len;
    if (len > buffer_item_capacity) {
      l = buffer_item_capacity;
    }

    memmove((void *)_buffer, (const void *)buf, l);
    _spos = 0;
    _len = len;
    return l;
  }

  int set(const std::shared_ptr<BufferItem> bi) {
    return set(*bi);
  }

  int set(const BufferItem &bi) {
    if (bi._buffer == _buffer) {
      return _len;
    }

    memmove((void *)_buffer, (const void *)(bi._buffer), buffer_item_capacity);

    _spos = bi._spos;
    _len = bi._len;

    return _len;
  }

  int add(const char *buf, const int len) {
    int ava = buffer_item_capacity - _len - _spos;
    if (ava < 1) {
      return 0;
    }

    if (len > ava) {
      memmove((void *)&(_buffer[_spos + _len]), (const void *)buf, ava);
      _len += ava;
      return ava;
    }

    memmove((void *)&(_buffer[_spos + _len]), (const void *)buf, len);
    _len += len;
    return len;
  }

  int move(BufferItem &bi) {
    if (_buffer == bi._buffer) {
      return _len;
    }

    memmove((void *)(bi._buffer), (const void *)&(_buffer[_spos]), _len);
    bi._len = _len;
    _len = 0;
    return bi._len;
  }

  int move(char *buf, const int len) {
    int l = get(buf, len);

    _len -= l;
    _spos += l;

    return l;
  }

  int get(char *buf, const int len) {
    if (_len < 1) {
      return 0;
    }

    int l = _len;
    if (len < _len) {
      l = len;
    }

    memmove((void *)buf, (const void *)&(_buffer[_spos]), l);

    return l;
  }

  int get_until(char *buf, const int len, const char c, bool &flag) {
    if (_len < 1) {
      return 0;
    }

    int l = _len;
    if (len < _len) {
      l = len;
    }

    flag = false;
    int i = 0;
    while (i < l) {
      if (_buffer[_spos + i] == c) {
        *(buf + i) = _buffer[_spos + i];
        flag = true;
        break;
      }

      *(buf + i) = _buffer[_spos + i];

      ++i;
    }

    return i;
  }


  int drop(const int len) {
    int ret;
    if (_len < 1) {
      return 0;
    }

    if (len > _len) {
      ret = _len;
      _len = 0;
      _spos = BufferItem::buffer_item_capacity;
      return ret;
    }

    _len -= len;
    _spos += len;

    return len;
  }

  char _buffer[BufferItem::buffer_item_capacity];
  int _len;
  int _spos;
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
  int getline(char *buf, int blen);

  int moveline(char *buf, int blen);
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

#ifndef _FILE_H__
#define _FILE_H__

#include <stdio.h>

#include <string>
#include <memory>

class File {
public:
  typedef std::shared_ptr<File> ptr;

  File(const std::string fname);
  File(const char *fname);

  ~File();

  int open();
  int close();
  int lock();
  int unlock();

  int cursor(int pos, int whence);
  int cursor_head();
  int cursor_end();
  int cursor_now();

  int read(char *buf, int len);
  int readline(char *buf, int len);
  int write(const char *buf, const int len);
  int write(const std::string &msg);

  int size();

  int sync();

public:
  std::string _name;
  FILE *_file;
  //int _fd;
  int _size;

public:
  //enum {
  //  errno_null   = 0x00,
  //  errno_nofile = 0x01,
  //};
};

#endif //_FILE_H__

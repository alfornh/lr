#include "file.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "zlog.h"


File::File(std::string fname) {
  _name = fname;
  _file = NULL;
}

File::File(const char *fname) {
  _name = std::string(fname);
  _file = NULL;
}

File::~File() {
  if (_file) {
    unlock();
    ::fclose(_file);
  }
  _file = NULL;
}

int File::close() {
  int ret = 0;
  if (_file) {
    unlock();
    ret = ::fclose(_file);
    _file = NULL;
  }

  return ret;
}

int File::open() {
  int ret;
  struct stat fst;
  _file = ::fopen(_name.c_str(), "a+");
  if (_file == NULL) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "open failed", _name);
    return -1;
  }

  ret = ::stat(_name.c_str(), &fst);
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, ret);
    return ret;
  }
  _size = fst.st_size;
  return 0;
}

int File::cursor(int pos, int whence) {
  int ret;
  if ( _file == NULL) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__);
    return -1;
  }
  ret = ::fseek(_file, pos, whence);
  return ret;
}

int File::cursor_head() {
  int ret = ::fseek(_file, 0, SEEK_SET);
  return ret;
}

int File::cursor_end() {
  int ret = ::fseek(_file, 0, SEEK_END);
  return ret;
}

int File::cursor_now() {
  return ::ftell(_file);
}

int File::read(char *buf, int len) {
  int ret;
  do {
    ret = ::fread(buf, len, 1, _file);
    if (ret <= 0) {
      if (::feof(_file)) {
        return 0;
      }
      if (errno == EINTR) {
        continue;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, ret);
      return -1;
    }

    break;
  } while ( true );

  return ret;
}

int File::readline(char *buf, int len) {
  memzero(buf, len);
  char *cr = ::fgets(buf, len, _file);
  if ( !cr ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__);
    return -1;
  }

  return strlen(buf);
}

int File::lock() {
  return 0;
  //return ::flock(_fd, LOCK_EX);
}

int File::unlock() {
  return 0;
  //return ::flock(_fd, LOCK_UN);
}

int File::sync() {
  return ::fflush(_file);
}

int File::write(const std::string &msg) {
  return File::write(msg.c_str(), msg.size());
}

int File::write(const char *buf, const int len) {
  int ret = 0;
  int l = 0;
  if ( !_file ) {
    return -1;
  }
  do {
    ret = ::fwrite(buf, 1, len, _file);
    if (ret < 0) {
      if (errno == EINTR) {
        continue;
      }
      return ret;
    }

    l += ret;
  } while ( l < len);

  //::fsync(_fd);

  _size += l;

  return l;
}

int File::size() {
  return _size;
}

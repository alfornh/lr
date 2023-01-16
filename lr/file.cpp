#include "file.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "zlog.h"

File::File(std::string fname) {
  _name = fname;
  _fd = 0;
}

File::File(const char *fname) {
  _name = std::string(fname);
  _fd = 0;
}

File::~File() {
  if (_fd) {
    unlock();
    ::close(_fd);
  }
  _fd = 0;
}

int File::close() {
  int ret;
  if (_fd) {
    unlock();
    ret = ::close(_fd);
  }

  _fd = 0;
  return ret;
}

int File::open() {
  int ret;
  struct stat fst;
  _fd = ::open(_name.c_str(), O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
  if (_fd < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, _fd, "open failed", _name);
    return _fd;
  }

  ret = ::fstat(_fd, &fst);
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, ret);
    return ret;
  }

  _size = fst.st_size;
 
  return _fd;
}

int File::cursor(int pos, int whence) {
  int ret;
  if ( _fd < 1) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, _fd);
    return -1;
  }

  ret = ::lseek(_fd, pos, whence);

  return ret;
}

int File::cursor_head() {
  int ret = ::lseek(_fd, 0, SEEK_SET);
  return ret;
}

int File::cursor_end() {
  int ret = ::lseek(_fd, 0, SEEK_END);
  return ret;
}

int File::cursor_now() {
  return ::lseek(_fd, 0, SEEK_CUR);
}

int File::read(char *buf, int len) {
  int ret;
  int l = 0;
  do {
    ret = ::read(_fd, buf + l, len - l);
    if (ret < 0) {
      if (errno == EINTR) {
        continue;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, ret);
      return -1;
    }

    if (ret == 0)  {
      break;
    }

    l += ret;
    break;

  } while ( true );

  buf[l] = '\0';

  return l;
}

int File::readline(char *buf, int len) {
  int ret;
  int l = 0;
  do {
    ret = ::read(_fd, buf + l, 1);
    if (ret < 0) {
      if (errno == EINTR) {
        continue;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, ret);
      return -1;
    }

    l += ret;

    if (ret == 0 || buf[l - 1] == '\n' || (l >= len)) {
      break;
    }

  } while ( true );

  buf[l] = '\0';

  return l;
}

int File::lock() {
  return ::flock(_fd, LOCK_EX);
}

int File::unlock() {
  return ::flock(_fd, LOCK_UN);
}

int File::sync() {
  return ::fsync(_fd);
}

int File::write(const std::string msg) {
  return File::write(msg.c_str(), msg.size());
}

int File::write(const char *buf, const int len) {
  int ret = 0;
  int l = 0;
  if ( !_fd ) {
    return -1;
  }
  do {
    ret = ::write(_fd, buf + l, len - l);
    if (ret < 0) {
      if (errno == EINTR) {
        continue;
      }

      ZLOG_ERROR(__FILE__, __LINE__, __func__, ret);
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

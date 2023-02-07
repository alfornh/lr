#include <direct.h>
#include <errno.h>
#include <io.h>


bool zfexist(const char *path) {
  int ret = _access(path, 00);
  if (ret == ENOENT) {
    return false;
  }
  return true;
}

int zmkdir(const char *path) {
  return _mkdir(path);
}


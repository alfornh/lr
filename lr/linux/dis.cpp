
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {

bool zfexist(const char *path) {
  int ret = access(path, F_OK);
  if (ret == 0) {
    return true;
  }
  return false;
}

int zmkdir(const char *path) {
  return mkdir(path, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
}

}

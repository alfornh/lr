#include "zlog.h"

#include <time.h>

#include <exception>
#include <fstream>

#include "plt/oth-inc.h"
#include "plt/dis-inc.h"

#include "file.h"
#include "utils.h"

std::shared_ptr<ZLog> ZLog::_instance = MAKE_SHARED(ZLog);

std::string Record::to_string() {
  char head[128] = {0};
  int spos = 0;
  int ret = 0;
  const char *lev ;

  head[spos++] = '[';

  if (PZLOG->_sections & ZLog::log_section::stime) {
    ret = sprintf(head + spos, "%d", _time);
    if (ret >= 0) {
      spos += ret;
    }
  }

  if (PZLOG->_sections & ZLog::log_section::slevel) {
    switch (_level) {
    case ZLog::log_level::debug:
      lev = "debug";
    break;

    case ZLog::log_level::info:
      lev = "info ";
    break;

    case ZLog::log_level::warn:
      lev = "warn ";
    break;

    case ZLog::log_level::error:
      lev = "error";
    break;

    default:
      lev = "unknown";
    }

    ret = sprintf(head + spos, " %s", lev);
    if (ret >= 0) {
      spos += ret;
    }
  }

  if (PZLOG->_sections & ZLog::log_section::sthread_id) {
    ret = sprintf(head + spos, " %ld", _tid);
    if (ret >= 0) {
      spos += ret;
    }
  }

  head[spos++] = ']';
  head[spos++] = ' ';

  return std::string(head) + _msg + "\n";
}

bool ZLog::fexist(const std::string f) {
  return zfexist(f.c_str());
}

int ZLog::rotate() {
  TIME_T now = time(0);
  std::string lfile = _path + FILE_PATH_SEPERATOR + _prefix + "." + std::to_string(now) + ".txt";

  _file = MAKE_SHARED(File, lfile);
  now = _file->open();
  if (now < 0) {
    std::cout << "can not open log file" << lfile << std::endl;
    return -1;
  }
  return 0;
}

int ZLog::init() {
  int ret;
  _path = DEFAULT_LOG_PATH;
  if (! zfexist(_path.c_str())) {
    ret = zmkdir(_path.c_str());
    if ( ret < 0 ) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "zmkdir", _path);
      return ret;
    }
  }

  set_log_file_prefix(DEFAULT_LOG_FILE_PREFIX);
  set_log_level(ZLog::log_level::debug);
  set_log_sections(
    ZLog::log_section::stime |
    ZLog::log_section::sthread_id |
    ZLog::log_section::smessage |
    ZLog::log_section::slevel
  );

  set_log_sink(
    ZLog::log_sink::file |
    ZLog::log_sink::terminal
  );

  _rotate_size = DEFAULT_ROTATE_SIZE;

  ret = rotate();
  if (ret < 0) {
    std::cout << "create log file failed" << std::endl;
    return -1;
  }

  //_thread = MAKE_SHARED(std::thread, 
  //  std::bind(&ZLog::_flush_proc, this)
  //);
  //_thread->detach();

  return 0;
}
//void ZLog::stop() {
  //_stop_flag = true;
  //std::unique_lock<std::mutex> ul(_mutex_records);
  //_cv_records.notify_all();
//}

void ZLog::record(const int lev, std::string msg) {
  Record::ptr rec;
  std::string log;

  if (_level > lev) {
    return ;
  }

  rec = MAKE_SHARED(Record, _sections, lev, msg);
  log = rec->to_string();

  {
    std::unique_lock<std::mutex> ul(_mutex_records);

    //_records.emplace_back(rec);
    if (_file->size() >= _rotate_size) {
      rotate();
    }

    if (_sinks & log_sink::file) {
      _file->write(log);
      _file->sync();

      if (lev >= log_level::error) {
       // _file->sync();
      }
    }
  }

  if (_sinks & log_sink::terminal) {
    std::cout << log;
  }

  //_cv_records.notify_one();
}

ZLog::~ZLog() {
  if (_file) {
    _file->sync();
    _file->close();
  }
}

#include "configure.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "zlog.h"
#include "data_buffer.h"
#include "file.h"
#include "utils.h"
#include "configure_item.h"

static const std::string _configure_file_name = "frame.conf";

Value::Value() {
  this->_v = "";
}

Value::Value(const char *c) {
  this->_v = std::string(c);
}

int Value::_int() {
  return std::stoi(this->_v);
}

double Value::_double() {
  return std::stod(this->_v);
}

std::string Value::_string() {
  return this->_v;
}

bool Value::valid() {
  return this->_v != "";
}

std::shared_ptr<Configure> Configure::_instance = std::make_shared<Configure>();

int Configure::init() {
  ZLOG_DEBUG(__FILE__, __LINE__, __func__);

  _tcp_accept_event_flag = false;

  _file = MAKE_SHARED(File, _configure_file_name);

  int ret = _file->open();
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "open file failed", _configure_file_name);
    return -1;
  }

  ret = _file->lock();
  if (ret < 0) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "lock file failed", _configure_file_name);
    return -1;
  }

  BufferItem::ptr bi = std::make_shared<BufferItem>();
  char *colsign = NULL;
  char *endofl = NULL;
  char *head = NULL;

  _file->cursor_head();
  while (_file->readline(bi->_buffer, BufferItem::buffer_item_capacity) > 0) {
    head = bi->_buffer;

    bool nonspace = false;
    while (*head) {
      if (*head && *head != ' ' && *head != '\r' && *head != '\n' && *head != '\t') {
        nonspace = true;
      }
      ++head;
    }

    if (!nonspace) {
      continue;
    }

    head = bi->_buffer;

    while (*head && *head == ' ') ++head;

    if (*head && *(head +1) && *head == '/' && *(head + 1) == '/') {
      continue;
    }

    colsign = strchr(head, ':');
    if (!colsign) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "configure line format error", head);
      continue;
    }

    char *vbeg = colsign + 1;

    while (*colsign && (*colsign == ' ' || *colsign == ':')) {
      *colsign = '\0';
      --colsign;
    }

    endofl  = strchr(vbeg, '\n');
    if (!endofl) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "configure line exceed maximum characters", head);
      continue;
    }

    while (*endofl && (*endofl == '\r' || *endofl == '\n' || *endofl == ' ')) {
      *endofl = '\0';
      --endofl;
    }

    while (*vbeg && *vbeg == ' ') {
      ++vbeg;
    }

    _configures[std::string(head)] = std::make_shared<Value>(vbeg);

    ZLOG_DEBUG(__FILE__, __LINE__, __func__, head, ":", vbeg);
  }

  _file->close();

  return parse_configure();
}

int Configure::parse_configure() {
  std::shared_ptr<ConfigureItem> ci = parse_server_info();
  if ( ci ) {
    _configure_items[ci->_id] = ci;
  }

  ci = parse_timer_line();
  if ( ci ) {
    _configure_items[ci->_id] = ci;
  }

  parse_left_end();

  parse_right_end();

  return 0;
}

std::shared_ptr<ConfigureItem> Configure::parse_server_info() {
  ZLOG_INFO(__FILE__, __LINE__, __func__);

  std::shared_ptr<ConfigureServerInfo> csi = std::make_shared<ConfigureServerInfo>();
  ConfigureContainer::iterator it = _configures.find("server_info");
  if (it == _configures.end()) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "no server_info");
    return csi;
  }

  const char *si = it->second->_v.c_str();

  char word[64] = {0};
  si = extract_section_from_str(si, word, sizeof(word), ',');

  csi->_server_id = std::string(word);

  si = extract_section_from_str(si, word, sizeof(word), ',');
  csi->_server_type = std::string(word);

  si = extract_section_from_str(si, word, sizeof(word), ',');
  csi->_server_name = std::string(word);

  si = extract_section_from_str(si, word, sizeof(word), ',');
  csi->_max_concurrency = atoll(word);

  csi->_notes = std::string(si);

  return csi;
}

std::shared_ptr<ConfigureItem> Configure::parse_timer_line() {
  ZLOG_INFO(__FILE__, __LINE__, __func__);

  ConfigureTimer::ptr ct = std::make_shared<ConfigureTimer>();
  ConfigureContainer::iterator it = _configures.find("timer_line");
  if (it == _configures.end()) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "no timer_line");
    return ct;
  }
  char word[32] = {0};
  const char *t = it->second->_v.c_str();
  t = extract_section_from_str(t, word, sizeof(word), '/');
  ct->_enable = (word[0] == '1');

  memset(word, 0x0, sizeof(word));
  t = extract_section_from_str(t, word, sizeof(word), '/');
  ct->_max_worker_thread_num = atoi(word);

  return ct; 
}

std::shared_ptr<ConfigureItem> Configure::parse_lr_tu(const int ciid) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, ciid);

  char section[128];
  char word[128];
  const char *pone;
  const char *ptwo;
  std::string lrut;

  switch (ciid) {
  case CONFIGURE_LEFT_END_TCP:
    lrut = "left_tcp";
  break;

  case CONFIGURE_LEFT_END_WEBSOCKET:
    lrut = "left_websocket";
  break;

  case CONFIGURE_LEFT_END_HTTP:
    lrut = "left_http";
  break;

  case CONFIGURE_LEFT_END_UDP:
    lrut = "left_udp";
  break;

  case CONFIGURE_RIGHT_END_TCP:
    lrut = "right_tcp";
  break;

  case CONFIGURE_RIGHT_END_UDP:
    lrut = "right_udp";
  break;

  default:
    return ConfigureItem::ptr();
  }

  ConfigureNetworkLine::ptr cnl = std::make_shared<ConfigureNetworkLine>();
  cnl->_id = ciid;

  ConfigureContainer::iterator it = _configures.find(lrut);
  if (it == _configures.end()) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "can not found", lrut);
    return cnl;
  }

  pone = it->second->_v.c_str();
  ptwo = extract_section_from_str(pone, section, sizeof(section), ',');

  pone = extract_section_from_str(section, word, sizeof(word), '/');
  cnl->_enable = (word[0] == '1');

  if ( !pone ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "incomplete", lrut);
    return cnl;
  }
  pone = extract_section_from_str(pone, word, sizeof(word), '/');
  cnl->_max_worker_thread_num = atoi(word);

  if ( !ptwo ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "incomplete", lrut);
    return cnl;
  }

  if ( !ptwo ) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "incomplete", lrut);
    return cnl;
  }

  while (ptwo && *ptwo) {
    ptwo = extract_section_from_str(ptwo, word, sizeof(word), ',');

    IPInfo ipi;
    int ret = get_ipi_from_str(word, ipi);
    if (ret < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "invalid line info", word);
      continue;
    }
    cnl->_lines.emplace_back(ipi);
  }

  return cnl;
}

int Configure::get_log_level() {
  ConfigureContainer::iterator it = _configures.find("log_level");
  if (it == _configures.end()) {
    return ZLog::log_level::info;
  }

  if (it->second->_v == "debug") {
    return ZLog::log_level::debug;
  } else if (it->second->_v == "info") {
    return ZLog::log_level::info;
  } else if (it->second->_v == "warn") {
    return ZLog::log_level::warn;
  } else if (it->second->_v == "error") {
    return ZLog::log_level::error;
  }

  return ZLog::log_level::info;
}

std::shared_ptr<Value> Configure::get_value(const Key &key) {
  ConfigureContainer::iterator it = _configures.find(key);
  if (it != _configures.end()) {
    return it->second;
  }

  return Value::ptr();
}

int Configure::get_int(const Key &key, int *val) {
  Value::ptr v = get_value(key);
  if ( !v ) {
    return -1;
  }

  *val = v->_int();
  return 0;
}

int Configure::get_double(const Key &key, double *val) {
  Value::ptr v = get_value(key);
  if ( !v ) {
    return -1;
  }

  *val = v->_double();
  return 0;
}

int Configure::get_string(const Key &key, std::string &val) {
  Value::ptr v = get_value(key);
  if ( !v ) {
    return -1;
  }

  val = v->_string();
  return 0;

}

//result 0 not equal, 1 equal, -1 no key
bool Configure::is_key_equal_value(const std::string& key, const std::string& value) {
  Value::ptr v = get_value(key);
  if ( !v ) {
    return false;
  }

  if (v->_string() != value) {
    return false;
  }

  return true;
}

int Configure::get_ipi_from_str(const char *str, IPInfo &ipi) {
  char word[32];

  const char *sec = str;
  sec = extract_section_from_str(sec, word, sizeof(word), '/');
  if (strncmp(word, "udp", 3) == 0) {
    ipi._protocal = PROTOCAL_UDP;
  } else if (strncmp(word, "tcp", 3) == 0) {
    ipi._protocal = PROTOCAL_TCP;
  } else if (strncmp(word, "websocket", 3) == 0) {
    ipi._protocal = PROTOCAL_WEBSOCKET;
  } else if (strncmp(word, "http", 3) == 0) {
    ipi._protocal = PROTOCAL_HTTP;
  } else {
    ipi._protocal = PROTOCAL_NULL;
    return -1;
  }
  
  if (*sec == '\0'){
    return -1;
  }

  sec = extract_section_from_str(sec, word, sizeof(word), '/');
  ipi._ip = std::string(word);

  if (*sec == '\0'){
    return -1;
  }
  sec = extract_section_from_str(sec, word, sizeof(word), '/');
  ipi._port = atoi(word);

  if (*sec == '\0'){
    ZLOG_WARN(__FILE__, __LINE__, __func__, "no thread info");
    return 0;
  }
  sec = extract_section_from_str(sec, word, sizeof(word), '/');
  ipi._reactor_thread_num = atoi(word);

  if (*sec == '\0'){
    ZLOG_WARN(__FILE__, __LINE__, __func__, "no write thread info");
    return 0;
  }
  sec = extract_section_from_str(sec, word, sizeof(word), '/');
  ipi._io_thread_num = atoi(word);

  return 0; 
}

std::shared_ptr<ConfigureItem> Configure::get_ci(int cid) {
  ConfigureItem::ptr ci = ConfigureItem::ptr();
  ConfigureItemContainer::iterator it = _configure_items.find(cid);
  if (it == _configure_items.end()) {
    return ci;
  }

  return it->second;
}

bool Configure::line_enable(int line) {
  bool ret;
  ConfigureItemContainer::iterator it = _configure_items.find(line);
  if (it == _configure_items.end()) {
    return false;
  }

  if (line == CONFIGURE_TIMER) {
    return it->second->_enable;
  }

  ret = it->second->_enable;
  it = _configure_items.find(line & CONFIGURE_TYPE_MASK);
  if (it == _configure_items.end()) {
    return ret;
  }

  return (ret && it->second->_enable);
}

int Configure::get_max_thread_number(int line) {
  ConfigureItemContainer::iterator it;
  switch (line) {
  case CONFIGURE_LEFT_END_TCP:
  case CONFIGURE_LEFT_END_WEBSOCKET:
  case CONFIGURE_LEFT_END_HTTP:
  case CONFIGURE_LEFT_END_UDP:
  case CONFIGURE_RIGHT_END_TCP:
  case CONFIGURE_RIGHT_END_UDP:
    it = _configure_items.find(line);
    if (it == _configure_items.end()) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown line type", line);
      return -1;
    }

    return STATIC_CAST(ConfigureNetworkLine, it->second)->_max_worker_thread_num;
  break;

  case CONFIGURE_TIMER:
    it = _configure_items.find(CONFIGURE_TIMER);
    if (it == _configure_items.end()) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown line type", line);
      return -1;
    }
    return STATIC_CAST(ConfigureTimer, it->second)->_max_worker_thread_num;
  break;
  }

  return -1;
}

void Configure::parse_left_end() {

  std::shared_ptr<ConfigureLeftEnd> ci = MAKE_SHARED(ConfigureLeftEnd);
  ConfigureContainer::iterator it = _configures.find("left_end");
  if (it == _configures.end()) {
    ci->_enable = true;
    return ;

  } else {
    const char *le = it->second->_v.c_str();
    ci->_enable = (le[0] && (le[0] == '1'));
  }

  _configure_items[ci->_id] = ci;

  ConfigureItem::ptr pci = parse_lr_tu(CONFIGURE_LEFT_END_TCP);
  if ( pci ) {
    _configure_items[pci->_id] = pci;
    ci->_tcp = STATIC_CAST(ConfigureNetworkLine, pci);
  }

  pci = parse_lr_tu(CONFIGURE_LEFT_END_WEBSOCKET);
  if ( pci ) {
    _configure_items[pci->_id] = pci;
    ci->_tcp = STATIC_CAST(ConfigureNetworkLine, pci);
  }

  pci = parse_lr_tu(CONFIGURE_LEFT_END_HTTP);
  if ( pci ) {
    _configure_items[pci->_id] = pci;
    ci->_tcp = STATIC_CAST(ConfigureNetworkLine, pci);
  }

  pci = parse_lr_tu(CONFIGURE_LEFT_END_UDP);
  if ( pci ) {
    _configure_items[pci->_id] = pci;
    ci->_tcp = STATIC_CAST(ConfigureNetworkLine, pci);
  }
}

void Configure::parse_right_end() {
  std::shared_ptr<ConfigureRightEnd> ci = MAKE_SHARED(ConfigureRightEnd);
  ConfigureContainer::iterator it = _configures.find("right_end");
  if (it == _configures.end()) {
    ci->_enable = true;
  } else {
    const char *sp = it->second->_v.c_str();
    ci->_enable = (sp[0] && (sp[0] == '1'));
  }

  _configure_items[ci->_id] = ci;

  ConfigureItem::ptr pci = parse_lr_tu(CONFIGURE_RIGHT_END_TCP);
  if ( pci ) {
    _configure_items[pci->_id] = pci;
    ci->_tcp = STATIC_CAST(ConfigureNetworkLine, pci);
  }

  pci = parse_lr_tu(CONFIGURE_RIGHT_END_UDP);
  if ( pci ) {
    _configure_items[pci->_id] = pci;
    ci->_tcp = STATIC_CAST(ConfigureNetworkLine, pci);
  }
}

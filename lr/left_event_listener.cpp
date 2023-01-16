#include "left_event_listener.h"

#include "configure.h"
#include "configure_item.h"
#include "ipinfo.h"
#include "left_tcp_end.h"
#include "left_udp_end.h"
#include "thread_manager.h"
#include "timer.h"

std::shared_ptr<LeftEventListener> LeftEventListener::_instance = MAKE_SHARED(LeftEventListener);

LeftEventListener::LeftEventListener() {
}

int LeftEventListener::init() {
  int ret;

  _stop_flag = false;

  ConfigureItem::ptr c;
  ConfigureNetworkLine::ptr cnl;

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_TCP)) {
    c = PCONFIGURE->get_ci(CONFIGURE_LEFT_END_TCP);
    cnl = STATIC_CAST(ConfigureNetworkLine, c);

    for (IPInfo &ipi: cnl->_lines) {
      LeftTcpEnd::ptr tcp = MAKE_SHARED(LeftTcpEnd, ipi);
      ret = tcp->init();
      if (ret < 0) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "left tcp init");
        return -1;
      }
      _tcp_listeners.emplace_back(tcp);
      ZLOG_INFO(__FILE__, __LINE__, __func__, "tcp init ok", ipi._ip, ipi._port);
    }
    _tcp_listeners.shrink_to_fit();

  } else {
    ZLOG_WARN(__FILE__, __LINE__, __func__, "tcp not enable");
  }



  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_WEBSOCKET)) {
    c = PCONFIGURE->get_ci(CONFIGURE_LEFT_END_WEBSOCKET);
    cnl = STATIC_CAST(ConfigureNetworkLine, c);
    for (IPInfo &ipi: cnl->_lines) {
      LeftTcpEnd::ptr websocket = MAKE_SHARED(LeftTcpEnd, ipi);
      ret = websocket->init();
      if (ret < 0) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "websocket init", ipi._ip, ipi._port);
        return -1;
      }
      _websocket_listeners.emplace_back(websocket);
      ZLOG_INFO(__FILE__, __LINE__, __func__, "websocket init ok", ipi._ip, ipi._port);
    }
    _websocket_listeners.shrink_to_fit();

  } else {
    ZLOG_WARN(__FILE__, __LINE__, __func__, "websocket not enable");
  }


  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_HTTP)) {
    c = PCONFIGURE->get_ci(CONFIGURE_LEFT_END_HTTP);
    cnl = STATIC_CAST(ConfigureNetworkLine, c);

    for (IPInfo &ipi: cnl->_lines) {
      LeftTcpEnd::ptr http = MAKE_SHARED(LeftTcpEnd, ipi);
      ret = http->init();
      if (ret < 0) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "http init", ipi._ip, ipi._port);
        return -1;
      }
      _http_listeners.emplace_back(http);
      ZLOG_INFO(__FILE__, __LINE__, __func__, "http init ok", ipi._ip, ipi._port);
    }
    _http_listeners.shrink_to_fit();

  } else {
    ZLOG_WARN(__FILE__, __LINE__, __func__, "http not enable");
  }



  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_UDP)) {
    c = PCONFIGURE->get_ci(CONFIGURE_LEFT_END_UDP);
    cnl = STATIC_CAST(ConfigureNetworkLine, c);
    for (IPInfo &ipi: cnl->_lines) {
      LeftUdpEnd::ptr ul = MAKE_SHARED(LeftUdpEnd, ipi);
      ret = ul->init();
      if (ret < 0) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__,"udp", ipi._ip, ipi._port);
        return -1;
      }
      _udp_listeners.emplace_back(ul);
      ZLOG_INFO(__FILE__, __LINE__, __func__, "udp init ok", ipi._ip, ipi._port);
    }
    _udp_listeners.shrink_to_fit();

  } else {
    ZLOG_WARN(__FILE__, __LINE__, __func__, "udp not enable");
  }

  if (PCONFIGURE->line_enable(CONFIGURE_TIMER)) {
    ret = PTIMER->init();
    if (ret < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "timer init");
    }
    ZLOG_INFO(__FILE__, __LINE__, __func__, "timer init ok");
  } else {
    ZLOG_WARN(__FILE__, __LINE__, __func__, "timer not enable");
  }

  return 0; 
}

int LeftEventListener::listen() {
  if (_stop_flag) {
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "_stop_flag", _stop_flag);
    return -1;
  }

  int ret;
  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_TCP)) {
    for (LeftTcpEnd::ptr &l: _tcp_listeners) {
      ret = l->listen();
      if (ret < 0) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, l->_stype);
        return -1;
      }
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_WEBSOCKET)) {
    for (LeftTcpEnd::ptr &l: _websocket_listeners) {
      ret = l->listen();
      if (ret < 0) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, l->_stype);
        return -1;
      }
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_HTTP)) {
    for (LeftTcpEnd::ptr &l: _http_listeners) {
      ret = l->listen();
      if (ret < 0) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, l->_stype);
        return -1;
      }
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_UDP)) {
    for (LeftUdpEnd::ptr &l: _udp_listeners) {
      ret = l->listen();
      if (ret < 0) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, l->_stype);
        return -1;
      }
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_TIMER)) {
    ret = PTIMER->start();
    if (ret < 0) {
      ZLOG_ERROR(__FILE__, __LINE__, __func__, "timer start");
      return -1;
    }
  }

  return 0;
}

void LeftEventListener::stop() {
  ZLOG_WARN(__FILE__, __LINE__, __func__);

  _stop_flag = true;

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_TCP)) {
    for (LeftTcpEnd::ptr &l: _tcp_listeners) {
      l->stop();
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_WEBSOCKET)) {
    for (LeftTcpEnd::ptr &l: _websocket_listeners) {
      l->stop();
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_HTTP)) {
    for (LeftTcpEnd::ptr &l: _http_listeners) {
      l->stop();
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_LEFT_END_UDP)) {
    for (LeftUdpEnd::ptr &l: _udp_listeners) {
      l->stop();
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_TIMER)) {
    PTIMER->stop();
  }
}

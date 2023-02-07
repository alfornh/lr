#include "right_event_listener.h"

#include "configure.h"
#include "configure_item.h"
#include "right_tcp_end.h"
#include "right_udp_end.h"
#include "ipinfo.h"
#include "socket.h"
#include "event_type.h"
#include "zlog.h"
#include "configure.h"
#include "configure_item.h"


std::shared_ptr<RightEventListener> RightEventListener::_instance = MAKE_SHARED(RightEventListener);

RightEventListener::RightEventListener() {

}

int RightEventListener::init() {
  _stop_flag = false;

  ConfigureItem::ptr ci;
  ConfigureNetworkLine::ptr cnl;

  int ret;
  if (PCONFIGURE->line_enable(CONFIGURE_RIGHT_END_TCP)) {
    ci = PCONFIGURE->get_ci(CONFIGURE_RIGHT_END_TCP);
    cnl = STATIC_CAST(ConfigureNetworkLine, ci);
    for (IPInfo::ptr &ipi: cnl->_lines) {
      RightTcpEnd::ptr re = MAKE_SHARED(RightTcpEnd, ipi);
      ret = re->init();
      if (ret < 0) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "right end init", ipi->_protocal, ipi->_ip, ipi->_port);
        continue;
      }

      if (ipi->_protocal == PROTOCAL_TCP) {
        _tcp_listeners.emplace_back(re);
      } else {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "unkndown protocal", ipi->_protocal);
      }
    }
  } else {
    ZLOG_WARN(__FILE__, __LINE__, __func__, "right tcp not enable");
  }

  if (PCONFIGURE->line_enable(CONFIGURE_RIGHT_END_UDP)) {
    ci = PCONFIGURE->get_ci(CONFIGURE_RIGHT_END_UDP);
    cnl = STATIC_CAST(ConfigureNetworkLine, ci);
    for (IPInfo::ptr &ipi: cnl->_lines) {
      RightUdpEnd::ptr re = MAKE_SHARED(RightUdpEnd, ipi);
      ret = re->init();
      if (ret < 0) {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "right end init", ipi->_protocal, ipi->_ip, ipi->_port);
        continue;
      }

      if (ipi->_protocal == PROTOCAL_UDP) {
        _udp_listeners.emplace_back(re);
      } else {
        ZLOG_ERROR(__FILE__, __LINE__, __func__, "unkndown protocal", ipi->_protocal);
      }
    }
  } else {
    ZLOG_WARN(__FILE__, __LINE__, __func__, "right udp not enable");
  }

  return 0;
}

int RightEventListener::listen() {
  if (PCONFIGURE->line_enable(CONFIGURE_RIGHT_END_TCP)) {
    for (RightTcpEnd::ptr &re: _tcp_listeners) {
      if ( !re ) { 
        continue;
      }
      re->listen();
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_RIGHT_END_UDP)) {
    for (RightUdpEnd::ptr &re: _udp_listeners) {
      if ( !re ) {
        continue;
      }
      re->listen();
    }
  }

  return 0;
}

void RightEventListener::stop() {
  _stop_flag = true;
  if (PCONFIGURE->line_enable(CONFIGURE_RIGHT_END_TCP)) {
    for (RightTcpEnd::ptr &re: _tcp_listeners) {
      if ( !re ) { 
        continue;
      }

      re->stop();
    }
  }

  if (PCONFIGURE->line_enable(CONFIGURE_RIGHT_END_UDP)) {
    for (RightUdpEnd::ptr &re: _udp_listeners) {
      if ( !re ) {
        continue;
      }
      re->stop();
    }
  }
}


#include "signals_handler.h"

#include "event_type.h"
#include "event.h"
#include "zlog.h"
#include "left_object.h"
#include "right_object.h"

int SignalsHandler::handle(uint32_t id, std::shared_ptr<Event> ev) {
  ZLOG_INFO(__FILE__, __LINE__, __func__, "get signal event:", id); 
  switch (id) {
  case EVENT_TYPE_SIGNAL_SIGHUP:
    ZLOG_INFO(__FILE__, __LINE__, __func__, "EVENT_TYPE_SIGNAL_SIGHUP");
  break;

  case EVENT_TYPE_SIGNAL_SIGKILL:
    ZLOG_INFO(__FILE__, __LINE__, __func__, "EVENT_TYPE_SIGNAL_SIGKILL");
  break;

  case EVENT_TYPE_SIGNAL_SIGINT:
    ZLOG_INFO(__FILE__, __LINE__, __func__, "EVENT_TYPE_SIGNAL_SIGINT");
    PRIGHTOBJECT->stop();
    PLEFTOBJECT->stop();
  break;

  case EVENT_TYPE_SIGNAL_SIGALRM:
    ZLOG_INFO(__FILE__, __LINE__, __func__, "EVENT_TYPE_SIGNAL_SIGALRM");
  break;

  default:
    ZLOG_ERROR(__FILE__, __LINE__, __func__, "unknown signal", id);
  }

  return 0;
}

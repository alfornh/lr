#include "signal_handler.h"

#include "event.h"
#include "event_type.h"
#include "left_object.h"
#include "zlog.h"

int SignalHandler::handle(EVENTID id, std::shared_ptr<Event> ev) {
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

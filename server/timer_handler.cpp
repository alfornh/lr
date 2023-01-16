#include "timer_handler.h"

#include "zlog.h"

int TimerHandler::handle(EVENTID id, std::shared_ptr<Event>) {
  ZLOG_INFO("{}:{} {} timer id {}", __FILE__, __LINE__, __func__, id);
  return 0;
}

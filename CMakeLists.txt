cmake_minimum_required(VERSION 2.8)

PROJECT(lr)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "Compiler ${CMAKE_CXX_COMPILER} don't support c++11.")
endif()

set(lr_VERSION_MAJOR 1)
set(lr_VERSION_MINOR 0)

configure_file(
 ${PROJECT_SOURCE_DIR}/lr/lr.conf
 ${PROJECT_SOURCE_DIR}/server/lr.conf
)

MESSAGE(STATUS "Project: lr")


include_directories(
  ${PROJECT_SOURCE_DIR}/lr
)


set(FRAME_SRC 
  ${PROJECT_SOURCE_DIR}/lr/zlog.cpp
  ${PROJECT_SOURCE_DIR}/lr/data_buffer.cpp
  ${PROJECT_SOURCE_DIR}/lr/thread_manager.cpp
  ${PROJECT_SOURCE_DIR}/lr/timer.cpp
  ${PROJECT_SOURCE_DIR}/lr/utils.cpp
  ${PROJECT_SOURCE_DIR}/lr/signals.cpp
  ${PROJECT_SOURCE_DIR}/lr/event_queue.cpp
  ${PROJECT_SOURCE_DIR}/lr/configure.cpp
  ${PROJECT_SOURCE_DIR}/lr/reactor_epoll.cpp
  ${PROJECT_SOURCE_DIR}/lr/file.cpp
  ${PROJECT_SOURCE_DIR}/lr/socket.cpp
  ${PROJECT_SOURCE_DIR}/lr/tcp_socket.cpp
  ${PROJECT_SOURCE_DIR}/lr/udp_socket.cpp
  ${PROJECT_SOURCE_DIR}/lr/event_pool.cpp
  ${PROJECT_SOURCE_DIR}/lr/listener.cpp
  ${PROJECT_SOURCE_DIR}/lr/left_tcp_end.cpp
  ${PROJECT_SOURCE_DIR}/lr/left_udp_end.cpp
  ${PROJECT_SOURCE_DIR}/lr/left_event_listener.cpp
  ${PROJECT_SOURCE_DIR}/lr/left_event_dispatcher.cpp
  ${PROJECT_SOURCE_DIR}/lr/left_event_handler.cpp
  ${PROJECT_SOURCE_DIR}/lr/left_object.cpp
  ${PROJECT_SOURCE_DIR}/lr/right_tcp_end.cpp
  ${PROJECT_SOURCE_DIR}/lr/right_udp_end.cpp
  ${PROJECT_SOURCE_DIR}/lr/right_event_listener.cpp
  ${PROJECT_SOURCE_DIR}/lr/right_event_dispatcher.cpp
  ${PROJECT_SOURCE_DIR}/lr/right_event_handler.cpp
  ${PROJECT_SOURCE_DIR}/lr/right_object.cpp
)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lr)

set(FRAME_NAME lr)
set(SERVER_NAME server)


add_library(${FRAME_NAME} STATIC ${FRAME_SRC})
 


target_link_libraries(${FRAME_NAME} pthread dl)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/server)

set(SERVER_SRC
  ${PROJECT_SOURCE_DIR}/server/server.cpp
  ${PROJECT_SOURCE_DIR}/server/websocketpackage.cpp
  ${PROJECT_SOURCE_DIR}/server/left_websocket_handler.cpp
  ${PROJECT_SOURCE_DIR}/server/left_tcp_handler.cpp
  ${PROJECT_SOURCE_DIR}/server/left_udp_handler.cpp
  ${PROJECT_SOURCE_DIR}/server/left_http_handler.cpp
  ${PROJECT_SOURCE_DIR}/server/right_tcp_handler.cpp
  ${PROJECT_SOURCE_DIR}/server/right_udp_handler.cpp
  ${PROJECT_SOURCE_DIR}/server/timer_handler.cpp
  ${PROJECT_SOURCE_DIR}/server/signal_handler.cpp
)

add_executable(${SERVER_NAME} ${SERVER_SRC})

target_link_libraries(${SERVER_NAME} pthread dl ${FRAME_NAME})

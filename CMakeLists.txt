cmake_minimum_required(VERSION 2.8)

PROJECT(lr)

MESSAGE(STATUS "Project: lr")

MESSAGE(STATUS "OS: ${CMAKE_SYSTEM_NAME}")

set(PLATFORM_INC )
set(PLATFORM_SRC )

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(PLATFORM_INC
    ${PROJECT_SOURCE_DIR}/lr/linux/inc/type-inc.h
    ${PROJECT_SOURCE_DIR}/lr/linux/inc/net-inc.h
    ${PROJECT_SOURCE_DIR}/lr/linux/inc/dis-inc.h
    ${PROJECT_SOURCE_DIR}/lr/linux/inc/oth-inc.h
  )
  set(PLATFORM_SRC
    ${PROJECT_SOURCE_DIR}/lr/linux/net.cpp
    ${PROJECT_SOURCE_DIR}/lr/linux/dis.cpp
    ${PROJECT_SOURCE_DIR}/lr/linux/oth.cpp
  )
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(PLATFORM_INC
    ${PROJECT_SOURCE_DIR}/lr/win32/inc/type-inc.h
    ${PROJECT_SOURCE_DIR}/lr/win32/inc/net-inc.h
    ${PROJECT_SOURCE_DIR}/lr/win32/inc/dis-inc.h
    ${PROJECT_SOURCE_DIR}/lr/win32/inc/oth-inc.h
  )
  set (PLATFORM_SRC
    ${PROJECT_SOURCE_DIR}/lr/win32/net.cpp
    ${PROJECT_SOURCE_DIR}/lr/win32/dis.cpp
    ${PROJECT_SOURCE_DIR}/lr/win32/oth.cpp
  )
ELSE ()
  MESSAGE(FATAL_ERROR "unsupport operating system: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

file(COPY ${PLATFORM_INC} DESTINATION ${PROJECT_SOURCE_DIR}/lr/plt)


set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

add_definitions(-std=c++11)
add_definitions(-Wall)
add_definitions(-g)

set(lr_VERSION_MAJOR 1)
set(lr_VERSION_MINOR 0)

configure_file(
 ${PROJECT_SOURCE_DIR}/lr/lr.conf
 ${PROJECT_SOURCE_DIR}/server/lr.conf
)

configure_file(
 ${PROJECT_SOURCE_DIR}/lr/lr.conf
 ${PROJECT_SOURCE_DIR}/gate/lr.conf
)

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
  ${PROJECT_SOURCE_DIR}/lr/reactor_select.cpp
  ${PROJECT_SOURCE_DIR}/lr/file.cpp
  ${PROJECT_SOURCE_DIR}/lr/id_set.cpp
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


add_library(${FRAME_NAME} STATIC ${PLATFORM_SRC} ${FRAME_SRC})


IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(${FRAME_NAME} pthread dl)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

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
  ${PROJECT_SOURCE_DIR}/server/signals_handler.cpp
)

add_executable(${SERVER_NAME} ${SERVER_SRC})
target_link_libraries(${SERVER_NAME} ${FRAME_NAME})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/gate)
set(GATE_NAME gate)
set(GATE_SRC
  ${PROJECT_SOURCE_DIR}/gate/gate.cpp
  ${PROJECT_SOURCE_DIR}/gate/left_tcp_handler.cpp
  ${PROJECT_SOURCE_DIR}/gate/left_udp_handler.cpp
  ${PROJECT_SOURCE_DIR}/gate/right_tcp_handler.cpp
  ${PROJECT_SOURCE_DIR}/gate/right_udp_handler.cpp
  ${PROJECT_SOURCE_DIR}/gate/timer_handler.cpp
  ${PROJECT_SOURCE_DIR}/gate/signals_handler.cpp
  ${PROJECT_SOURCE_DIR}/gate/from_and_to.cpp
)
add_executable(${GATE_NAME} ${GATE_SRC})
target_link_libraries(${GATE_NAME} ${FRAME_NAME})


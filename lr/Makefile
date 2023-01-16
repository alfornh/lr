CXX=g++
CFLAGS=-I. -I../include -g -O0 -Wall -std=c++11 -fpermissive -fPIC
AR=ar
RM=rm -rf

ARFLAGS=crv 

FRAME_NAME=libframe.a

FRAME_OBJS = objs/zlog.o           \
     objs/data_buffer.o            \
     objs/thread_manager.o         \
     objs/timer.o                  \
     objs/utils.o                  \
     objs/signals.o                \
     objs/event_queue.o            \
     objs/configure.o              \
     objs/reactor.o                \
     objs/socket.o                 \
     objs/file.o                   \
     objs/tcp_socket.o             \
     objs/udp_socket.o             \
     objs/event_pool.o             \
     objs/listener.o               \
     objs/left_tcp_end.o           \
     objs/left_udp_end.o           \
     objs/left_event_listener.o    \
     objs/left_event_dispatcher.o  \
     objs/left_event_handler.o     \
     objs/left_object.o            \
     objs/right_tcp_end.o          \
     objs/right_udp_end.o          \
     objs/right_event_listener.o   \
     objs/right_event_dispatcher.o \
     objs/right_event_handler.o    \
     objs/right_object.o           \

$(FRAME_NAME): $(FRAME_OBJS)
	$(AR) $(ARFLAGS) $(FRAME_NAME) $(FRAME_OBJS)
	chmod 755 $(FRAME_NAME)
 
objs/zlog.o: zlog.cpp zlog.h 
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/data_buffer.o: data_buffer.cpp data_buffer.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/configure.o: configure.cpp configure.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/utils.o: utils.cpp utils.h zlog.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/socket.o: socket.cpp socket.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/file.o: file.cpp file.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/tcp_socket.o: tcp_socket.cpp tcp_socket.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/udp_socket.o: udp_socket.cpp udp_socket.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/event_queue.o: event_queue.cpp event_queue.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/left_event_listener.o: left_event_listener.cpp left_event_listener.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/timer.o: timer.cpp timer.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/signals.o: signals.cpp signals.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/reactor.o: reactor.cpp reactor.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/left_tcp_end.o: left_tcp_end.cpp left_tcp_end.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/left_udp_end.o: left_udp_end.cpp left_udp_end.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/listener.o: listener.cpp listener.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/right_tcp_end.o: right_tcp_end.cpp right_tcp_end.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/right_udp_end.o: right_udp_end.cpp right_udp_end.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/right_event_listener.o: right_event_listener.cpp right_event_listener.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/right_event_dispatcher.o: right_event_dispatcher.cpp right_event_dispatcher.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/left_event_dispatcher.o: left_event_dispatcher.cpp left_event_dispatcher.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/right_event_handler.o: right_event_handler.cpp right_event_handler.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/right_object.o:right_object.cpp right_object.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/left_event_handler.o: left_event_handler.cpp left_event_handler.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/thread_manager.o: thread_manager.cpp thread_manager.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/event_pool.o: event_pool.cpp event_pool.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

objs/left_object.o: left_object.cpp left_object.h
	$(CXX) $(CFLAGS) -c -o $@ $< 

clean:
	$(RM) $(FRAME_OBJS)
	$(RM) $(FRAME_NAME)

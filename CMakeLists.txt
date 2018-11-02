cmake_minimum_required(VERSION 3.7)
project(telnet_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")

find_package (Threads)

set(SOURCES control-protocol.h control-protocol.cc errors.h audio-transmission.cc audio-transmission.h udp.cc udp.h threading.cc threading.h)

set(RECEIVER_SOURCES receiver-main.cc radio-menu.cc radio-menu.h tcp-socket.cc tcp-socket.h byte-stream.h system-stream.cc system-stream.h telnet-processor.cc telnet-processor.h menu.cc menu.h terminal.cc terminal.h text-screen.cc text-screen.h menu-drawer.cc menu-drawer.h ${SOURCES} player.cc player.h)

set(SENDER_SOURCES sender.h sender.cc sender-main.cc ${SOURCES})

add_executable(receiver ${RECEIVER_SOURCES})

add_executable(sender ${SENDER_SOURCES})

target_link_libraries(receiver ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(sender ${CMAKE_THREAD_LIBS_INIT})
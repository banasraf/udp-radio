cmake_minimum_required(VERSION 3.10)
project(telnet_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")

add_executable(telnet_server main.cpp tcp-socket.cpp tcp-socket.h byte-stream.h system-stream.cpp system-stream.h standard-stream.h telnet-processor.cpp telnet-processor.h menu.cpp menu.h terminal.cpp terminal.h text-screen.cpp text-screen.h menu-drawer.cpp menu-drawer.h application.cpp application.h)
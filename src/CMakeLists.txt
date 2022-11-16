cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

set(SOURCES epoll_wrapper/Error.cpp
            epoll_wrapper/Event.cpp
            epoll_wrapper/Light.cpp)

add_library(epoll_wrapper ${SOURCES})

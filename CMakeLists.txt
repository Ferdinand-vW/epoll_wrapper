cmake_minimum_required(VERSION 3.8)

project(epoll_wrapper)

set(CMAKE_CXX_STANDARD 17)

set(INCLUDES ${CMAKE_CURRENT_LIST_DIR}/include)

add_subdirectory(src)

enable_testing ()
add_subdirectory(test)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
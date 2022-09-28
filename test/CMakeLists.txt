include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${GMOCK_INCLUDE_DIRS})
# include_directories(${EPOLL_INCLUDE_DIR})

add_executable(
    testEpoll
    testEpoll.cpp
)

target_link_libraries(testEpoll gtest_main gmock_main epoll_wrapper_lib)
message(${EPOLL_INCLUDE_DIR})
target_include_directories(testEpoll PUBLIC ${EPOLL_INCLUDE_DIR})

include(GoogleTest)
gtest_discover_tests(testEpoll)

cmake_minimum_required(VERSION 3.8)

project(epoll_wrapper VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
include(GNUInstallDirs)

add_subdirectory(include)
add_subdirectory(src)

enable_testing ()
add_subdirectory(test)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

target_include_directories(epoll_wrapper_lib 
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )

install(
    TARGETS epoll_wrapper_lib
    EXPORT epoll_wrapper
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT epoll_wrapper
    FILE epoll_wrapper.cmake
    DESTINATION lib/cmake/epoll_wrapper)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "Config.cmake.in"
  "epoll_wrapperConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/epoll_wrapper
  PATH_VARS CMAKE_INSTALL_LIBDIR
  )

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/epoll_wrapperConfigVersion.cmake"
    VERSION "${epoll_wrapper_VERSION_MAJOR}.${epoll_wrapper_VERSION_MINOR}"
    COMPATIBILITY AnyNewerVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/epoll_wrapperConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/epoll_wrapperConfigVersion.cmake
  DESTINATION lib/cmake/epoll_wrapper
  )

export(EXPORT epoll_wrapper
  FILE "${CMAKE_CURRENT_BINARY_DIR}/epoll_wrapper.cmake"
)

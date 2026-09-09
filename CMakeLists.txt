cmake_minimum_required(VERSION 3.20)
project(LockFreeDS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(LFDS_BUILD_TESTS "Build tests" ON)
set(LFDS_SANITIZER "" CACHE STRING "Sanitizer: thread, address, undefined, or empty")

find_package(Threads REQUIRED)

# Header-only library. Every consumer (demo, tests) inherits its flags.
add_library(lfds INTERFACE)
add_library(lfds::lfds ALIAS lfds)
target_include_directories(lfds INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lfds INTERFACE Threads::Threads)
target_compile_options(lfds INTERFACE -Wall -Wextra -Wpedantic)

if(LFDS_SANITIZER)
  target_compile_options(lfds INTERFACE -fsanitize=${LFDS_SANITIZER} -fno-omit-frame-pointer -g)
  target_link_options(lfds INTERFACE -fsanitize=${LFDS_SANITIZER})
endif()

add_executable(lfds_demo src/main.cpp)
target_link_libraries(lfds_demo PRIVATE lfds::lfds)

if(LFDS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

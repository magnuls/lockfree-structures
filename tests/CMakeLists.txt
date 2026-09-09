include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Every tests/test_<name>.cpp becomes its own test binary linked against lfds.
# Re-run cmake configure after adding a new test file.
file(GLOB LFDS_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp)

foreach(src IN LISTS LFDS_TEST_SOURCES)
  get_filename_component(name ${src} NAME_WE)
  add_executable(${name} ${src})
  target_link_libraries(${name} PRIVATE lfds::lfds GTest::gtest_main)
  gtest_discover_tests(${name})
endforeach()

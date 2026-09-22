# Every tests/test_<name>.cpp becomes its own test binary linked against lfds
# Tests use static_assert / assert and a failing assert aborts
# ctest reports a failure
# Re-run cmake configure after adding a test file.
file(
    GLOB LFDS_TEST_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp
)

foreach(src IN LISTS LFDS_TEST_SOURCES)
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE lfds::lfds)
    add_test(NAME ${name} COMMAND ${name})
endforeach()

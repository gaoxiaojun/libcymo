cmake_minimum_required(VERSION 3.1)

include_directories("../src")

file(GLOB_RECURSE TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
          ${CMAKE_CURRENT_SOURCE_DIR}/*.c)


add_executable(libcrmo_test ${TEST_SRCS})
target_link_libraries(libcrmo_test cymo ${LIBUV_LIBRARY})

add_test(NAME cymo_unit_test COMMAND libcymo_test -s)

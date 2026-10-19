cmake_minimum_required(VERSION 3.16)

project(tinygs_unit_tests LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

get_filename_component(TINYGS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

add_executable(tinygs_unit_tests
  test_main.cpp
  test_golay24.cpp
  ${TINYGS_ROOT}/tinyGS/src/BitCode/golay24.cpp
)

target_include_directories(tinygs_unit_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TINYGS_ROOT}/tinyGS/src/BitCode
)

target_link_libraries(tinygs_unit_tests PRIVATE doctest)

enable_testing()
add_test(NAME tinygs_unit_tests COMMAND tinygs_unit_tests)

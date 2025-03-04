cmake_minimum_required(VERSION 3.21)

project(CQ-SIMBE
  VERSION 0.1.0
  DESCRIPTION "CQ compliant simulated backend."
  LANGUAGES C
)

find_package(Threads REQUIRED)
find_package(QuEST REQUIRED)

add_library(cq-simbe)
add_library(CQ-SIMBE::cq-simbe ALIAS cq-simbe)

target_compile_features(cq-simbe PRIVATE c_std_99)
target_compile_options(cq-simbe PRIVATE -Wall)

option(BUILD_SHARED_LIBS "Build shared library. Turned ON by default." ON)

target_include_directories(cq-simbe
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(cq-simbe PRIVATE Threads::Threads QuEST::QuEST)

add_subdirectory(src)

add_subdirectory(examples)

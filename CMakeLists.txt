cmake_minimum_required(VERSION 3.21)

project(CQ-SIMBE
  VERSION 0.1.0
  DESCRIPTION "CQ compliant simulated backend."
  LANGUAGES C
)

# User configure options

option(BUILD_SHARED_LIBS "Build shared library. Turned ON by default." ON)
message(STATUS "BUILD_SHARED_LIBS turned ${BUILD_SHARED_LIBS}.")

option(ENABLE_TESTING "Build tests using unity testing framework. Turned ON by default." ON)
message(STATUS "ENABLE_TESTING turned ${ENABLE_TESTING}.")

option(BUILD_EXAMPLES "Build example applications. Turned OFF by default." OFF)
message(STATUS "BUILD_EXAMPLES turned ${BUILD_EXAMPLES}.")

find_package(Threads REQUIRED)
find_package(QuEST 4.0 REQUIRED)

add_library(cq-simbe)
add_library(CQ-SIMBE::cq-simbe ALIAS cq-simbe)

target_compile_features(cq-simbe PRIVATE c_std_99)
target_compile_options(cq-simbe PRIVATE -Wall)

target_include_directories(cq-simbe
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(cq-simbe PRIVATE Threads::Threads QuEST::QuEST)

add_subdirectory(src)

if (ENABLE_TESTING)
  find_package(unity REQUIRED)

  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
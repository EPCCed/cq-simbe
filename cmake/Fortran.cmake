# Keep the optional Fortran ABI and compiler runtime separate from the C library.
enable_language(Fortran)
include(GNUInstallDirs)

set(CQ_SIMBE_FORTRAN_COMPILER_ID "${CMAKE_Fortran_COMPILER_ID}")
set(CQ_SIMBE_FORTRAN_COMPILER_VERSION "${CMAKE_Fortran_COMPILER_VERSION}")
set(CQ_SIMBE_FORTRAN_MODULE_DIR "${PROJECT_BINARY_DIR}/fortran/modules")
set(CQ_SIMBE_FORTRAN_MODULE_INSTALL_DIR
  "${CMAKE_INSTALL_LIBDIR}/cq-simbe/fortran/${CQ_SIMBE_FORTRAN_COMPILER_ID}-${CQ_SIMBE_FORTRAN_COMPILER_VERSION}")
file(MAKE_DIRECTORY "${CQ_SIMBE_FORTRAN_MODULE_DIR}")

add_library(cq-simbe-fortran)
add_library(CQ-SIMBE::fortran ALIAS cq-simbe-fortran)
set_target_properties(cq-simbe-fortran PROPERTIES
  EXPORT_NAME fortran
  Fortran_MODULE_DIRECTORY "${CQ_SIMBE_FORTRAN_MODULE_DIR}"
  Fortran_PREPROCESS ON
  POSITION_INDEPENDENT_CODE ON
)
target_sources(cq-simbe-fortran
  PRIVATE "${PROJECT_SOURCE_DIR}/include/fortran/cq.f90"
  PUBLIC FILE_SET fortran_headers TYPE HEADERS
    BASE_DIRS "${PROJECT_SOURCE_DIR}/include/fortran"
    FILES "${PROJECT_SOURCE_DIR}/include/fortran/cqf.h"
)
target_link_libraries(cq-simbe-fortran PUBLIC CQ-SIMBE::cq-simbe)
target_include_directories(cq-simbe-fortran PUBLIC
  "$<BUILD_INTERFACE:${CQ_SIMBE_FORTRAN_MODULE_DIR}>"
  "$<INSTALL_INTERFACE:${CQ_SIMBE_FORTRAN_MODULE_INSTALL_DIR}>"
)
target_compile_options(cq-simbe-fortran PRIVATE
  "$<$<COMPILE_LANG_AND_ID:Fortran,GNU>:-Wimplicit-interface>"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/src/host/fortran"
  "${PROJECT_BINARY_DIR}/fortran/host")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/device/fortran"
  "${PROJECT_BINARY_DIR}/fortran/device")

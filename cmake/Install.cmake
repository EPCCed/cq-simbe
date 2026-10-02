include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(_cq_simbe_config_dir "${CMAKE_INSTALL_LIBDIR}/cmake/CQ-SIMBE")
get_target_property(_cq_simbe_library_type cq-simbe TYPE)
set(CQ_SIMBE_STATIC_PACKAGE FALSE)
if(_cq_simbe_library_type STREQUAL "STATIC_LIBRARY")
  set(CQ_SIMBE_STATIC_PACKAGE TRUE)
endif()

install(TARGETS cq-simbe EXPORT CQ-SIMBETargets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  FILE_SET HEADERS DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(EXPORT CQ-SIMBETargets
  FILE CQ-SIMBETargets.cmake NAMESPACE CQ-SIMBE::
  DESTINATION "${_cq_simbe_config_dir}"
)
configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/CQ-SIMBEConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CQ-SIMBEConfig.cmake"
  INSTALL_DESTINATION "${_cq_simbe_config_dir}"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/CQ-SIMBEConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}" COMPATIBILITY SameMinorVersion
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/CQ-SIMBEConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/CQ-SIMBEConfigVersion.cmake"
  DESTINATION "${_cq_simbe_config_dir}"
)
install(FILES "${PROJECT_SOURCE_DIR}/LICENSE"
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/CQ-SIMBE"
)

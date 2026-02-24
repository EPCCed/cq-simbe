add_subdirectory(host)
add_subdirectory(device)
add_subdirectory(host-device)
add_subdirectory(validation)

if (BUILD_FORTRAN_INTERFACE)
  add_subdirectory(fortran)
endif()

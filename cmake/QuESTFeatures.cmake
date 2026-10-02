# QuEST's generated header describes the installed binary. Do not infer its
# features from consumer-side QUEST_ENABLE_* options or redefine its macros.
include(CheckCSourceCompiles)
include(CMakePushCheckState)
cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_LIBRARIES QuEST::QuEST)
unset(CQ_SIMBE_QUEST_CONFIG_USABLE CACHE)
check_c_source_compiles("
  #include <quest/include/config.h>
  #if !defined(QUEST_COMPILE_MPI) || !defined(QUEST_COMPILE_SUBCOMM)
  #error Missing QuEST binary configuration
  #endif
  int main(void) { return 0; }
" CQ_SIMBE_QUEST_CONFIG_USABLE)
if(NOT CQ_SIMBE_QUEST_CONFIG_USABLE)
  message(FATAL_ERROR
    "The selected QuEST package is unusable. Select the corrected QuEST 4.3 "
    "package from eessmann/QuEST's cmake-packaging branch; see the configure "
    "log for missing compiler or dependency requirements.")
endif()
foreach(feature IN ITEMS MPI SUBCOMM)
  # A dependency prefix can change between configurations of the same tree.
  unset(CQ_SIMBE_HAS_${feature} CACHE)
  check_c_source_compiles("
    #include <quest/include/config.h>
    #if !QUEST_COMPILE_${feature}
    #error QuEST feature is disabled
    #endif
    int main(void) { return 0; }
  " CQ_SIMBE_HAS_${feature})
endforeach()
cmake_pop_check_state()
if(CQ_SIMBE_HAS_MPI AND NOT CQ_SIMBE_HAS_SUBCOMM)
  message(FATAL_ERROR
    "MPI-enabled QuEST requires QUEST_ENABLE_SUBCOMM=ON for cq-simbe's "
    "thread-aware MPI lifecycle. Rebuild QuEST with MPI and SUBCOMM enabled, "
    "then select that installation with QuEST_DIR or CMAKE_PREFIX_PATH.")
endif()

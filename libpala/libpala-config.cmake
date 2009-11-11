get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/pala-targets.cmake)
get_filename_component(pala_INCLUDE_DIRS "${SELF_DIR}/../../include" ABSOLUTE)

project(palapeli)

add_subdirectory(libpala)
# The following macro is needed to correctly build the following stuff (slicers and the Palapeli app).
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libpala)
add_subdirectory(slicers)
add_subdirectory(tools)

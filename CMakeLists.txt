project(palapeli)

# Below the subdirectory "libpala" are subdirectories that simulate the structure of ${INCLUDE_INSTALL_DIR}. This allows us to use the same include paths everywhere.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libpala)
add_subdirectory(libpala)
add_subdirectory(slicers)
add_subdirectory(tools)

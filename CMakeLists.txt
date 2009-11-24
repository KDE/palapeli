project(palapeli)
cmake_minimum_required(VERSION 2.6)

# allow standalone build of Palapeli (outside kdegames, e.g. in work branches)
if (NOT KDE4_FOUND)
  find_package(Qt4 4.6.0 REQUIRED)
  find_package(KDE4 REQUIRED)

  include_directories(${KDE4_INCLUDES})

  include(KDE4Defaults)
endif (NOT KDE4_FOUND)

add_subdirectory(libpala)
add_subdirectory(slicers)
add_subdirectory(tools)
add_subdirectory(src)
add_subdirectory(mime)
add_subdirectory(puzzles)

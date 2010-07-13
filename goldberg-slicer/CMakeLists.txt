include_directories(${pala_INCLUDE_DIRS})

### locate qvoronoi executable (only to print a nice warning if it's not there)

find_program(QVORONOI_EXECUTABLE NAMES qvoronoi
	# search in $ENV{PATH} only (to match the behavior of the program)
	NO_CMAKE_PATH
	NO_CMAKE_ENVIRONMENT_PATH
	NO_CMAKE_SYSTEM_PATH
	#NO_SYSTEM_ENVIRONMENT_PATH <- not this one, because we want $ENV{PATH} (see CMake manual for details)
)
if(QVORONOI_EXECUTABLE)
	set(QVORONOI_FOUND TRUE)
else(QVORONOI_EXECUTABLE)
	set(QVORONOI_FOUND FALSE)
endif(QVORONOI_EXECUTABLE)

macro_log_feature(QVORONOI_FOUND "qvoronoi" "Part of the qhull package, needed for the irregular grid from the Palapeli Slicer Collection" "http://www.qhull.org/download" FALSE "" "")

### build

set(palapeli_goldbergslicer_SRCS
	src/slicer-goldberg.cpp
	src/goldberg-engine.cpp
	src/grid-cairo.cpp
	src/grid-hex.cpp
	src/grid-rect.cpp
	src/grid-rotrex.cpp
	src/grid-voronoi.cpp
    src/pointfinder.cpp
	src/utilities.cpp
)

kde4_add_plugin(palapeli_goldbergslicer ${palapeli_goldbergslicer_SRCS})
target_link_libraries(palapeli_goldbergslicer pala ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
#TODO: Fix libpala's link interface.

install(TARGETS palapeli_goldbergslicer DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES palapeli_goldbergslicer.desktop DESTINATION ${SERVICES_INSTALL_DIR})

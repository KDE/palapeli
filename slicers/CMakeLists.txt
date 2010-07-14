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



#correctly include the headers of libpala when they are not yet installed
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS USE_LOCAL_LIBPALA)
set(SLICER_LINK_LIBS ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} pala)

kde4_add_plugin(palapeli_rectslicer slicer-rect.cpp)
target_link_libraries(palapeli_rectslicer ${SLICER_LINK_LIBS})

kde4_add_plugin(palapeli_jigsawslicer slicer-jigsaw.cpp)
target_link_libraries(palapeli_jigsawslicer ${SLICER_LINK_LIBS})

set(palapeli_goldbergslicer_SRCS
	goldberg/slicer-goldberg.cpp
	goldberg/goldberg-engine.cpp
	goldberg/grid-cairo.cpp
	goldberg/grid-hex.cpp
	goldberg/grid-rect.cpp
	goldberg/grid-rotrex.cpp
	goldberg/grid-voronoi.cpp
	goldberg/pointfinder.cpp
	goldberg/utilities.cpp
)

kde4_add_plugin(palapeli_goldbergslicer ${palapeli_goldbergslicer_SRCS})
target_link_libraries(palapeli_goldbergslicer ${SLICER_LINK_LIBS})


install(TARGETS
	palapeli_goldbergslicer
	palapeli_rectslicer
	palapeli_jigsawslicer
DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES
	palapeli_goldbergslicer.desktop
	palapeli_rectslicer.desktop
	palapeli_jigsawslicer.desktop
DESTINATION ${SERVICES_INSTALL_DIR})

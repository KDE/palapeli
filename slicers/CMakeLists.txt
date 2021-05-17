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

#PORT IT
#macro_log_feature(QVORONOI_FOUND "qvoronoi" "Part of the qhull package, needed for the irregular grid from the Palapeli Slicer Collection" "http://www.qhull.org/download" FALSE "" "")


set(SLICER_LINK_LIBS KF5::CoreAddons KF5::I18n Qt5::Gui pala)

add_library(palapeli_rectslicer MODULE slicer-rect.cpp)
target_link_libraries(palapeli_rectslicer ${SLICER_LINK_LIBS})

add_library(palapeli_jigsawslicer MODULE slicer-jigsaw.cpp)
target_link_libraries(palapeli_jigsawslicer ${SLICER_LINK_LIBS})

add_library(palapeli_goldbergslicer MODULE)
target_link_libraries(palapeli_goldbergslicer ${SLICER_LINK_LIBS})
target_sources(palapeli_goldbergslicer PRIVATE
    goldberg/slicer-goldberg.cpp
    goldberg/goldberg-engine.cpp
    goldberg/grid-preset.cpp
    goldberg/grid-cairo.cpp
    goldberg/grid-hex.cpp
    goldberg/grid-rect.cpp
    goldberg/grid-rotrex.cpp
    goldberg/grid-voronoi.cpp
    goldberg/pointfinder.cpp
    goldberg/utilities.cpp
)

install(TARGETS
	palapeli_goldbergslicer
	palapeli_rectslicer
	palapeli_jigsawslicer
DESTINATION ${KDE_INSTALL_PLUGINDIR}/palapelislicers)

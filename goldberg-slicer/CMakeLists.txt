include_directories(${pala_INCLUDE_DIRS})

set(palapeli_goldbergslicer_SRCS
	src/slicer-goldberg.cpp
	src/goldberg-engine.cpp
	src/grid-cairo.cpp
	src/grid-hex.cpp
	src/grid-rect.cpp
	src/grid-rotrex.cpp
	src/grid-voronoi.cpp
	src/utilities.cpp
)

kde4_add_plugin(palapeli_goldbergslicer ${palapeli_goldbergslicer_SRCS})
target_link_libraries(palapeli_goldbergslicer pala ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
#TODO: Fix libpala's link interface.

install(TARGETS palapeli_goldbergslicer DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES palapeli_goldbergslicer.desktop DESTINATION ${SERVICES_INSTALL_DIR})

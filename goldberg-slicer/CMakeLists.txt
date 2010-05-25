project(libpala-goldbergslicer)
cmake_minimum_required(VERSION 2.6)

find_package(Qt4 4.6.0 REQUIRED)
find_package(KDE4 REQUIRED)
find_package(LibPala REQUIRED)

include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${pala_INCLUDE_DIRS})

set(SLICER_LINK_LIBS ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} pala)

set(SOURCE_FILES
            src/slicer-goldberg.cpp
            src/goldberg-engine.cpp 
            src/grid-cairo.cpp
            src/grid-hex.cpp
            src/grid-rect.cpp
            src/grid-rotrex.cpp
            src/grid-voronoi.cpp
            src/utilities.cpp
    )

kde4_add_plugin(palapeli_goldbergslicer ${SOURCE_FILES})
target_link_libraries(palapeli_goldbergslicer ${SLICER_LINK_LIBS})

install(TARGETS
	palapeli_goldbergslicer
DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES
	palapeli_goldbergslicer.desktop
DESTINATION ${SERVICES_INSTALL_DIR})

project(myslicer)
find_package(KDE4 REQUIRED)
find_package(LibPala REQUIRED)

include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${pala_INCLUDE_DIRS})

kde4_add_plugin(myslicer myslicer.cpp)
target_link_libraries(myslicer pala ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS})

install(TARGETS myslicer DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES myslicer.desktop DESTINATION ${SERVICES_INSTALL_DIR})

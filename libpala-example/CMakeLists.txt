project(myslicer)
find_package(KDE4 REQUIRED)
find_package(LibPala REQUIRED)

add_definitions(${LIBPALA_DEFINITIONS})
include_directories(${LIBPALA_INCLUDE_DIRS})

kde4_add_plugin(myslicer myslicer.cpp)
target_link_libraries(myslicer ${LIBPALA_LIBRARIES})

install(TARGETS myslicer DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES myslicer.desktop DESTINATION ${SERVICES_INSTALL_DIR})

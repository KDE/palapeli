project(palapeli-slicers)
#correctly include the headers of libpalapeli when they are not yet installed
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS USE_LOCAL_LIBPALA)

kde4_add_plugin(palapeli_rectslicer slicer-rect.cpp)
target_link_libraries(palapeli_rectslicer ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} pala)
# add_dependencies(palapeli_rectslicer pala)

install(TARGETS palapeli_rectslicer DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES palapeli_rectslicer.desktop DESTINATION ${SERVICES_INSTALL_DIR})

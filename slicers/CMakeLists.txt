#correctly include the headers of libpala when they are not yet installed
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS USE_LOCAL_LIBPALA)
set(SLICER_LINK_LIBS ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} pala)

kde4_add_plugin(palapeli_rectslicer slicer-rect.cpp)
target_link_libraries(palapeli_rectslicer ${SLICER_LINK_LIBS})

kde4_add_plugin(palapeli_svgslicer_jigsaw slicer-svg-jigsaw.cpp)
target_link_libraries(palapeli_svgslicer_jigsaw ${SLICER_LINK_LIBS} ${QT_QTSVG_LIBRARY})

install(TARGETS
	palapeli_rectslicer
	palapeli_svgslicer_jigsaw
DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES
	palapeli_rectslicer.desktop
	palapeli_svgslicer_jigsaw.desktop
DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(pics)

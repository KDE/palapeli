#correctly include the headers of libpala when they are not yet installed
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS USE_LOCAL_LIBPALA)
set(SLICER_LINK_LIBS ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} pala)

kde4_add_plugin(palapeli_rectslicer slicer-rect.cpp)
target_link_libraries(palapeli_rectslicer ${SLICER_LINK_LIBS})

kde4_add_plugin(palapeli_jigsawslicer slicer-jigsaw.cpp)
target_link_libraries(palapeli_jigsawslicer ${SLICER_LINK_LIBS})

install(TARGETS
	palapeli_rectslicer
	palapeli_jigsawslicer
DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES
	palapeli_rectslicer.desktop
	palapeli_jigsawslicer.desktop
DESTINATION ${SERVICES_INSTALL_DIR})

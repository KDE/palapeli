project(paladesign)

set(paladesign_SRCS
	action-addrelation.cpp
	action-removerelation.cpp
	algebra.cpp
	main.cpp
	mainwindow.cpp
	manager.cpp
	mouseinteractor.cpp
	objview.cpp
	points.cpp
	propmodel.cpp
	relation.cpp
	shapes.cpp
	view.cpp
)

kde4_add_executable(paladesign ${paladesign_SRCS})
target_link_libraries(paladesign ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS paladesign DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES paladesignui.rc DESTINATION ${DATA_INSTALL_DIR}/paladesign)
install(FILES paladesign.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

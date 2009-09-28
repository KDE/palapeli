#correctly include the headers of libpala when they are not yet installed
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS USE_LOCAL_LIBPALA)

set(palapeli_SRCS
	engine/part.cpp
	engine/piece.cpp
	engine/scene.cpp
	engine/view.cpp
	engine/viewmenu.cpp
	engine/viewmenu-components.cpp
	file-io/librarydelegate.cpp
	file-io/librarymodel.cpp
	file-io/libraryview.cpp
	file-io/puzzlereader.cpp
	main.cpp
	window/librarywidget.cpp
	window/mainwindow.cpp
	window/puzzletablewidget.cpp
	window/tabwindow.cpp
)
kde4_add_kcfg_files(palapeli_SRCS settings.kcfgc)
kde4_add_ui_files(palapeli_SRCS settings.ui)

kde4_add_executable(palapeli ${palapeli_SRCS})
target_link_libraries(palapeli pala ${KDE4_KIO_LIBS} ${KDEGAMES_LIBRARY})

install(TARGETS palapeli ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES palapeliui.rc palapeli-libraryui.rc palapeli-puzzletableui.rc DESTINATION ${DATA_INSTALL_DIR}/palapeli)
install(FILES palapeli.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
add_subdirectory(pics)

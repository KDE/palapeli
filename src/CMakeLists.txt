# CMakeLists for /palapeli/src
set(palapeli_SRCS
	main.cpp
	mainwindow.cpp
	minimap.cpp
	part.cpp
	pattern-abstract.cpp
	pattern-rect.cpp
	piece.cpp
	preview.cpp
	scene.cpp
	view.cpp
)
kde4_add_ui_files(palapeli_SRCS dialognew.ui)

kde4_add_executable(palapeli ${palapeli_SRCS})
target_link_libraries(palapeli ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDEGAMES_LIBRARY})

install(TARGETS palapeli DESTINATION ${BIN_INSTALL_DIR})
install(FILES palapeliui.rc DESTINATION ${DATA_INSTALL_DIR}/palapeli)
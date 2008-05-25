# CMakeLists for /palapeli/src
set(palapeli_SRCS
	loadaction.cpp
	main.cpp
	mainwindow.cpp
	manager.cpp
	minimap.cpp
	part.cpp
	pattern-abstract.cpp
	pattern-rect.cpp
	piece.cpp
	piecerelation.cpp
	preview.cpp
	saveaction.cpp
	savegamemodel.cpp
	savegameview.cpp
	view.cpp
	gamestorage/gamestorage.cpp
	gamestorage/gamestorageattribs.cpp
	gamestorage/gamestorageitem.cpp
)
kde4_add_ui_files(palapeli_SRCS dialognew.ui)
#add_subdirectory(gamestorage)

kde4_add_executable(palapeli ${palapeli_SRCS})
target_link_libraries(palapeli ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDEGAMES_LIBRARY})

install(TARGETS palapeli DESTINATION ${BIN_INSTALL_DIR})
install(FILES palapeliui.rc DESTINATION ${DATA_INSTALL_DIR}/palapeli)

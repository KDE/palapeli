# CMakeLists for /palapeli/src
set(palapeli_SRCS
	main.cpp
	part.cpp
	piece.cpp
	scene.cpp
	view.cpp
)

kde4_add_executable(palapeli ${palapeli_SRCS})
target_link_libraries(palapeli ${KDE4_KDEUI_LIBS} kdegames)

install(TARGETS palapeli DESTINATION ${BIN_INSTALL_DIR})

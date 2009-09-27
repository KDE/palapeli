#correctly include the headers of libpala when they are not yet installed
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS USE_LOCAL_LIBPALA)

set(palapeli_SRCS
	engine/part.cpp
	engine/piece.cpp
	engine/scene.cpp
	engine/view.cpp
	engine/viewmenu.cpp
	engine/viewmenu-components.cpp
	file-io/library.cpp
	file-io/puzzle.cpp
	main.cpp
)

kde4_add_executable(palapeli ${palapeli_SRCS})
target_link_libraries(palapeli pala ${KDE4_KDEUI_LIBS} ${KDEGAMES_LIBRARY})

install(TARGETS palapeli ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES palapeli.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
add_subdirectory(pics)

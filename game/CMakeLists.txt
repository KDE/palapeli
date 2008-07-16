project(palapeli)

set(palapelicore_LIB_MAJOR_VERSION "0")
set(palapelicore_LIB_MINOR_VERSION "1")
set(palapelicore_LIB_PATCH_VERSION "0")
set(palapelicore_LIB_VERSION "${palapelicore_LIB_MAJOR_VERSION}.${palapelicore_LIB_MINOR_VERSION}.${palapelicore_LIB_PATCH_VERSION}")
set(palapelicore_LIB_SOVERSION "0")

set(palapelicore_LINK_LIBS ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDEGAMES_LIBRARY})
if(OPENGL_FOUND)
	add_definitions(-DPALAPELI_WITH_OPENGL)
	if(WIN32)
		set(opengl_libs opengl32.lib glu32.lib)
	endif(WIN32)
	set(palapelicore_LINK_LIBS ${palapelicore_LINK_LIBS} ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
endif(OPENGL_FOUND)

set(palapeli_SRCS
	main.cpp
)
set(palapelicore_SRCS
	listmenu.cpp
	mainwindow.cpp
	manager.cpp
	minimap.cpp
	newpuzzledialog.cpp
	part.cpp
	pattern.cpp
	pattern-configuration.cpp
	pattern-rectangular.cpp
	piece.cpp
	piecerelation.cpp
	preview.cpp
	saveaction.cpp
	savegamemodel.cpp
	savegameview.cpp
	view.cpp
	../storage/gamestorage.cpp
	../storage/gamestorageattribs.cpp
	../storage/gamestorageitem.cpp
)
set(palapeli_SRCS
	main.cpp
)
kde4_add_ui_files(palapelicore_SRCS settings.ui)
kde4_add_kcfg_files(palapelicore_SRCS settings.kcfgc)

kde4_add_library(palapelicore SHARED ${palapelicore_SRCS})
target_link_libraries(palapelicore ${palapelicore_LINK_LIBS})
kde4_add_executable(palapeli ${palapeli_SRCS})
target_link_libraries(palapeli palapelicore ${KDE4_KDEUI_LIBS})

set_target_properties(palapelicore PROPERTIES
	VERSION ${palapelicore_LIB_VERSION}
	SOVERSION ${palapelicore_LIB_SOVERSION}
	DEFINE_SYMBOL MAKE_LIBPALAPELICORE
)
add_definitions(-DMAKE_LIBPALAPELICORE) #because I need that in palapeli, too

install(TARGETS palapelicore palapeli DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES macros.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME Macros)
install(FILES pattern.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME Pattern)
install(FILES pattern-configuration.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME PatternConfiguration)
install(FILES palapeliui.rc DESTINATION ${DATA_INSTALL_DIR}/palapeli)
install(FILES palapeli.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

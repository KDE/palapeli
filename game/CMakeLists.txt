project(palapeli)
add_subdirectory(desktop)
add_subdirectory(pics)

add_definitions(-DMAKE_LIBPALAPELIPATTERN) #needed to correctly include the headers of libpalapelipattern

set(palapeli_LINK_LIBS ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${KDEGAMES_LIBRARY})
if(OPENGL_FOUND)
	add_definitions(-DPALAPELI_WITH_OPENGL)
	if(WIN32)
		set(opengl_libs opengl32.lib glu32.lib)
	endif(WIN32)
	set(palapeli_LINK_LIBS ${palapeli_LINK_LIBS} ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
endif(OPENGL_FOUND)

set(palapeli_SRCS
	main.cpp
	mainwindow.cpp
	manager.cpp
	minimap.cpp
	part.cpp
	piece.cpp
	piecerelation.cpp
	preview.cpp
	textprogressbar.cpp
	view.cpp
	welcomewidget.cpp
	actions/createaction.cpp
	actions/deleteaction.cpp
	actions/exportaction.cpp
	actions/importaction.cpp
	actions/loadaction.cpp
	actions/resetaction.cpp
	library/library.cpp
	library/librarybase.cpp
	library/librarydelegate.cpp
	library/libraryfilter.cpp
	library/libraryview.cpp
	library/puzzleinfo.cpp
)
kde4_add_ui_files(palapeli_SRCS settings_appearance.ui settings_gameplay.ui)
kde4_add_kcfg_files(palapeli_SRCS settings.kcfgc)

kde4_add_executable(palapeli ${palapeli_SRCS})
target_link_libraries(palapeli palapelipattern ${palapeli_LINK_LIBS})

install(TARGETS palapeli ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES palapeliui.rc DESTINATION ${DATA_INSTALL_DIR}/palapeli)
install(FILES palapeli.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

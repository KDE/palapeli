project(palapeli)
add_subdirectory(pics)

add_definitions(-DMAKE_LIBPALAPELIBASE) #needed to correctly include the headers of libpalapelibase when they are not yet installed

set(palapeli_SRCS
	main.cpp
	mainwindow.cpp
	manager.cpp
	minimap.cpp
	preview.cpp
	welcomewidget.cpp
)
kde4_add_kcfg_files(palapeli_SRCS ../lib/settings.kcfgc)
kde4_add_ui_files(palapeli_SRCS settings_appearance.ui settings_gameplay.ui)

kde4_add_executable(palapeli ${palapeli_SRCS})
target_link_libraries(palapeli palapelibase ${KDE4_KDEUI_LIBS})

install(TARGETS palapeli ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES palapeliui.rc DESTINATION ${DATA_INSTALL_DIR}/palapeli)
install(FILES palapeli.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES palapeli_import.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

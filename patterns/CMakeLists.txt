project(palapeli-patterns)
add_subdirectory(pics)
add_definitions(-DMAKE_LIBPALAPELIBASE) #needed to correctly include the headers of libpalapelibase when they are not yet installed

set(palapeli_rectangularpattern_PART_SRCS
	pattern-rectangular.cpp
)

kde4_add_plugin(palapeli_rectangularpattern ${palapeli_rectangularpattern_PART_SRCS})
target_link_libraries(palapeli_rectangularpattern ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} palapelibase)

install(TARGETS palapeli_rectangularpattern DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES palapeli_rectangularpattern.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(palapeli_jigsawpattern_PART_SRCS
	pattern-jigsaw.cpp
)

kde4_add_plugin(palapeli_jigsawpattern ${palapeli_jigsawpattern_PART_SRCS})
target_link_libraries(palapeli_jigsawpattern ${KDE4_KDEUI_LIBS} palapelibase)

install(TARGETS palapeli_jigsawpattern DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES palapeli_jigsawpattern.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(palapeli_hexagonpattern_PART_SRCS
	pattern-hexagon.cpp
)

kde4_add_plugin(palapeli_hexagonpattern ${palapeli_hexagonpattern_PART_SRCS})
target_link_libraries(palapeli_hexagonpattern ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} palapelibase)

install(TARGETS palapeli_hexagonpattern DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES palapeli_hexagonpattern.desktop DESTINATION ${SERVICES_INSTALL_DIR})

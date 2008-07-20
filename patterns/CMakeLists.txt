project(palapeli-patterns)

set(palapeli_rectangularpattern_PART_SRCS
	pattern-rectangular.cpp
)

add_definitions(-DMAKE_LIBPALAPELIPATTERN) #needed to correctly include the headers of libpalapelipattern
kde4_add_plugin(palapeli_rectangularpattern ${palapeli_rectangularpattern_PART_SRCS})
target_link_libraries(palapeli_rectangularpattern ${KDE4_KDECORE_LIBS} palapelipattern)

install(TARGETS palapeli_rectangularpattern DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES palapeli_rectangularpattern.desktop DESTINATION ${SERVICES_INSTALL_DIR})
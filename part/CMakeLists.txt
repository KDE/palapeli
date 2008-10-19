project(palapelipart)

add_definitions(-DMAKE_LIBPALAPELIBASE) #needed to correctly include the headers of libpalapelibase when they are not yet installed

set(palapelipart_SRCS
	part.cpp
)
#TODO: Should the KPart include the settings dialogs and everything that results from this?

kde4_add_plugin(palapelipart ${palapelipart_SRCS})
target_link_libraries(palapelipart palapelibase ${KDE4_KPARTS_LIBS})

install(TARGETS palapelipart DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES palapelipart.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES palapelipart.rc DESTINATION ${DATA_INSTALL_DIR}/palapeli)

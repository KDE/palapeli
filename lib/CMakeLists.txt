project(libpalapelipattern)

set(palapelipattern_LIB_MAJOR_VERSION "0")
set(palapelipattern_LIB_MINOR_VERSION "4")
set(palapelipattern_LIB_PATCH_VERSION "0")
set(palapelipattern_LIB_VERSION "${palapelipattern_LIB_MAJOR_VERSION}.${palapelipattern_LIB_MINOR_VERSION}.${palapelipattern_LIB_PATCH_VERSION}")
set(palapelipattern_LIB_SOVERSION "0")

set(palapelipattern_SRCS
	pattern.cpp
	pattern-configuration.cpp
	pattern-executor.cpp
	pattern-plugin.cpp
	pattern-trader.cpp
	variantmapper.cpp
)

kde4_add_library(palapelipattern SHARED ${palapelipattern_SRCS})
target_link_libraries(palapelipattern ${KDE4_KDEUI_LIBS})

set_target_properties(palapelipattern PROPERTIES
	VERSION ${palapelipattern_LIB_VERSION}
	SOVERSION ${palapelipattern_LIB_SOVERSION}
	DEFINE_SYMBOL MAKE_LIBPALAPELIPATTERN
)

install(TARGETS palapelipattern ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES palapelipatternplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES macros.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME Macros)
install(FILES pattern.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME Pattern)
install(FILES pattern-configuration.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME PatternConfiguration)
install(FILES pattern-executor.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME PatternExecutor)
install(FILES pattern-plugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME PatternPlugin)
install(FILES pattern-trader.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME PatternTrader)
install(FILES FindPalapeli.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules COMPONENT Devel)

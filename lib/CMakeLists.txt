project(palapeli)

set(palapelipattern_LIB_MAJOR_VERSION "0")
set(palapelipattern_LIB_MINOR_VERSION "1")
set(palapelipattern_LIB_PATCH_VERSION "0")
set(palapelipattern_LIB_VERSION "${palapelipattern_LIB_MAJOR_VERSION}.${palapelipattern_LIB_MINOR_VERSION}.${palapelipattern_LIB_PATCH_VERSION}")
set(palapelipattern_LIB_SOVERSION "0")

set(palapelipattern_SRCS
	pattern.cpp
	pattern-configuration.cpp
	pattern-rectangular.cpp
)

kde4_add_library(palapelipattern SHARED ${palapelipattern_SRCS})
target_link_libraries(palapelipattern ${KDE4_KDEUI_LIBS})

set_target_properties(palapelipattern PROPERTIES
	VERSION ${palapelipattern_LIB_VERSION}
	SOVERSION ${palapelipattern_LIB_SOVERSION}
	DEFINE_SYMBOL MAKE_LIBPALAPELIPATTERN
)

install(TARGETS palapelipattern DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES macros.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME Macros)
install(FILES pattern.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME Pattern)
install(FILES pattern-configuration.h DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli COMPONENT Devel RENAME PatternConfiguration)

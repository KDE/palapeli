project(libpalapelipart)
add_subdirectory(devel)

set(palapelipart_LIB_MAJOR_VERSION "0")
set(palapelipart_LIB_MINOR_VERSION "4")
set(palapelipart_LIB_PATCH_VERSION "0")
set(palapelipart_LIB_VERSION "${palapelipart_LIB_MAJOR_VERSION}.${palapelipart_LIB_MINOR_VERSION}.${palapelipart_LIB_PATCH_VERSION}")
set(palapelipart_LIB_SOVERSION "0")

set(palapelipart_SRCS
	core/engine.cpp
	core/part.cpp
	core/piece.cpp
	core/piecerelation.cpp
	core/view.cpp
	library/library.cpp
	library/librarybase.cpp
	library/librarydelegate.cpp
	library/libraryfilter.cpp
	library/libraryview.cpp
	library/puzzleinfo.cpp
	patterns/pattern.cpp
	patterns/pattern-configuration.cpp
	patterns/pattern-executor.cpp
	patterns/pattern-plugin.cpp
	patterns/pattern-trader.cpp
	patterns/variantmapper.cpp
)
set(palapelipart_HEADERS #only the public interface which needs to be installed
	patterns/macros.h
	patterns/pattern.h
	patterns/pattern-configuration.h
	patterns/pattern-plugin.h
)

kde4_add_library(palapelipart SHARED ${palapelipart_SRCS})
target_link_libraries(palapelipart ${KDE4_KDEUI_LIBS})

set_target_properties(palapelipart PROPERTIES
	VERSION ${palapelipart_LIB_VERSION}
	SOVERSION ${palapelipart_LIB_SOVERSION}
	DEFINE_SYMBOL MAKE_LIBPALAPELIPART
)

install(TARGETS palapelipart ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES palapelipatternplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES ${palapelipart_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/palapelipart COMPONENT Devel)

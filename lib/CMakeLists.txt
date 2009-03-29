project(libpalapelibase)
add_subdirectory(devel)

set(palapelibase_LIB_MAJOR_VERSION "0")
set(palapelibase_LIB_MINOR_VERSION "4")
set(palapelibase_LIB_PATCH_VERSION "0")
set(palapelibase_LIB_VERSION "${palapelibase_LIB_MAJOR_VERSION}.${palapelibase_LIB_MINOR_VERSION}.${palapelibase_LIB_PATCH_VERSION}")
set(palapelibase_LIB_SOVERSION "0")

set(palapelibase_LINK_LIBS ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${KDE4_KNEWSTUFF2_LIBS} ${KDEGAMES_LIBRARY})
if(OPENGL_FOUND)
	add_definitions(-DPALAPELI_WITH_OPENGL)
	if(WIN32)
		set(opengl_libs opengl32.lib glu32.lib)
	endif(WIN32)
	set(palapelibase_LINK_LIBS ${palapelibase_LINK_LIBS} ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
endif(OPENGL_FOUND)

set(palapelibase_SRCS
	actions/commonaction.cpp
	actions/createaction.cpp
	actions/deleteaction.cpp
	actions/exportaction.cpp
	actions/importaction.cpp
	actions/kns-downloadaction.cpp
	actions/loadaction.cpp
	actions/resetaction.cpp
	actions/textureaction.cpp
	core/constraint.cpp
	core/engine.cpp
	core/part.cpp
	core/piece.cpp
	core/piecerelation.cpp
	core/view.cpp
	core/viewmenu.cpp
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
	gameloader.cpp
	textprogressbar.cpp
)
kde4_add_kcfg_files(palapelibase_SRCS settings.kcfgc)

set(palapelibase_HEADERS #only the public interface which needs to be installed
	macros.h
	patterns/pattern.h
	patterns/pattern-configuration.h
	patterns/pattern-plugin.h
)

kde4_add_library(palapelibase SHARED ${palapelibase_SRCS})
target_link_libraries(palapelibase ${palapelibase_LINK_LIBS})

set_target_properties(palapelibase PROPERTIES
	VERSION ${palapelibase_LIB_VERSION}
	SOVERSION ${palapelibase_LIB_SOVERSION}
	DEFINE_SYMBOL MAKE_LIBPALAPELIBASE
)

install(TARGETS palapelibase ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES palapelipatternplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES ${palapelibase_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/palapelibase COMPONENT Devel)

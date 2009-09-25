kde4_add_executable(libpala-puzzlebuilder libpala-puzzlebuilder.cpp)
target_link_libraries(libpala-puzzlebuilder ${KDE4_KDEUI_LIBS} pala)

install(TARGETS libpala-puzzlebuilder ${INSTALL_TARGETS_DEFAULT_ARGS})

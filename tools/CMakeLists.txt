#correctly include the headers of libpala when they are not yet installed
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS USE_LOCAL_LIBPALA)

kde4_add_executable(libpala-puzzlebuilder libpala-puzzlebuilder.cpp)
target_link_libraries(libpala-puzzlebuilder ${KDE4_KIO_LIBS} pala)

install(TARGETS libpala-puzzlebuilder ${INSTALL_TARGETS_DEFAULT_ARGS})

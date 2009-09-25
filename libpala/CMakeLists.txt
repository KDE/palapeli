project(libpala)

set(libpala_VERSION "0.1.0")
set(libpala_SOVERSION "0")

set(libpala_SRCS
	slicer.cpp
	slicerjob.cpp
	slicerproperty.cpp
)
set(libpala_HDRS
	libpalamacros.h
	slicer.h
	slicerjob.h
	slicerproperty.h
)
#TODO: create fancy headers

kde4_add_library(pala SHARED ${libpala_SRCS})
target_link_libraries(pala ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
set_target_properties(pala PROPERTIES
	VERSION ${libpala_VERSION}
	SOVERSION ${libpala_SOVERSION}
	DEFINE_SYMBOL MAKE_LIBPALA
)

install(TARGETS pala ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${libpala_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/libpala)
install(FILES libpala-slicerplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

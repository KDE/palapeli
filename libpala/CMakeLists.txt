project(libpala)

set(libpala_VERSION "0.1.0")
set(libpala_SOVERSION "0")

set(libpala_SRCS
	slicer.cpp
	slicerjob.cpp
	slicerproperty.cpp
	slicerpropertyset.cpp
)
set(libpala_HDRS
	libpalamacros.h
	slicer.h
	slicerjob.h
	slicerproperty.h
	slicerpropertyset.cpp
)
set(libpala_FANCY_HDRS
	Pala/Slicer
	Pala/SlicerJob
	Pala/SlicerProperty
	Pala/SlicerPropertySet
)

kde4_add_library(pala SHARED ${libpala_SRCS})
target_link_libraries(pala ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
set_target_properties(pala PROPERTIES
	VERSION ${libpala_VERSION}
	SOVERSION ${libpala_SOVERSION}
	DEFINE_SYMBOL MAKE_LIBPALA
)

install(TARGETS pala EXPORT pala-targets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(EXPORT pala-targets DESTINATION ${LIB_INSTALL_DIR}/libpala)
install(FILES libpala-config.cmake DESTINATION ${LIB_INSTALL_DIR}/libpala)
install(FILES ${libpala_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/libpala COMPONENT Devel)
install(FILES ${libpala_FANCY_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/Pala COMPONENT Devel)
install(FILES libpala-slicerplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

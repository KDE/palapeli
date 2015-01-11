project(libpala)

set(libpala_VERSION "0.2.0")
set(libpala_SOVERSION "0")

set(libpala_SRCS
	slicer.cpp
	slicerjob.cpp
	slicermode.cpp
	slicerproperty.cpp
	slicerpropertyset.cpp
)
set(libpala_HDRS
	libpalamacros.h
	slicer.h
	slicerjob.h
	slicermode.h
	slicerproperty.h
	slicerpropertyset.h
)
set(libpala_FANCY_HDRS
	Pala/Slicer
	Pala/SlicerJob
	Pala/SlicerMode
	Pala/SlicerProperty
	Pala/SlicerPropertySet
)

add_library(pala SHARED ${libpala_SRCS})
target_link_libraries(pala KF5::KDELibs4Support Qt5::Gui)
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

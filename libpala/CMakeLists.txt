project(libpala)

include(ECMGenerateHeaders)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

set(libpala_VERSION "0.2.0")
set(libpala_SOVERSION "0")
# Pala/ is include visibility protection layer
set(libpala_INSTALL_INCLUDEDIR ${KDE_INSTALL_INCLUDEDIR}/Pala)

add_library(pala SHARED)

target_sources(pala PRIVATE
    slicer.cpp
    slicerjob.cpp
    slicermode.cpp
    slicerproperty.cpp
    slicerpropertyset.cpp
)

ecm_qt_declare_logging_category(pala
    HEADER libpala_debug.h
    IDENTIFIER PALAPELI_LIBPALA_LOG
    CATEGORY_NAME org.kde.palapeli.libpala
    DESCRIPTION "palapeli (libpala)"
    EXPORT PALAPELI
)

set(libpala_HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/libpala_export.h
)
ecm_generate_headers(libpala_CamelCase_HEADERS
    HEADER_NAMES
        Slicer
        SlicerJob
        SlicerMode
        SlicerProperty
        SlicerPropertySet
    REQUIRED_HEADERS libpala_HDRS
    PREFIX Pala
)

generate_export_header(pala BASE_NAME libpala)
target_link_libraries(pala KF5::I18n Qt5::Gui)
target_include_directories(pala
    INTERFACE
        "$<INSTALL_INTERFACE:${libpala_INSTALL_INCLUDEDIR}>"
)
set_target_properties(pala PROPERTIES
    VERSION ${libpala_VERSION}
    SOVERSION ${libpala_SOVERSION}
    EXPORT_NAME Pala
)

install(TARGETS pala EXPORT PalaTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
# pala/ & Pala/ are class namespace path part
install(FILES ${libpala_HDRS} DESTINATION ${libpala_INSTALL_INCLUDEDIR}/pala COMPONENT Devel)
install(FILES ${libpala_CamelCase_HEADERS} DESTINATION ${libpala_INSTALL_INCLUDEDIR}/Pala COMPONENT Devel)

export(TARGETS pala FILE "${PROJECT_BINARY_DIR}/PalaTargets.cmake")

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/Pala")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/PalaConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PalaConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/PalaConfigVersion.cmake"
    VERSION ${libpala_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/PalaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/PalaConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
)

install(EXPORT PalaTargets
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT Devel
)

# define the application/x-palapeli-puzzle mimetype
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.23")
find_package(SharedMimeInfo REQUIRED)

install(FILES palapeli-mimetypes.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

# servicemenu integration
install(FILES palapeli_servicemenu.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

# thumbnail creator plugin
kde4_add_plugin(palathumbcreator thumbnail-creator.cpp)
target_link_libraries(palathumbcreator ${KDE4_KIO_LIBS})

install(TARGETS palathumbcreator DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES palathumbcreator.desktop DESTINATION ${SERVICES_INSTALL_DIR})

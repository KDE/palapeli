# define the application/x-palapeli-puzzle mimetype
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.23")
find_package(SharedMimeInfo REQUIRED)

install(FILES palapeli-mimetypes.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

# servicemenu integration
install(FILES palapeli_servicemenu.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

# thumbnail creator plugin
add_library(palathumbcreator MODULE thumbnail-creator.cpp)
target_link_libraries(palathumbcreator KF5::KIOCore KF5::KIOWidgets KF5::Archive KF5::KDELibs4Support)

install(TARGETS palathumbcreator DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES palathumbcreator.desktop DESTINATION ${SERVICES_INSTALL_DIR})

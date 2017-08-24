# define the application/x-palapeli-puzzle mimetype
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.23")
find_package(SharedMimeInfo REQUIRED)

install(FILES palapeli-mimetypes.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})

# servicemenu integration
install(FILES palapeli_servicemenu.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/ServiceMenus)

# thumbnail creator plugin
add_library(palathumbcreator MODULE thumbnail-creator.cpp)
target_link_libraries(palathumbcreator KF5::KIOCore KF5::KIOWidgets KF5::Archive)

install(TARGETS palathumbcreator DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES palathumbcreator.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

# define the application/x-palapeli-puzzle mimetype
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.23")
find_package(SharedMimeInfo REQUIRED)

install(FILES palapeli-mimetypes.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})

# servicemenu integration
install(PROGRAMS palapeli_servicemenu.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kio/servicemenus)

# thumbnail creator plugin
kcoreaddons_add_plugin(palathumbcreator SOURCES thumbnail-creator.cpp INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/thumbcreator")
target_link_libraries(palathumbcreator KF5::KIOCore KF5::KIOWidgets KF5::Archive)

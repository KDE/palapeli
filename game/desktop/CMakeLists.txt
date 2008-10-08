set(SHARED_MIME_INFO_MINIMUM_VERSION "0.23")
find_package(SharedMimeInfo REQUIRED)

########### install files ###############
install(FILES palapeli.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
install(FILES palapeli_import.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

project(palapeli)
add_subdirectory(lib) # needs to be compiled before anything else
add_subdirectory(app)
add_subdirectory(part)
#add_subdirectory(editor) # deactivated until the patterns created in this editor are actually implemented
add_subdirectory(puzzles)
add_subdirectory(patterns)

#define the x-palapeli-puzzle mimetype which is needed by the part and the application
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.23")
find_package(SharedMimeInfo REQUIRED)

install(FILES palapeli-mimetypes.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

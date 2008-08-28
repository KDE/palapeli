project(palapeli)
add_subdirectory(lib) # needs to be compiled before game
add_subdirectory(game)
add_subdirectory(cmake)
#add_subdirectory(editor) # deactivated until the patterns created in this editor are actually implemented
add_subdirectory(puzzles)
add_subdirectory(patterns)

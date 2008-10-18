set(palapelibase_PRETTY_HEADERS
	Pattern
	PatternConfiguration
	PatternPlugin
)

install(FILES FindPalapeli.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)
install(FILES ${palapelibase_PRETTY_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/Palapeli)

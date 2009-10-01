if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/castle-maintenon.pala)
	install(FILES
		castle-maintenon.pala
		cincinnati-bridge.pala
		citrus-fruits.pala
		european-honey-bee.pala
		panther-chameleon-female.pala
	DESTINATION ${DATA_INSTALL_DIR}/palapeli/puzzlelibrary)
endif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/castle-maintenon.pala)

install(FILES
	castle-maintenon.puzzle
	cincinnati-bridge.puzzle
	citrus-fruits.puzzle
	european-honey-bee.puzzle
	panther-chameleon-female.puzzle
DESTINATION ${DATA_INSTALL_DIR}/palapeli/collection)

install(FILES
	default-collection.conf
DESTINATION ${CONFIG_INSTALL_DIR}
RENAME palapeli-collectionrc)

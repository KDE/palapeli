install(FILES
	castle-maintenon.puzzle
	cincinnati-bridge.puzzle
	citrus-fruits.puzzle
	european-honey-bee.puzzle
	panther-chameleon-female.puzzle
DESTINATION ${DATA_INSTALL_DIR}/palapeli/collection
PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

install(FILES
	default-collection.conf
DESTINATION ${CONFIG_INSTALL_DIR}
PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
RENAME palapeli-collectionrc)

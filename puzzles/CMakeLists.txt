install(FILES
	castle-maintenon.desktop
	castle-maintenon.jpg
	cincinnati-bridge.desktop
	cincinnati-bridge.jpg
	citrus-fruits.desktop
	citrus-fruits.jpg
	european-honey-bee.desktop
	european-honey-bee.jpg
	panther-chameleon-female.desktop
	panther-chameleon-female.jpg
DESTINATION ${KDE_INSTALL_DATADIR}/palapeli/collection)

install(FILES
	default-collection.conf
DESTINATION ${KDE_INSTALL_CONFDIR}
RENAME palapeli-collectionrc)

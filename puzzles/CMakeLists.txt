macro(palapeli_install_puzzle name extension)
 install(FILES ${name}.${extension} ${name}.desktop DESTINATION ${DATA_INSTALL_DIR}/palapeli/puzzlelibrary)
endmacro(palapeli_install_puzzle)

palapeli_install_puzzle(castle-maintenon jpg)
palapeli_install_puzzle(cincinnati-bridge jpg)
palapeli_install_puzzle(citrus-fruits jpg)
palapeli_install_puzzle(european-honey-bee jpg)
palapeli_install_puzzle(panther-chameleon-female jpg)


#correctly include the headers of libpala when they are not yet installed
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS USE_LOCAL_LIBPALA)

set(palapeli_SRCS
	config/configdialog.cpp
	config/elidinglabel.cpp
	config/mouseinputbutton.cpp
	config/triggerconfigwidget.cpp
	config/triggerlistview.cpp
	creator/propertywidget.cpp
	creator/puzzlecreator.cpp
	creator/slicerconfwidget.cpp
	creator/slicerselector.cpp
	engine/gameplay.cpp
	engine/constraintinteractor.cpp
	engine/constraintvisualizer.cpp
	engine/interactor.cpp
	engine/interactors.cpp
	engine/interactormanager.cpp
	engine/mergegroup.cpp
	engine/piece.cpp
	engine/piecevisuals.cpp
	engine/puzzlepreview.cpp
	engine/scene.cpp
	engine/texturehelper.cpp
	engine/trigger.cpp
	engine/triggermapper.cpp
	engine/view.cpp
	engine/zoomwidget.cpp
	file-io/collection.cpp
	file-io/collection-delegate.cpp
	file-io/collection-view.cpp
	file-io/components-archivestorage.cpp
	file-io/components-collectionstorage.cpp
	file-io/components-copy.cpp
	file-io/components-creationcontext.cpp
	file-io/components-directorystorage.cpp
	file-io/components-retailstorage.cpp
	file-io/puzzle.cpp
	importhelper.cpp
	main.cpp
	window/loadingwidget.cpp
	window/mainwindow.cpp
	window/puzzletablewidget.cpp
	window/pieceholder.cpp
)
kconfig_add_kcfg_files(palapeli_SRCS settings.kcfgc)
ki18n_wrap_ui(palapeli_SRCS settings.ui)
kde4_add_app_icon(palapeli_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../pics/hi*-app-palapeli.png")

add_executable(palapeli ${palapeli_SRCS})
target_link_libraries(palapeli pala KF5::KDELibs4Support KF5::Archive Qt5::Concurrent Qt5::Svg)

install(TARGETS palapeli ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES palapeliui.rc
        DESTINATION ${KXMLGUI_INSTALL_DIR}/palapeli)

install(FILES palapeli.kcfg palapeli.notifyrc
        DESTINATION ${DATA_INSTALL_DIR}/palapeli)
install(PROGRAMS org.kde.palapeli.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
add_subdirectory(pics)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX PALAPELI
    VERSION_HEADER palapeli_version.h
)

add_executable(palapeli)

target_sources(palapeli PRIVATE
    config/configdialog.cpp config/configdialog.h
    config/elidinglabel.cpp config/elidinglabel.h
    config/mouseinputbutton.cpp config/mouseinputbutton.h
    config/triggerconfigwidget.cpp config/triggerconfigwidget.h
    config/triggerlistview.cpp config/triggerlistview.h
    creator/propertywidget.cpp creator/propertywidget.h
    creator/puzzlecreator.cpp creator/puzzlecreator.h
    creator/slicerconfwidget.cpp creator/slicerconfwidget.h
    creator/slicerselector.cpp creator/slicerselector.h
    engine/gameplay.cpp engine/gameplay.h
    engine/constraintinteractor.cpp engine/constraintinteractor.h
    engine/constraintvisualizer.cpp engine/constraintvisualizer.h
    engine/interactor.cpp
    engine/interactors.cpp engine/interactors.h
    engine/interactormanager.cpp engine/interactormanager.h
    engine/mergegroup.cpp engine/mergegroup.h
    engine/piece.cpp engine/piece.h
    engine/piecevisuals.cpp engine/piecevisuals.h
    engine/puzzlepreview.cpp engine/puzzlepreview.h
    engine/scene.cpp engine/scene.h
    engine/texturehelper.cpp engine/texturehelper.h
    engine/trigger.cpp engine/trigger.h
    engine/triggermapper.cpp engine/triggermapper.h
    engine/view.cpp
    engine/zoomwidget.cpp engine/zoomwidget.h
    file-io/collection.cpp file-io/collection.h
    file-io/collection-delegate.cpp file-io/collection-delegate.h
    file-io/collection-view.cpp file-io/collection-view.h
    file-io/components-archivestorage.cpp
    file-io/components-collectionstorage.cpp
    file-io/components-copy.cpp
    file-io/components-creationcontext.cpp
    file-io/components-directorystorage.cpp
    file-io/components-retailstorage.cpp
    file-io/puzzle.cpp file-io/puzzle.h
    importhelper.cpp importhelper.h
    main.cpp
    window/loadingwidget.cpp window/loadingwidget.h
    window/mainwindow.cpp window/mainwindow.h
    window/puzzletablewidget.cpp window/puzzletablewidget.h
    window/pieceholder.cpp window/pieceholder.h

    palapeli.qrc
)

ecm_qt_declare_logging_category(palapeli
    HEADER palapeli_debug.h
    IDENTIFIER PALAPELI_LOG
    CATEGORY_NAME org.kde.palapeli
    DESCRIPTION "palapeli"
    EXPORT PALAPELI
)

kconfig_add_kcfg_files(palapeli settings.kcfgc)

ki18n_wrap_ui(palapeli settings.ui)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../pics/*-apps-palapeli.png")
ecm_add_app_icon(palapeli ICONS ${ICONS_SRCS})

target_link_libraries(palapeli
    pala
    KDEGames6
    KF6::Archive
    KF6::ConfigWidgets
    KF6::Crash
    KF6::ItemViews
    KF6::Notifications
    KF6::KIOWidgets
    KF6::WidgetsAddons
    KF6::XmlGui
    Qt6::Svg
)

install(TARGETS palapeli ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES palapeli.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

install(FILES palapeli.kcfg
        DESTINATION ${KDE_INSTALL_DATADIR}/palapeli)
install(PROGRAMS org.kde.palapeli.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.palapeli.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

add_subdirectory(pics)

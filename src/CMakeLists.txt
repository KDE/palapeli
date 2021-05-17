ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX PALAPELI
    VERSION_HEADER palapeli_version.h
)

add_executable(palapeli)

target_sources(palapeli PRIVATE
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
    KF5KDEGames
    KF5::Archive
    KF5::ConfigWidgets
    KF5::Crash
    KF5::ItemViews
    KF5::Notifications
    KF5::KIOWidgets
    KF5::WidgetsAddons
    KF5::XmlGui
    Qt5::Svg
)

install(TARGETS palapeli ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES palapeli.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR})

install(FILES palapeli.kcfg
        DESTINATION ${KDE_INSTALL_DATADIR}/palapeli)
install(PROGRAMS org.kde.palapeli.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.palapeli.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

add_subdirectory(pics)

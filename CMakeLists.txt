project(palapeli)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.3.0")
set (KF5_MIN_VERSION "5.15.0")

find_package(ECM 1.7.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Concurrent Svg)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Config
    ConfigWidgets
    CoreAddons
    Crash
    GuiAddons
    I18n
    ItemModels
    KDELibs4Support
    KIO
    NewStuff
    NotifyConfig
    TextWidgets
    WidgetsAddons
    WindowSystem
    XmlGui
)

find_package(KF5KDEGames 4.9.0 REQUIRED)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/libpala)

add_subdirectory(libpala)
add_subdirectory(slicers)
add_subdirectory(pics)
add_subdirectory(src)
add_subdirectory(mime)
add_subdirectory(puzzles)
add_subdirectory(doc)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

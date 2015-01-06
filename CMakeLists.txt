project(palapeli)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.3.0")

find_package(ECM 1.3.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Concurrent Svg)
find_package(KF5 REQUIRED COMPONENTS 
    CoreAddons
    Archive
    Config
    ItemModels
    WidgetsAddons
    WindowSystem
    I18n
    GuiAddons
    ConfigWidgets
    TextWidgets
    XmlGui
    KIO
    NotifyConfig
    NewStuff
    KDELibs4Support)

find_package(KF5KDEGames 4.9.0 REQUIRED)

include(FeatureSummary)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DTRANSLATION_DOMAIN="palapeli")


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/libpala)

add_subdirectory(libpala)
add_subdirectory(slicers)
add_subdirectory(pics)
add_subdirectory(src)
add_subdirectory(mime)
add_subdirectory(puzzles)
add_subdirectory(doc)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

project(palapeli)

cmake_minimum_required (VERSION 3.5 FATAL_ERROR)
set (QT_MIN_VERSION "5.9.0")
set (KF5_MIN_VERSION "5.46.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Concurrent Svg)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Completion
    Config
    ConfigWidgets
    CoreAddons
    Crash
    I18n
    ItemViews
    KIO
    Notifications
    Service
    WidgetsAddons
    XmlGui
)

find_package(KF5DocTools ${KF5_MIN_VERSION})

find_package(KF5KDEGames 4.9.0 REQUIRED)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMQtDeclareLoggingCategory)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/libpala)
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x060000)
endif()
add_subdirectory(libpala)
add_subdirectory(slicers)
add_subdirectory(pics)
add_subdirectory(src)
add_subdirectory(mime)
add_subdirectory(puzzles)
if (KF5DocTools_FOUND)
    add_subdirectory(doc)
endif()

if (${ECM_VERSION} STRGREATER "5.58.0")
    install(FILES palapeli.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES palapeli.categories  DESTINATION ${KDE_INSTALL_CONFDIR})
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

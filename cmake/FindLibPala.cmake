# - Find an installation of libpala
#
# Sets the following variables:
#  LIBPALA_FOUND            - true is libpala has been found
#  LIBPALA_INCLUDE_DIR      - The include directory
#  LIBPALA_LIBRARY          - The path to libpala
#  LIBPALA_DEFINITIONS      - The minimum set of compile time definitions which slicers will have to define
#  LIBPALA_INCLUDE_DIRS     - The minimum set of include dirs which slicers will have to define
#  LIBPALA_LIBRARIES        - The minimum set of libraries which slicers will have to link to (includes QtGui for image processing and kdecore for plugin infrastructure)
#
# Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(LIBPALA_FIND_REQUIRED OR LibPala_FIND_REQUIRED)
  set(_REQ_STRING_LIBPALA REQUIRED)
else(LIBPALA_FIND_REQUIRED OR LibPala_FIND_REQUIRED)
  set(_REQ_STRING_LIBPALA)
endif(LIBPALA_FIND_REQUIRED OR LibPala_FIND_REQUIRED)
find_package(KDE4 ${_REQ_STRING_LIBPALA})

if (KDE4_FOUND)
  include(KDE4Defaults)
  include(MacroLibrary)

  if(LIBPALA_INCLUDE_DIR AND LIBPALA_LIBRARIES)
    # in cache already
    set(LIBPALA_FIND_QUIETLY TRUE)
  endif(LIBPALA_INCLUDE_DIR AND LIBPALA_LIBRARIES)

  find_path(LIBPALA_INCLUDE_DIR
    NAMES
    libpala/slicer.h
    Pala/Slicer
    HINTS
    ${INCLUDE_INSTALL_DIR}
    ${KDE4_INCLUDE_DIR}
  )

  find_library(LIBPALA_LIBRARY
    NAMES
    pala
    HINTS
    ${KDE4_LIB_DIR}
    ${LIB_INSTALL_DIR}
  )
endif (KDE4_FOUND)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libpala DEFAULT_MSG LIBPALA_INCLUDE_DIR LIBPALA_LIBRARY)

if (LIBPALA_FOUND)
  set(LIBPALA_DEFINITIONS ${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
  set(LIBPALA_INCLUDE_DIRS ${KDE4_INCLUDES} ${LIBPALA_INCLUDE_DIR})
  set(LIBPALA_LIBRARIES ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${LIBPALA_LIBRARY})
  mark_as_advanced(LIBPALA_DEFINTIONS LIBPALA_INCLUDE_DIRS LIBPALA_LIBRARIES)
endif (LIBPALA_FOUND)

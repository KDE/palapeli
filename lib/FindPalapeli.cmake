#
# Find an installation of Palapeli
#
# Sets the following variables:
#  Palapeli_FOUND             - true if Palapeli has been found
#  PALAPELI_INCLUDE_DIR       - the include directory
#  PALAPELI_PATTERN_LIBRARIES - the libpalapelipattern library

find_path(PALAPELI_INCLUDE_DIR
	NAMES
	Palapeli/Macros
	Palapeli/Pattern
	Palapeli/PatternConfiguration
	Palapeli/PatternTrader
	PATHS
	${KDE4_INCLUDE_DIR}
	${INCLUDE_INSTALL_DIR}
)
find_library(PALAPELI_PATTERN_LIBRARIES
	NAMES
	palapelipattern
	PATHS
	${KDE4_LIB_DIR}
	${LIB_INSTALL_DIR}
)

if(PALAPELI_INCLUDE_DIR)
	if(NOT Palapeli_FIND_QUIETLY)
		message(STATUS "Found Palapeli includes: ${PALAPELI_INCLUDE_DIR}")
	endif(NOT Palapeli_FIND_QUIETLY)
else(PALAPELI_INCLUDE_DIR)
	if(Palapeli_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find Palapeli includes.")
	endif(Palapeli_FIND_REQUIRED)
endif(PALAPELI_INCLUDE_DIR)

if(PALAPELI_PATTERN_LIBRARIES)
	if(NOT Palapeli_FIND_QUIETLY)
		message(STATUS "Found Palapeli libraries: ${PALAPELI_PATTERN_LIBRARIES}")
	endif(NOT Palapeli_FIND_QUIETLY)
else(PALAPELI_PATTERN_LIBRARIES)
	if(Palapeli_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find Palapeli libraries.")
	endif(Palapeli_FIND_REQUIRED)
endif(PALAPELI_PATTERN_LIBRARIES)

if(PALAPELI_INCLUDE_DIR AND PALAPELI_PATTERN_LIBRARIES)
	set(Palapeli_FOUND TRUE)
endif(PALAPELI_INCLUDE_DIR AND PALAPELI_PATTERN_LIBRARIES)

mark_as_advanced(PALAPELI_INCLUDE_DIR PALAPELI_PATTERN_LIBRARIES)

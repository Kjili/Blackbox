find_library(IRRLICHT_LIBRARY NAMES Irrlicht irrlicht.so irrlicht.1.8.so)
message(STATUS ${IRRLICHT_LIBRARY})

find_path(IRRLICHT_INCLUDE_DIR irrlicht.h PATH_SUFFIXES irrlicht)
message(STATUS ${IRRLICHT_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set IRRLICHT_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Irrlicht  DEFAULT_MSG
                                  IRRLICHT_LIBRARY IRRLICHT_INCLUDE_DIR)

mark_as_advanced(IRRLICHT_INCLUDE_DIR IRRLICHT_LIBRARY)

set(IRRLICHT_LIBRARIES ${IRRLICHT_LIBRARY})
set(IRRLICHT_INCLUDE_DIRS ${IRRLICHT_INCLUDE_DIR})

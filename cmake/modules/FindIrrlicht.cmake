#  An Irrlicht implementation of the Blackbox board game.
#
#  Copyright (C) 2018  Annemarie Mattmann
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.

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

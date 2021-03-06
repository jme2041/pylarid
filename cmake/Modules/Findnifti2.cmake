###############################################################################
#                                 _                  _       _                #
#                                | |                (_)     | |               #
#                 _ __    _   _  | |   __ _   _ __   _    __| |               #
#                | '_ \  | | | | | |  / _` | | '__| | |  / _` |               #
#                | |_) | | |_| | | | | (_| | | |    | | | (_| |               #
#                | .__/   \__, | |_|  \__,_| |_|    |_|  \__,_|               #
#                | |       __/ |                                              #
#                |_|      |___/                                               #
#                                                                             #
#                         Load Arrays of Imaging Data                         #
#                                                                             #
# Copyright (c) 2021, Jeffrey M. Engelmann                                    #
#                                                                             #
# pylarid is released under the revised (3-clause) BSD license.               #
# For details, see LICENSE.txt                                                #
#                                                                             #
###############################################################################

# Findnifti2.cmake: Find the NIfTI-2 header

include(FindPackageHandleStandardArgs)

if(DEFINED ENV{NIFTI2_DIR})
    set(NIFTI2_DIR "$ENV{NIFTI2_DIR}")
endif()

find_path(NIFTI2_INCLUDE_DIR nifti2.h HINTS "${NIFTI2_DIR}")

find_package_handle_standard_args(nifti2
    "Could NOT find 'nifti2.h' (try using -DNIFTI2_DIR=)"
    NIFTI2_INCLUDE_DIR)

if(nifti2_FOUND)
    set(NIFTI2_INCLUDE_DIRS "${NIFTI2_INCLUDE_DIR}")
    mark_as_advanced(NIFTI2_INCLUDE_DIR NIFTI2_DIR)
else()
    set(NIFTI2_DIR "" CACHE STRING
        "An optional hint to a directory for finding 'nifti2.h'")
endif()

###############################################################################

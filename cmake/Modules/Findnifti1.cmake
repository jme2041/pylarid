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

# Findnifti1.cmake: Find the NIfTI-1 header

include(FindPackageHandleStandardArgs)

if(DEFINED ENV{NIFTI1_DIR})
    set(NIFTI1_DIR "$ENV{NIFTI1_DIR}")
endif()

find_path(NIFTI1_INCLUDE_DIR nifti1.h HINTS "${NIFTI1_DIR}")

find_package_handle_standard_args(nifti1
    "Could NOT find 'nifti1.h' (try using -DNIFTI1_DIR=)"
    NIFTI1_INCLUDE_DIR)

if(nifti1_FOUND)
    set(NIFTI1_INCLUDE_DIRS "${NIFTI1_INCLUDE_DIR}")
    mark_as_advanced(NIFTI1_INCLUDE_DIR NIFTI1_DIR)
else()
    set(NIFTI1_DIR "" CACHE STRING
        "An optional hint to a directory for finding 'nifti1.h'")
endif()

###############################################################################

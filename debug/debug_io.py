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

# debug_io.py: Use this when debugging larid I/O routines

import larid


def debug_like_nifti(path, datatype=None, nt=None):
    # Coming in from "run", parameters are positional
    # Thus datatype cannot be None when nt is not none
    if datatype is None and nt is None:
        obj = larid.Dset.like_nifti(path)
    elif datatype is not None and nt is None:
        obj = larid.Dset.like_nifti(path, datatype)
    else:
        obj = larid.Dset.like_nifti(path, datatype, int(nt))
    print(obj)


def debug_from_nifti(path):
    obj = larid.Dset.from_nifti(path)
    print(obj)


def debug_to_nifti(source, destination, nifti_ver=2):
    obj1 = larid.Dset.from_nifti(source)
    obj1.to_nifti(destination, nifti_ver)
    obj2 = larid.Dset.from_nifti(destination)
    print(obj1)
    print(obj2)

###############################################################################

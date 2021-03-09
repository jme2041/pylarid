/*****************************************************************************/
/*                               _                  _       _                */
/*                              | |                (_)     | |               */
/*               _ __    _   _  | |   __ _   _ __   _    __| |               */
/*              | '_ \  | | | | | |  / _` | | '__| | |  / _` |               */
/*              | |_) | | |_| | | | | (_| | | |    | | | (_| |               */
/*              | .__/   \__, | |_|  \__,_| |_|    |_|  \__,_|               */
/*              | |       __/ |                                              */
/*              |_|      |___/                                               */
/*                                                                           */
/*                        Load Arrays of Imaging Data                        */
/*                                                                           */
/* Copyright (c) 2021, Jeffrey M. Engelmann                                  */
/*                                                                           */
/* pylarid is released under the revised (3-clause) BSD license.             */
/* For details, see LICENSE.txt                                              */
/*                                                                           */
/*****************************************************************************/

/* util.c: Miscellaneous utilities */

#define NO_IMPORT_ARRAY
#include "larid.h"

/*****************************************************************************/

/* Floating point comparison (based on numpy.allclose); b is the reference */

bool
larid_close(double const a, double const b)
{
    return fabs(a-b) <= (LARID_ABS_TOL+LARID_REL_TOL*fabs(b));
}

/*****************************************************************************/

/* Like asprintf, but uses Python memory allocators and exceptions. The buffer
 * pointed to by str should be freed by the caller using PyMem_Free().
 */

int
larid_asprintf(char * * str, char * fmt, ...)
{
    assert(!PyErr_Occurred());
    va_list argp;
    va_start(argp, fmt);
    int len = vsnprintf(NULL, 0, fmt, argp);
    if(len < 1)
    {
        if(errno != 0) PyErr_SetFromErrno(PyExc_RuntimeError);
        else PyErr_SetString(PyExc_RuntimeError, "vsnprintf encoding error");
        *str = NULL;
        return len;
    }
    va_end(argp);
    *str = PyMem_Malloc((size_t)len + 1);
    if(!(*str))
    {
        PyErr_SetNone(PyExc_MemoryError);
        return -1;
    }
    va_start(argp, fmt);
    vsnprintf(*str, (size_t)len + 1, fmt, argp);
    va_end(argp);
    assert(!PyErr_Occurred());
    return len;
}

/*****************************************************************************/

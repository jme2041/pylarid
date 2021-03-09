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

/* larid.c: Extension module larid for package pylarid */

#include "larid.h"

#ifdef LARID_ZLIB
static long const zlib = 1;
#else
static long const zlib = 0;
#endif

/*****************************************************************************/

/* Require that long long integers be 64 bits. This permits the assumption that
 * PyLong_FromLongLong() converts a 64-bit signed integer to a Python object.
 */

static_assert(sizeof(long long) == sizeof(int64_t),
              "sizeof(long long) must equal sizeof(int64_t)");

/* NIfTI assumes that short is 16 bits */

static_assert(sizeof(short) == sizeof(int16_t),
              "sizeof(short) must equal sizeof(int16_t)");

/* NIfTI assumes that int is 32 bits */

static_assert(sizeof(int) == sizeof(int32_t),
              "sizeof(int) must equal sizeof(int32_t)");

/* Verify that NIfTI header structs compile to the correct number of bytes */

static_assert(sizeof(nifti_1_header) == 348,
              "NIfTI-1 header must be 348 bytes");

static_assert(sizeof(nifti_2_header) == 540,
              "NIfTI-2 header must be 540 bytes");

/* Require 64-bit NumPy */

static_assert(sizeof(npy_intp) == sizeof(int64_t),
              "sizeof(npy_intp) must equal sizeof(int64_t)");

/*****************************************************************************/

/* Exceptions */

PyObject * LaridError;

/*****************************************************************************/

/* Module documentation string */

PyDoc_STRVAR(larid__doc__,
"Load Arrays of Imaging Data\n"
"\n"
"larid is intended to facilitate analysis of functional magnetic\n"
"resonance imaging (fMRI) datasets by providing a class for loading\n"
"datasets as NumPy arrays.");

/*****************************************************************************/

/* Module definition structure */

static PyModuleDef larid_module = {
    .m_name = "larid",
    .m_doc = larid__doc__,
    .m_size = -1
};

/*****************************************************************************/

/* Module initialization function */

PyMODINIT_FUNC
PyInit_larid()
{
    PyObject * m = NULL;

    /* Initialize NumPy */
    import_array();

    /* Finalize type objects */
    if(PyType_Ready(&DsetType)) goto error;

    /* Create the extension module object */
    m = PyModule_Create(&larid_module);
    if(!m) goto error;

    /* Add the package version string as a global */
    if(PyModule_AddStringConstant(m, "__version__", PYLARID_VER)) goto error;

    /* Add whether this is a zlib build as a global */
    if(PyModule_AddIntConstant(m, "zlib", zlib)) goto error;

    /* Create an exception type for larid */
    LaridError = PyErr_NewExceptionWithDoc(
        "larid.LaridError",
        PyDoc_STR("Exception class for the larid module"),
        NULL,
        NULL);
    if(!LaridError) goto error;

    /* Add objects to the module */
    Py_INCREF(&DsetType);   /* Reference stolen upon success */
    if(PyModule_AddObject(m, "Dset", (PyObject *)&DsetType))
    {
        Py_DECREF(&DsetType);
        goto error;
    }

    Py_INCREF(LaridError);  /* Reference stolen upon success */
    if(PyModule_AddObject(m, "LaridError", LaridError))
    {
        Py_DECREF(LaridError);
        goto error;
    }

    /* Success */
    assert(!PyErr_Occurred());
    assert(m);
    return m;

error:
    Py_XDECREF(m);
    assert(PyErr_Occurred());
    return NULL;
}

/*****************************************************************************/

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

/* larid.h: Private header for extension module larid */

#ifndef LARID_H
#define LARID_H

#define PY_ARRAY_UNIQUE_SYMBOL larid_ARRAY_API
#define NPY_NO_DEPRECATED_API NPY_1_20_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdbool.h>
#include <nifti1.h>
#include <nifti2.h>

/*****************************************************************************/

/* restrict */

#ifdef _MSC_VER
 #define restrict __restrict
#endif

/*****************************************************************************/

/* Forward declarations */

extern PyObject * LaridError;
extern PyTypeObject DsetType;
extern char const Dset_to_datatype__doc__[];
extern char const Dset_rescale__doc__[];

/*****************************************************************************/

/* larid_morder enumeration */

typedef enum larid_morder_enum {
    LARID_MORDER_TKJI = 1,
    LARID_MORDER_KJIT = 2
} larid_morder;

/*****************************************************************************/

/* DsetObject structure */

typedef struct {
    PyObject_HEAD
    nifti_2_header hdr;         /* NIfTI-2 header */
    larid_morder morder;        /* Storage order */
    PyObject * data;            /* NumPy array containing the voxel data */
} DsetObject;

/*****************************************************************************/

/* Get dataset dimensions (p is a pointer to a DsetObject) */

#define DSET_NI(p) ((p)->hdr.dim[1])
#define DSET_NJ(p) ((p)->hdr.dim[2])
#define DSET_NK(p) ((p)->hdr.dim[3])
#define DSET_NT(p) ((p)->hdr.dim[0] == 3 ? 1 : \
    (p)->hdr.dim[0] == 4 ? (p)->hdr.dim[4] : (p)->hdr.dim[5])

/*****************************************************************************/

/* Translate dataset dimensions to an npy_intp array of dimensions */

#define DIM_TKJI(d, ni, nj, nk, nt) \
    (d)[0] = (nt);                  \
    (d)[1] = (nk);                  \
    (d)[2] = (nj);                  \
    (d)[3] = (ni)

#define DIM_KJIT(d, ni, nj, nk, nt) \
    (d)[0] = (nk);                  \
    (d)[1] = (nj);                  \
    (d)[2] = (ni);                  \
    (d)[3] = (nt)

/*****************************************************************************/

/* Function declarations */

bool
larid_close(double const a, double const b);

int
larid_asprintf(char * * str, char * fmt, ...);

PyObject *
Dset___new__(
    PyTypeObject * type,
    PyObject * Py_UNUSED(args),
    PyObject * Py_UNUSED(kwds));

char const *
datatype2str(int16_t const datatype);

int16_t
datatype2bitpix(int16_t const datatype);

int
obj2datatype(PyObject * const restrict obj, int16_t * const restrict datatype);

int
datatype2typenum(int16_t const datatype);

PyObject *
Dset_get_datatype(DsetObject * self, void * Py_UNUSED(closure));

PyObject *
Dset_internal_copy(
    DsetObject * const restrict self,
    int16_t const new_datatype);

PyObject *
Dset_to_datatype(DsetObject * self, PyObject * arg);

char const *
morder2str(larid_morder const morder);

int
obj2morder(
    PyObject * const restrict obj,
    larid_morder * const restrict morder);

PyObject *
Dset_get_morder(DsetObject * self, void * Py_UNUSED(closure));

int
Dset_set_morder(
    DsetObject * self,
    PyObject * value,
    void * Py_UNUSED(closure));

PyObject *
Dset_rescale(DsetObject * self, PyObject * args, PyObject * kwds);

/*****************************************************************************/

#endif /* LARID_H */

/*****************************************************************************/

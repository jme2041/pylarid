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

/* morder.c: Memory storage order for the larid.Dset class */

#define NO_IMPORT_ARRAY
#include "larid.h"

/*****************************************************************************/

/* Cache-oblivious transpose for 2D matrices.
 * Transpose the n x p matrix pointed to by in to a p x n matrix pointed to by
 * out. in and out must point to non-overlapping memory blocks of the same
 * size.
 *
 * Based on Kumar P. (2003). Cache oblivious algorithms. In: Meyer U,
 * Sanders P, Sibeyn J (eds). Algorithms for Memory Hierarchies. Lecture Notes
 * in Computer Science, vol 2625. Springer, Berlin, Heidelberg.
 */

static void
cot(npy_intp const x,
    npy_intp const delx,
    npy_intp const y,
    npy_intp const dely,
    npy_intp const n,
    npy_intp const p,
    npy_intp const byper,           /* bytes per element */
    void * const restrict in,
    void * const restrict out)
{
    assert(n);
    assert(p);
    assert(byper);
    assert(in);
    assert(out);

    if(delx == 1 && dely == 1)
    {
        uintptr_t dest = (uintptr_t)out + (y*n + x)*byper;
        uintptr_t src = (uintptr_t)in + (x*p + y)*byper;
        memcpy((void *)dest, (void *)src, byper);
        return;
    }

    if(delx >= dely)
    {
        npy_intp xmid = delx/2;
        cot(x, xmid, y, dely, n, p, byper, in, out);
        cot(x+xmid, delx-xmid, y, dely, n, p, byper, in, out);
    }
    else
    {
        npy_intp ymid = dely/2;
        cot(x, delx, y, ymid, n, p, byper, in, out);
        cot(x, delx, y+ymid, dely-ymid, n, p, byper, in, out);
    }
}

/*****************************************************************************/

/* Lookup table for memory orders */

typedef const struct {
    const larid_morder morder;
    char const * const str;
} morder_entry;

#define MORDER_TABLE_END 0

static morder_entry morder_table[] = {
    { LARID_MORDER_TKJI, "tkji" },
    { LARID_MORDER_KJIT, "kjit" },
    { MORDER_TABLE_END, NULL }
};

/*****************************************************************************/

/* Convert a larid_morder code to a string */

char const *
morder2str(larid_morder const morder)
{
    assert(!PyErr_Occurred());
    morder_entry * p = morder_table;
    while(p->morder != MORDER_TABLE_END)
    {
        if(morder == p->morder) return p->str;
        ++p;
    }
    Py_UNREACHABLE();
}

/*****************************************************************************/

/* Convert a PyObject to a larid_morder code. This function takes the form of a
 * ParseTuple converter for O& arguments. Thus, the function takes two
 * arguments (the Python object to be converted and the address at which to
 * store the result) and returns an integer (1 for success, 0 for failure).
 */

int
obj2morder(PyObject * const restrict obj, larid_morder * const restrict morder)
{
    assert(obj);
    assert(morder);
    assert(!PyErr_Occurred());

    *morder = 0;

    if(!PyUnicode_Check(obj))
    {
        PyErr_SetString(PyExc_TypeError, "Memory order must be a string");
        return 0;
    }

    morder_entry * p = morder_table;
    while(p->morder != MORDER_TABLE_END)
    {
        if(!PyUnicode_CompareWithASCIIString(obj, p->str))
        {
            *morder = p->morder;
            return 1;
        }
        ++p;
    }

    PyErr_Format(PyExc_ValueError, "Invalid memory order: %U", obj);
    return 0;
}

/*****************************************************************************/

/* Public getter for larid.Dset.morder */

PyObject *
Dset_get_morder(DsetObject * self, void * Py_UNUSED(closure))
{
    assert(!PyErr_Occurred());
    return PyUnicode_FromString(morder2str(self->morder));
}

/*****************************************************************************/

/* Public setter for larid.Dset.morder */

int
Dset_set_morder(DsetObject * self, PyObject * value, void * Py_UNUSED(closure))
{
    PyObject * tmp = NULL;
    void * buf = NULL;
    int ret;

    assert(!PyErr_Occurred());
    Py_XINCREF(value);

    /* Prevent attribute deletion */
    if(!value)
    {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the morder attribute");
        goto except;
    }

    /* Get the desired morder */
    larid_morder new_morder;
    if(!obj2morder(value, &new_morder)) goto except;

    /* If the dataset already has the requested morder, no change is needed */
    if(self->morder == new_morder) goto ok;

    /* From this point, we are doing a transpose (n x p --> p x n) */

    /* Get new dimensions and current n x p shape */
    npy_intp dim[4], n, p;
    switch(new_morder)
    {
        case LARID_MORDER_TKJI:
            assert(self->morder == LARID_MORDER_KJIT);
            /* New dimensions */
            DIM_TKJI(dim,
                     DSET_NI(self),
                     DSET_NJ(self),
                     DSET_NK(self),
                     DSET_NT(self));
            /* Current n and p */
            n = (npy_intp)(DSET_NI(self)*DSET_NJ(self)*DSET_NK(self));
            p = (npy_intp)(DSET_NT(self));
            break;
        case LARID_MORDER_KJIT:
            assert(self->morder == LARID_MORDER_TKJI);
            /* New dimensions */
            DIM_KJIT(dim,
                     DSET_NI(self),
                     DSET_NJ(self),
                     DSET_NK(self),
                     DSET_NT(self));
            /* Current n and p */
            n = (npy_intp)(DSET_NT(self));
            p = (npy_intp)(DSET_NI(self)*DSET_NJ(self)*DSET_NK(self));
            break;
        default:
            Py_UNREACHABLE();
    }

    /* Make sure that we hold the GIL */
    if(!PyGILState_Check())
    {
        PyErr_SetString(
            PyExc_RuntimeError,
            "Attempting to change the memory order without holding the GIL");
        goto except;
    }

    /* Create a template NumPy array with the new dims and strides */
    PyArray_Dims dims = { .ptr = dim, .len = 4 };
    tmp = PyArray_Newshape((PyArrayObject *)self->data, &dims, NPY_CORDER);
    if(!tmp) goto except;

    assert(
        PyArray_DATA((PyArrayObject *)self->data) ==
        PyArray_DATA((PyArrayObject *)tmp));
    assert(((PyArrayObject_fields *)self->data)->nd == 4);
    assert(((PyArrayObject_fields *)tmp)->nd == 4);

    /* Create a temporary data buffer */
    npy_intp bytes = PyArray_NBYTES((PyArrayObject *)self->data);
    buf = PyArray_malloc((size_t)bytes);
    if(!buf)
    {
        if(!PyErr_Occurred()) PyErr_SetNone(PyExc_MemoryError);
        goto except;
    }

    /* Put the voxel data into the buffer */
    memmove(buf, PyArray_DATA((PyArrayObject *)self->data), bytes);

    /* Transpose the data block */
    cot(0, n, 0, p, n, p,
        PyArray_ITEMSIZE((PyArrayObject *)self->data),
        buf,
        PyArray_DATA((PyArrayObject *)self->data));

    /* Rearrange the NumPy array stored at self->data so that its dimensions
     * and strides reflect the transposed data. The memory address of the
     * beginning of the voxel data remains unchanged, as does the identity of
     * the NumPy array. To the caller, the data have been transposed in place.
     * This implementation is based on in-place shape modification of NumPy
     * arrays as implemented in the NumPy source code (multiarray/getset.c -->
     * array_shape_set). The number of dimensions does not differ between
     * self->data and tmp, so this is just a memcpy.
     */

    memcpy(
        PyArray_DIMS((PyArrayObject *)self->data),
        PyArray_DIMS((PyArrayObject *)tmp),
        4*sizeof(npy_intp));

    memcpy(
        PyArray_STRIDES((PyArrayObject *)self->data),
        PyArray_STRIDES((PyArrayObject *)tmp),
        4*sizeof(npy_intp));

    /* Store the new memory order */
    self->morder = new_morder;

ok:
    assert(!PyErr_Occurred());
    ret = 0;
    goto finally;

except:
    assert(PyErr_Occurred());
    ret = -1;

finally:
    PyArray_free(buf);
    Py_XDECREF(tmp);
    Py_XDECREF(value);
    return ret;
}

/*****************************************************************************/

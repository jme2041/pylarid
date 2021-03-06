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

/* datatype.c: Data types for the larid.Dset class */

#define NO_IMPORT_ARRAY
#include "larid.h"

/*****************************************************************************/

/* Lookup table for data types */

typedef const struct {
    const int16_t datatype;
    const int16_t bitpix;
    const int typenum;
    char const * const str;
} datatype_entry;

#define DATATYPE_TABLE_END -1

static datatype_entry datatype_table[] = {
    { DT_UINT8, sizeof(uint8_t)*CHAR_BIT, NPY_UINT8, "uint8"},
    { DT_INT16, sizeof(int16_t)*CHAR_BIT, NPY_INT16, "int16" },
    { DT_INT32, sizeof(int32_t)*CHAR_BIT, NPY_INT32, "int32" },
    { DT_FLOAT32, sizeof(float)*CHAR_BIT, NPY_FLOAT32, "float32" },
    { DT_FLOAT64, sizeof(double)*CHAR_BIT, NPY_FLOAT64, "float64" },
    { DATATYPE_TABLE_END, 0, 0, NULL }
};

/*****************************************************************************/

/* Convert a NIfTI data type code to a string */

char const *
datatype2str(int16_t const datatype)
{
    assert(!PyErr_Occurred());
    datatype_entry * p = datatype_table;
    while(p->datatype != DATATYPE_TABLE_END)
    {
        if(datatype == p->datatype) return p->str;
        ++p;
    }
    Py_UNREACHABLE();
}

/*****************************************************************************/

/* Get the number of bits per pixel for a NIfTI data type code */

int16_t
datatype2bitpix(int16_t const datatype)
{
    assert(!PyErr_Occurred());
    datatype_entry * p = datatype_table;
    while(p->datatype != DATATYPE_TABLE_END)
    {
        if(datatype == p->datatype) return p->bitpix;
        ++p;
    }
    Py_UNREACHABLE();
}

/*****************************************************************************/

/* Convert a PyObject to a NIfTI data type code. This function takes the form
 * of a ParseTuple converter for O& arguments. Thus, the function takes two
 * arguments (the Python object to be converted and the address at which to
 * store the result) and returns an integer (1 for success, 0 for failure).
 */

int
obj2datatype(PyObject * const restrict obj, int16_t * const restrict datatype)
{
    assert(obj);
    assert(datatype);
    assert(!PyErr_Occurred());

    *datatype = 0;

    if(!PyUnicode_Check(obj))
    {
        PyErr_SetString(PyExc_TypeError, "Data type must be a string");
        return 0;
    }

    datatype_entry * p = datatype_table;
    while(p->datatype != DATATYPE_TABLE_END)
    {
        if(!PyUnicode_CompareWithASCIIString(obj, p->str))
        {
            *datatype = p->datatype;
            return 1;
        }
        ++p;
    }

    PyErr_Format(PyExc_ValueError, "Invalid data type: %U", obj);
    return 0;
}

/*****************************************************************************/

/* Convert a NIfTI data type code to a NumPy type number. This function is
 * called for datatype codes coming in from NIfTI. Thus, it sets a Python
 * exception and returns zero for unsupported data types.
 */

int
datatype2typenum(int16_t const datatype)
{
    assert(!PyErr_Occurred());
    datatype_entry * p = datatype_table;
    while(p->datatype != DATATYPE_TABLE_END)
    {
        if(datatype == p->datatype) return p->typenum;
        ++p;
    }

    PyErr_Format(LaridError,
                 "Invalid or unsupported NIfTI data type code (%"PRId16")",
                 datatype);
    return 0;
}

/*****************************************************************************/

/* Public getter for larid.Dset.datatype */

PyObject *
Dset_get_datatype(DsetObject * self, void * Py_UNUSED(closure))
{
    assert(!PyErr_Occurred());
    return PyUnicode_FromString(datatype2str(self->hdr.datatype));
}

/*****************************************************************************/

/* Internal method for copying a larid.Dset object to a new larid.Dset object,
 * optionally changing the datatype. Upon success, returns a pointer to the new
 * larid.Dset object. Upon failure, sets a Python exception and returns NULL.
 * It is assumed that new_datatype has already been validated by the caller.
 */

PyObject *
Dset_internal_copy(
    DsetObject * const restrict self,
    int16_t const new_datatype)
{
    PyObject * data = NULL;
    DsetObject * ret = NULL;

    assert(!PyErr_Occurred());

    /* Copy the voxel data. This works even if the datatype is unchanged.
     * PyArray_Cast uses NPY_UNSAFE_CASTING.
     */
    data = PyArray_Cast((PyArrayObject *)self->data,
                        datatype2typenum(new_datatype));
    if(!data) goto except;

    /* Create a new Dset object */
    ret = (DsetObject *)Dset___new__(Py_TYPE(self), NULL, NULL);
    if(!ret) goto except;

    /* From here, the function succeeds. Copy Dset attributes to new_dset. */
    memcpy(&ret->hdr, &self->hdr, sizeof(ret->hdr));
    ret->morder = self->morder;
    ret->data = data;   /* Steal the reference to data */

    /* Set new datatype and bitpix */
    ret->hdr.datatype = new_datatype;
    ret->hdr.bitpix = datatype2bitpix(new_datatype);

    assert(!PyErr_Occurred());
    assert(ret);
    assert(ret->data);
    goto finally;

except:
    Py_XDECREF(data);
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;

finally:
    return (PyObject *)ret;
}

/*****************************************************************************/

/* larid.Dset.to_datatype */

char const Dset_to_datatype__doc__[] = PyDoc_STR(
    "to_datatype($self, /, datatype)\n"
    "--\n"
    "\n"
    "Return a new Dset object with the requested datatype. The new Dset \n"
    "object is created by copying the data from self to a new Dset object\n"
    "and converting it to the new datatype using a C-style cast. When\n"
    "changing from a larger datatype to a smaller datatype (e.g., 'int32'\n"
    "to 'int16'), data loss will occur if an existing data point falls\n"
    "outside of the range of values that can be stored by the new\n"
    "datatype. When casting from a floating-point value to an integer, the\n"
    "value is rounded toward 0. To safely downcast without losing data,\n"
    "consider using rescale() to rescale the data to a range supported by\n"
    "the smaller datatype before calling to_datatype().");

PyObject *
Dset_to_datatype(DsetObject * self, PyObject * arg)
{
    PyObject * ret = NULL;

    assert(!PyErr_Occurred());
    assert(arg);
    Py_INCREF(arg);

    int16_t new_datatype;
    if(!obj2datatype(arg, &new_datatype)) goto except;

    ret = Dset_internal_copy(self, new_datatype);
    if(!ret) goto except;

    assert(!PyErr_Occurred());
    assert(ret);
    goto finally;

except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;

finally:
    Py_DECREF(arg);
    return ret;
}

/*****************************************************************************/

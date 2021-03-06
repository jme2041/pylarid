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

/* rescale.c: larid.Dset.rescale method */

#define NO_IMPORT_ARRAY
#include "larid.h"

/*****************************************************************************/

char const Dset_rescale__doc__[] = PyDoc_STR(
    "rescale($self, /, new_min, new_max)\n"
    "--\n"
    "\n"
    "Rescale the voxel data to fit within the range [new_min, new_max].\n"
    "This does not change the NIfTI scl_slope or scl_inter attributes.");

PyObject *
Dset_rescale(DsetObject * self, PyObject * args, PyObject * kwds)
{
    static char const * const kwlist[] = { "new_min", "new_max", NULL };
    PyObject * ret = NULL;

    assert(!PyErr_Occurred());

    double new_min, new_max;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "dd", (char **)kwlist,
                                    &new_min,  &new_max)) goto except;

    if(new_max <= new_min)
    {
        PyErr_SetString(PyExc_ValueError, "new_min must be less than new_max");
        goto except;
    }

    /* Get the minimum and maximum value of the existing dataset */
    double old_min = 0.0, old_max = 0.0, val;
    npy_intp const n = PyArray_SIZE((PyArrayObject *)self->data);
    void * const pdata = PyArray_DATA((PyArrayObject *)self->data);
    int const typenum = PyArray_TYPE((PyArrayObject *)self->data);
    switch(typenum)
    {
        case NPY_UINT8:
            for(npy_intp i = 0; i < n; ++i)
            {
                val = (double)(((uint8_t *)pdata)[i]);
                if(val < old_min) old_min = val;
                if(val > old_max) old_max = val;
            }
            break;
        case NPY_INT16:
            for(npy_intp i = 0; i < n; ++i)
            {
                val = (double)(((int16_t *)pdata)[i]);
                if(val < old_min) old_min = val;
                if(val > old_max) old_max = val;
            }
            break;
        case NPY_INT32:
            for(npy_intp i = 0; i < n; ++i)
            {
                val = (double)(((int32_t *)pdata)[i]);
                if(val < old_min) old_min = val;
                if(val > old_max) old_max = val;
            }
            break;
        case NPY_FLOAT32:
            for(npy_intp i = 0; i < n; ++i)
            {
                val = (double)(((float *)pdata)[i]);
                if(val < old_min) old_min = val;
                if(val > old_max) old_max = val;
            }
            break;
        case NPY_FLOAT64:
            for(npy_intp i = 0; i < n; ++i)
            {
                val = ((double *)pdata)[i];
                if(val < old_min) old_min = val;
                if(val > old_max) old_max = val;
            }
            break;
        default:
            Py_UNREACHABLE();
    }

    /* Compute scale factor */
    double scale;
    if(larid_close(old_max, old_min)) scale = 1.0;
    else scale = (new_max - new_min)/(old_max - old_min);

    /* Scale the dataset */
    double new;
    switch(typenum)
    {
        case NPY_UINT8:
            for(npy_intp i = 0; i < n; ++i)
            {
                val = (double)(((uint8_t *)pdata)[i]);
                new = new_min + scale * (val - old_min);
                new = round(new);

                uint8_t inew;
                if(new < 0.0) inew = (uint8_t)0;
                else if(new > (double)UINT8_MAX) inew = (uint8_t)UINT8_MAX;
                else inew = (uint8_t)new;

                ((uint8_t *)pdata)[i] = inew;
            }
            break;
        case NPY_INT16:
            for(npy_intp i = 0; i < n; ++i)
            {
                val = (double)(((int16_t *)pdata)[i]);
                new = new_min + scale * (val - old_min);
                new = round(new);

                int16_t inew;
                if(new < (double)INT16_MIN) inew = (int16_t)INT16_MIN;
                else if(new > (double)INT16_MAX) inew = (int16_t)INT16_MAX;
                else inew = (int16_t)new;

                ((int16_t *)pdata)[i] = inew;
            }
            break;
        case NPY_INT32:
            for(npy_intp i = 0; i < n; ++i)
            {
                val = (double)(((int32_t *)pdata)[i]);
                new = new_min + scale * (val - old_min);
                new = round(new);

                int32_t inew;
                if(new < (double)INT32_MIN) inew = (int32_t)INT32_MIN;
                else if(new > (double)INT32_MAX) inew = (int32_t)INT32_MAX;
                else inew = (int32_t)new;

                ((int32_t *)pdata)[i] = inew;
            }
            break;
        case NPY_FLOAT32:
            for(npy_intp i = 0; i < n; ++i)
            {
                val = (double)(((float *)pdata)[i]);
                new = new_min + scale * (val - old_min);
                ((float *)pdata)[i] = (float)new;
            }
            break;
        case NPY_FLOAT64:
            for(npy_intp i = 0; i < n; ++i)
            {
                val = ((double *)pdata)[i];
                new = new_min + scale * (val - old_min);
                ((double *)pdata)[i] = new;
            }
            break;
        default:
            Py_UNREACHABLE();
    }

    /* Success */
    assert(!PyErr_Occurred());
    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;

except:
    assert(PyErr_Occurred());
    ret = NULL;

finally:
    return ret;
}

/*****************************************************************************/

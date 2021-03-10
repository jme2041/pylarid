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

/* dset.c: Main file for the larid.Dset class */

#define NO_IMPORT_ARRAY
#include "larid.h"
#include <structmember.h>

/*****************************************************************************/

PyDoc_STRVAR(
    Dset__doc__,
    "Dset(ni, nj, nk, nt, datatype, morder)\n"
    "--\n"
    "\n"
    "Access an MRI/fMRI dataset as a NumPy array\n"
    "\n"
    "  ni\n"
    "    Number of columns\n"
    "  nj\n"
    "    Number of rows\n"
    "  nk\n"
    "    Number of slices\n"
    "  nt\n"
    "    Number of values per voxel (frames)\n"
    "  datatype\n"
    "    Data type: 'uint8', 'int16', 'int32', 'float32', or 'float64'\n"
    "  morder\n"
    "    Memory order: 'tkji' or 'kjit'\n"
    "\n"
    "A Dset object provides access to an MRI/fMRI dataset by storing it in\n"
    "memory in a NumPy array. Datasets are 4 dimensional. There are three\n"
    "spatial dimensions: columns from 0 to ni-1, rows from 0 to nj-1, and\n"
    "slices 0 to nk-1. The fourth dimension is used to index multiple\n"
    "values per voxel (frames) from 0 to nt-1. These values are typically\n"
    "time points (often referred to as volumes) or vectors of statistics.\n"
    "\n"
    "Data can be stored using different data types, referred to by larid\n"
    "as 'datatype's. The following data types are supported:\n"
    "\n"
    "  uint8\n"
    "    Unsigned 8-bit integers\n"
    "  int16\n"
    "    Signed 16-bit integers\n"
    "  int32\n"
    "    Signed 32-bit integers\n"
    "  float32\n"
    "    IEEE 754 single-precision floating-point values\n"
    "  float64\n"
    "    IEEE 754 double-precision floating-point values\n"
    "\n"
    "In program memory, data can be stored in different orders. These\n"
    "orders are referred to as 'morder' in larid functions. 'morder' is\n"
    "short for 'memory order'. Memory order is specified using character\n"
    "strings in which the leftmost character specifies the outermost loop\n"
    "and the rightmost character specifies the innermost loop. Different\n"
    "memory orders are optimal for different analyses.\n"
    "\n"
    "larid supports the following memory orders:\n"
    "\n"
    "  tkji\n"
    "    Columns are stored contiguously and vary the fastest, followed by\n"
    "    rows, slices, and frames. Data are typically described as an\n"
    "    array of three-dimensional volumes. This is the storage order\n"
    "    used by NIfTI files. 'tkji' order is useful for space-dependent\n"
    "    operations such as smoothing. When accessing a Dset object's\n"
    "    voxel data for a 'tkji' dataset, the array is indexed as\n"
    "    data[t][k][j][i], where:\n"
    "      t indexes frame, from 0 to nt-1\n"
    "      k indexes slice, from 0 to nk-1\n"
    "      j indexes row, from 0 to nj-1\n"
    "      i indexes column, from 0 to ni-1\n"
    "  kjit\n"
    "    Frames (e.g., time points, statistics) are stored contiguously\n"
    "    and vary the fastest, followed by columns, rows, and slices. Data\n"
    "    are typically described as a single three-dimensional volume with\n"
    "    a vector of multiple values stored at each voxel. This order\n"
    "    reduces cache-misses when conducting voxelwise (i.e., voxel-by-\n"
    "    voxel) analysis of a dataset (e.g., general linear models of time\n"
    "    series data, higher-order analysis of data in standard \n"
    "    stereotaxic space). When accessing a Dset object's voxel data for\n"
    "    a 'kjit' dataset, the array is indexed as data[k][j][i][t],\n"
    "    where:\n"
    "      k indexes slice, from 0 to nk-1\n"
    "      j indexes row, from 0 to nj-1\n"
    "      i indexes column, from 0 to ni-1\n"
    "      t indexes frame, from 0 to nt-1\n"
    "\n"
    "Dataset dimensions (ni, nj, nk, nt) and datatype are immutable.\n"
    "\n"
    "Memory order can be changed by setting the morder attribute.");

/*****************************************************************************/

/* Traversal function for the garbage collector */

static int
Dset_traverse(DsetObject * self, visitproc visit, void * arg)
{
    Py_VISIT(self->data);
    return 0;
}

/* Clear function for the garbage collector */

static int
Dset_clear(DsetObject * self)
{
    Py_CLEAR(self->data);
    return 0;
}

/* Instance destructor */

static void
Dset_dealloc(DsetObject * self)
{
    PyObject_GC_UnTrack(self);
    Dset_clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

/*****************************************************************************/

/* Return dataset dimensions */

static PyObject *
Dset_get_ni(DsetObject * self, void * Py_UNUSED(closure))
{
    assert(!PyErr_Occurred());
    return PyLong_FromLongLong(DSET_NI(self));
}

static PyObject *
Dset_get_nj(DsetObject * self, void * Py_UNUSED(closure))
{
    assert(!PyErr_Occurred());
    return PyLong_FromLongLong(DSET_NJ(self));
}

static PyObject *
Dset_get_nk(DsetObject * self, void * Py_UNUSED(closure))
{
    assert(!PyErr_Occurred());
    return PyLong_FromLongLong(DSET_NK(self));
}

static PyObject *
Dset_get_nt(DsetObject * self, void * Py_UNUSED(closure))
{
    assert(!PyErr_Occurred());
    return PyLong_FromLongLong(DSET_NT(self));
}

/*****************************************************************************/

/* Return a string representation of a Dset object */

static PyObject *
Dset___str__(DsetObject * self)
{
    char * buf = NULL;
    PyObject * ret = NULL;

    assert(!PyErr_Occurred());

    larid_asprintf(
        &buf,
        "<%s object at %p>\n"
        "data at  : %p\n"
        "ni       : %"PRId64"\n"
        "nj       : %"PRId64"\n"
        "nk       : %"PRId64"\n"
        "nt       : %"PRId64"\n"
        "datatype : %s (NIfTI = %"PRId16", NumPy = %"PRId32")\n"
        "morder   : %s (%"PRId32")\n"
        "- NIfTI-2 Fields "
        "--------------------------------------------------------------\n"
        "datatype       : %"PRId16"\n"
        "bitpix         : %"PRId16"\n"
        "dim[0]         : %"PRId64"\n"
        "dim[1]         : %"PRId64"\n"
        "dim[2]         : %"PRId64"\n"
        "dim[3]         : %"PRId64"\n"
        "dim[4]         : %"PRId64"\n"
        "dim[5]         : %"PRId64"\n"
        "dim[6]         : %"PRId64"\n"
        "dim[7]         : %"PRId64"\n"
        "intent_p1      : %8.6f\n"
        "intent_p2      : %8.6f\n"
        "intent_p3      : %8.6f\n"
        "pixdim[0]      : %8.6f\n"
        "pixdim[1]      : %8.6f\n"
        "pixdim[2]      : %8.6f\n"
        "pixdim[3]      : %8.6f\n"
        "pixdim[4]      : %8.6f\n"
        "pixdim[5]      : %8.6f\n"
        "pixdim[6]      : %8.6f\n"
        "pixdim[7]      : %8.6f\n"
        "scl_slope      : %8.6f\n"
        "scl_inter      : %8.6f\n"
        "cal_max        : %8.6f\n"
        "cal_min        : %8.6f\n"
        "slice_duration : %8.6f\n"
        "toffset        : %8.6f\n"
        "slice_start    : %"PRId64"\n"
        "slice_end      : %"PRId64"\n"
        "descrip        : %s\n"
        "aux_file       : %s\n"
        "qform_code     : %"PRId32"\n"
        "sform_code     : %"PRId32"\n"
        "quatern_b      : %8.6f\n"
        "quatern_c      : %8.6f\n"
        "quatern_d      : %8.6f\n"
        "qoffset_x      : %8.6f\n"
        "qoffset_y      : %8.6f\n"
        "qoffset_z      : %8.6f\n"
        "srow_x         : %8.6f %8.6f %8.6f %8.6f\n"
        "srow_y         : %8.6f %8.6f %8.6f %8.6f\n"
        "srow_z         : %8.6f %8.6f %8.6f %8.6f\n"
        "slice_code     : %"PRId32"\n"
        "xyzt_units     : %"PRId32"\n"
        "intent_code    : %"PRId32"\n"
        "intent_name    : %s\n"
        "dim_info       : %"PRId8"\n"
        "-----------------"
        "--------------------------------------------------------------",
        Py_TYPE(self)->tp_name, self,
        PyArray_DATA((PyArrayObject *)self->data),
        DSET_NI(self),
        DSET_NJ(self),
        DSET_NK(self),
        DSET_NT(self),
        datatype2str(self->hdr.datatype),
        self->hdr.datatype,
        PyArray_TYPE((PyArrayObject *)self->data),
        morder2str(self->morder),
        (int32_t)self->morder,
        self->hdr.datatype,
        self->hdr.bitpix,
        self->hdr.dim[0],
        self->hdr.dim[1],
        self->hdr.dim[2],
        self->hdr.dim[3],
        self->hdr.dim[4],
        self->hdr.dim[5],
        self->hdr.dim[6],
        self->hdr.dim[7],
        self->hdr.intent_p1,
        self->hdr.intent_p2,
        self->hdr.intent_p3,
        self->hdr.pixdim[0],
        self->hdr.pixdim[1],
        self->hdr.pixdim[2],
        self->hdr.pixdim[3],
        self->hdr.pixdim[4],
        self->hdr.pixdim[5],
        self->hdr.pixdim[6],
        self->hdr.pixdim[7],
        self->hdr.scl_slope,
        self->hdr.scl_inter,
        self->hdr.cal_max,
        self->hdr.cal_min,
        self->hdr.slice_duration,
        self->hdr.toffset,
        self->hdr.slice_start,
        self->hdr.slice_end,
        self->hdr.descrip,
        self->hdr.aux_file,
        self->hdr.qform_code,
        self->hdr.sform_code,
        self->hdr.quatern_b,
        self->hdr.quatern_c,
        self->hdr.quatern_d,
        self->hdr.qoffset_x,
        self->hdr.qoffset_y,
        self->hdr.qoffset_z,
        self->hdr.srow_x[0],
        self->hdr.srow_x[1],
        self->hdr.srow_x[2],
        self->hdr.srow_x[3],
        self->hdr.srow_y[0],
        self->hdr.srow_y[1],
        self->hdr.srow_y[2],
        self->hdr.srow_y[3],
        self->hdr.srow_z[0],
        self->hdr.srow_z[1],
        self->hdr.srow_z[2],
        self->hdr.srow_z[3],
        self->hdr.slice_code,
        self->hdr.xyzt_units,
        self->hdr.intent_code,
        self->hdr.intent_name,
        self->hdr.dim_info);

    if(!buf) goto except;
    ret = PyUnicode_FromString(buf);
    if(!ret) goto except;

    assert(!PyErr_Occurred());
    assert(ret);
    goto finally;

except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;

finally:
    PyMem_Free(buf);
    return ret;
}

/*****************************************************************************/

/* larid.Dset.__new__ */

PyObject *
Dset___new__(
    PyTypeObject * type,
    PyObject * Py_UNUSED(args),
    PyObject * Py_UNUSED(kwds))
{
    assert(!PyErr_Occurred());
    DsetObject * self = (DsetObject *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

/*****************************************************************************/

/* larid.Dset.__init__ */

static int
Dset___init__(DsetObject * self, PyObject * args, PyObject * kwds)
{
    PyObject * data = NULL;
    int ret;

    assert(!PyErr_Occurred());

    static char const * const kwlist[] =
        { "ni", "nj", "nk", "nt", "datatype", "morder", NULL };

    int64_t ni, nj, nk, nt;
    int16_t datatype;
    larid_morder morder;

    if(!PyArg_ParseTupleAndKeywords(args, kwds,
                                    "LLLLO&O&:Dset", (char **)kwlist,
                                    &ni, &nj, &nk, &nt,
                                    obj2datatype, &datatype,
                                    obj2morder, &morder)) goto except;

    /* Check dimensions */
    if(ni < 1 || nj < 1 || nk < 1 || nt < 1)
    {
        PyErr_SetString(PyExc_ValueError,
                        "Dataset dimensions must be greater than zero");
        goto except;
    }

    /* Get NumPy type number */
    int typenum = datatype2typenum(datatype);
    assert(typenum);

    /* Set NumPy array dimensions according to morder */
    npy_intp dim[4];
    switch(morder)
    {
        case LARID_MORDER_TKJI:
            DIM_TKJI(dim, ni, nj, nk, nt);
            break;
        case LARID_MORDER_KJIT:
            DIM_KJIT(dim, ni, nj, nk, nt);
            break;
        default:
            Py_UNREACHABLE();
    }

    /* Create the NumPy array (initialize the data block to zero) */
    data = PyArray_ZEROS(4, dim, typenum, 0);
    if(!data) goto except;

    /* From here, the function succeeds */

    /* Store the new NumPy array and release the old NumPy array */
    PyObject * tmp = self->data;
    self->data = data;
    Py_XDECREF(tmp);

    /* Set minimal NIfTI-2 elements */
    memset(&self->hdr, 0, sizeof(self->hdr));
    self->hdr.sizeof_hdr = sizeof(self->hdr);
    self->hdr.magic[0] = 0x6E;
    self->hdr.magic[2] = 0x32;
    self->hdr.magic[4] = 0x0D;
    self->hdr.magic[5] = 0x0A;
    self->hdr.magic[6] = 0x1A;
    self->hdr.magic[7] = 0x0A;
    self->hdr.datatype = datatype;
    self->hdr.bitpix = datatype2bitpix(datatype);
    self->hdr.dim[0] = (nt == 1 ? 3 : 4);
    self->hdr.dim[1] = ni;
    self->hdr.dim[2] = nj;
    self->hdr.dim[3] = nk;
    self->hdr.dim[4] = nt;
    self->hdr.pixdim[1] = 1.0;
    self->hdr.pixdim[2] = 1.0;
    self->hdr.pixdim[3] = 1.0;
    self->hdr.xyzt_units = NIFTI_UNITS_MM;
    if(self->hdr.dim[4] > 1)
    {
        self->hdr.pixdim[4] = 1.0;
        self->hdr.xyzt_units |= NIFTI_UNITS_SEC;
    }

    /* Set morder code */
    self->morder = morder;

    assert(!PyErr_Occurred());
    ret = 0;
    goto finally;

except:
    Py_XDECREF(data);
    assert(PyErr_Occurred());
    ret = -1;

finally:
    return ret;
}

/*****************************************************************************/

/* larid.Dset.copy */

PyDoc_STRVAR(
    Dset_copy__doc__,
    "copy($self, /)\n"
    "--\n"
    "\n"
    "Return a new Dset object that is a copy of self. The copy is a deep\n"
    "copy; all attributes are copied and a new voxel data array is created\n"
    "and used for the new Dset object.");

static PyObject *
Dset_copy(DsetObject * self, PyObject * Py_UNUSED(arg))
{
    return Dset_internal_copy(self, self->hdr.datatype);
}

/*****************************************************************************/

/* Table of members (properties) for larid.Dset */

static PyMemberDef Dset_members[] = {
    { "data", T_OBJECT, offsetof(DsetObject, data), READONLY,
        PyDoc_STR("Access the voxel data as a 4-dimensional NumPy array") },
    { NULL, 0, 0, 0, NULL }
};

/* Table of accessors (getters and setters) for larid.Dset */

static PyGetSetDef Dset_getsetters[] = {
    { "ni", (getter)Dset_get_ni, NULL, PyDoc_STR("Number of columns"), NULL },
    { "nj", (getter)Dset_get_nj, NULL, PyDoc_STR("Number of rows"), NULL },
    { "nk", (getter)Dset_get_nk, NULL, PyDoc_STR("Number of slices"), NULL },
    { "nt", (getter)Dset_get_nt, NULL, PyDoc_STR("Number of frames"), NULL },
    { "datatype", (getter)Dset_get_datatype, NULL,
        PyDoc_STR("Data type"), NULL },
    { "morder", (getter)Dset_get_morder, (setter)Dset_set_morder,
        PyDoc_STR("Memory order"), NULL },
    { NULL, NULL, NULL, NULL, NULL }
};

/* Table of methods for larid.Dset */

static PyMethodDef Dset_methods[] = {
    { "like_nifti", (PyCFunction)Dset_like_nifti,
        METH_CLASS | METH_VARARGS |  METH_KEYWORDS, Dset_like_nifti__doc__ },
    { "from_nifti", (PyCFunction)Dset_from_nifti,
        METH_CLASS | METH_O, Dset_from_nifti__doc__ },
    { "to_nifti", (PyCFunction)Dset_to_nifti,
        METH_VARARGS | METH_KEYWORDS, Dset_to_nifti__doc__ },
    { "copy", (PyCFunction)Dset_copy,
        METH_NOARGS, Dset_copy__doc__ },
    { "to_datatype", (PyCFunction)Dset_to_datatype,
        METH_O, Dset_to_datatype__doc__ },
    { "rescale", (PyCFunction)Dset_rescale,
        METH_VARARGS | METH_KEYWORDS, Dset_rescale__doc__ },
    { NULL, NULL, 0, NULL }
};

/* Type object for larid.Dset (determines larid.Dset behavior) */

PyTypeObject DsetType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name            = "larid.Dset",
    .tp_doc             = Dset__doc__,
    .tp_basicsize       = sizeof(DsetObject),
    .tp_itemsize        = 0,
    .tp_flags           = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_new             = Dset___new__,
    .tp_init            = (initproc)Dset___init__,
    .tp_dealloc         = (destructor)Dset_dealloc,
    .tp_traverse        = (traverseproc)Dset_traverse,
    .tp_clear           = (inquiry)Dset_clear,
    .tp_str             = (reprfunc)Dset___str__,
    .tp_members         = Dset_members,
    .tp_getset          = Dset_getsetters,
    .tp_methods         = Dset_methods
};

/*****************************************************************************/

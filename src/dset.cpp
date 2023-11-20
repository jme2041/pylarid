// pylarid: Load Arrays of Imaging Data
// Copyright 2023 Jeffrey Michael Engelmann

#include "dset.h"

///////////////////////////////////////////////////////////////////////////////
//
// Documentation string
//

PyDoc_STRVAR(Dset_doc,
    "Dset()\n"
    "--\n"
    "\n"
    "Access an MRI dataset.");

///////////////////////////////////////////////////////////////////////////////
//
// pylarid.Dset.__new__
//

PyObject*
Dset_new(PyTypeObject* const type,
         PyObject* const args,
         PyObject*)
{
    assert(!PyErr_Occurred());

    if (!PyArg_ParseTuple(args, ":Dset"))
    {
        assert(PyErr_Occurred());
        return nullptr;
    }

    assert(!PyErr_Occurred());
    return type->tp_alloc(type, 0);
}

///////////////////////////////////////////////////////////////////////////////
//
// pylarid.Dset.__init__
//

static int
Dset_init(
    DsetObject*,
    PyObject* const args,
    PyObject*)
{
    assert(!PyErr_Occurred());

    if (!PyArg_ParseTuple(args, ":Dset"))
    {
        assert(PyErr_Occurred());
        return -1;
    }

    assert(!PyErr_Occurred());
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// Instance destructor
//

static void
Dset_dealloc(DsetObject* self)
{
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

///////////////////////////////////////////////////////////////////////////////
//
// Type object
//

PyTypeObject DsetType = {
    .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
    .tp_name = "pylarid.Dset",
    .tp_basicsize = sizeof(DsetObject),
    .tp_itemsize = 0,
    .tp_dealloc = reinterpret_cast<destructor>(Dset_dealloc),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = Dset_doc,
    .tp_init = reinterpret_cast<initproc>(Dset_init),
    .tp_new = Dset_new,
};

///////////////////////////////////////////////////////////////////////////////

// pylarid: Load Arrays of Imaging Data
// Copyright 2023 Jeffrey Michael Engelmann

#pragma once
#ifndef PYLARID_H
#define PYLARID_H

#include <Python.h>
#include <memory>

struct PyObjectDeleter {
    typedef PyObject* pointer;
    void operator()(PyObject* const p) { Py_DECREF(p); }
};

typedef std::unique_ptr<PyObject*, PyObjectDeleter> PythonObject;

#endif  // PYLARID_H

///////////////////////////////////////////////////////////////////////////////

// pylarid: Load Arrays of Imaging Data
// Copyright 2023 Jeffrey Michael Engelmann

#pragma once
#ifndef DSET_H_INCLUDED
#define DSET_H_INCLUDED

#include "pylarid.h"

typedef struct {
    PyObject_HEAD
} DsetObject;

extern PyTypeObject DsetType;

#endif

///////////////////////////////////////////////////////////////////////////////

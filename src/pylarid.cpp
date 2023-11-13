// pylarid: Load Arrays of Imaging Data
// Copyright 2023 Jeffrey Michael Engelmann

#include <Python.h>

PyDoc_STRVAR(pylarid__doc__,
    "Load Arrays of Imaging Data\n"
    "\n"
    "pylarid is intended to facilitate analysis of magnetic resonance\n"
    "imaging (MRI) data.\n"
    "\n"
    "pylarid is for research and educational purposes only. Clinical\n"
    "applications are not recommended or advised. pylarid has not been\n"
    "evaluated by the United States Food and Drug Administration (FDA), or\n"
    "by any other agency. pylarid is not intended to diagnose, treat, cure,\n"
    "or prevent any disease.\n"
    "\n"
    "Copyright 2023 Jeffrey Michael Engelmann\n"
    "\n"
    "Permission is hereby granted, free of charge, to any person obtaining\n"
    "a copy of this software and associated documentation files (the\n"
    "\"Software\"), to deal in the Software without restriction, including\n"
    "without limitation the rights to use, copy, modify, merge, publish,\n"
    "distribute, sublicense, and/or sell copies of the Software, and to\n"
    "permit persons to whom the Software is furnished to do so, subject to\n"
    "the following conditions:\n"
    "\n"
    "The above copyright notice and this permission notice shall be\n"
    "included in all copies or substantial portions of the Software.\n"
    "\n"
    "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
    "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
    "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n"
    "NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE\n"
    "LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION\n"
    "OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION\n"
    "WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.");

static PyModuleDef pylarid_module = {
    .m_name = "pylarid",
    .m_doc = pylarid__doc__,
    .m_size = -1
};

PyMODINIT_FUNC
PyInit_pylarid() {

    // Create the extension module object
    auto const m = PyModule_Create(&pylarid_module);

    // Add the package version string as a global
    if (PyModule_AddStringConstant(m, "__version__", PYLARID_VER)) {
        Py_DECREF(m);
        return nullptr;
    }

    assert(!PyErr_Occurred());
    return m;
}

///////////////////////////////////////////////////////////////////////////////

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

/* run.c: Run a function in a Python module using embedded Python */

#include <Python.h>

int main(int argc, char ** argv)
{
    if(argc < 5)
    {
        fprintf(
            stderr,
            "Usage: run ext_path test_path test_module test_function [...]\n"
            "ext_path     : Path to the directory containing the extension module.\n"
            "test_path    : Path to the directory containing test_module\n"
            "test_module  : Name of the module containing the test\n"
            "test_function: Function in test_module to call\n"
            "...          : Remaining arguments are passed to test_function as positional\n"
            "               parameters (formatted as strings)\n");
        return 1;
    }

    PyObject * sys_path = NULL;
    PyObject * ext_path = NULL;
    PyObject * test_path = NULL;
    PyObject * test_module = NULL;
    PyObject * test_function = NULL;
    PyObject * test_args = NULL;
    PyObject * result = NULL;
    int ret;

    wchar_t * name = Py_DecodeLocale(argv[0], NULL);
    if(!name)
    {
        fprintf(stderr, "Error: Could not decode argv[0]\n");
        return 1;
    }

    Py_SetProgramName(name);
    Py_Initialize();

    if(!(sys_path = PySys_GetObject("path"))) goto except;
    Py_INCREF(sys_path);    /* sys_path is a borrowed reference */

    /* Insert test module path (this becomes second in line) */
    if(!(test_path = PyUnicode_FromString(argv[2]))) goto except;
    if(PyList_Insert(sys_path, 0, test_path) == -1) goto except;

    /* Insert extension module path (this goes at the front) */
    if(!(ext_path = PyUnicode_FromString(argv[1]))) goto except;
    if(PyList_Insert(sys_path, 0, ext_path) == -1) goto except;

    /* Set sys.path (this does not steal a reference to sys_path) */
    if(PySys_SetObject("path", sys_path) == -1) goto except;

    /* Open the test module */
    if(!(test_module = PyImport_ImportModule(argv[3]))) goto except;

    /* Find the test function */
    if(!(test_function = PyObject_GetAttrString(test_module, argv[4])))
        goto except;
    if(!PyCallable_Check(test_function))
    {
        PyErr_Format(PyExc_RuntimeError,
                     "Function '%s' is not callable",
                     argv[4]);
        goto except;
    }

    /* Put remaining arguments into a tuple of PyUnicode objects */
    for(int i = 5; i < argc; ++i)
    {
        if(!test_args && !(test_args = PyTuple_New(argc - 5))) goto except;
        PyObject * arg = PyUnicode_FromString(argv[i]);
        if(!arg) goto except;
        /* This steals the reference to arg whether it succeeds or fails */
        if(PyTuple_SetItem(test_args, i - 5, arg) == -1) goto except;
    }

    /* Call the test function */
    if(!(result = PyObject_CallObject(test_function, test_args))) goto except;

    /* Success */
    assert(!PyErr_Occurred());
    ret = 0;
    goto finally;

except:
    assert(PyErr_Occurred());
    PyErr_Print();
    ret = 1;

finally:
    Py_XDECREF(sys_path);
    Py_XDECREF(ext_path);
    Py_XDECREF(test_path);
    Py_XDECREF(test_module);
    Py_XDECREF(test_function);
    Py_XDECREF(test_args);
    Py_XDECREF(result);
    if(Py_FinalizeEx() < 0)
    {
        PyErr_Print();
        ret = 1;
    }
    PyMem_RawFree(name);
    return ret;
}

/*****************************************************************************/

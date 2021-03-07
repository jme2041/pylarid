pylarid: Load Arrays of Imaging Data
====================================

``pylarid`` is a Python package that is intended to facilitate analysis of
functional magnetic resonance imaging (fMRI) datasets. ``pylarid`` provides a
class for loading datasets as NumPy arrays.

Run-time Dependencies
=====================

Running ``pylarid`` requires CPython_ 3.9.2 or newer and NumPy_ 1.20.1 or
newer.

.. _CPython: https://www.python.org
.. _NumPy: https://numpy.org

Building pylarid
================

To build and install ``pylarid`` from source, CMake_ and a C compiler that
supports C11 are required. The ``nifti1.h`` and ``nifti2.h`` header files are
also required. These headers provide the NIfTI-1.1 and NIfTI-2 file format
specifications. ``nifti1.h`` and ``nifti2.h`` are available from the
`Neuroimaging Informatics Technology Initiative`__. To specify where
``nifti1.h`` and ``nifti2.h`` are located on the build system, use the CMake
options ``-DNIFTI1_DIR=`` and ``-DNIFTI2_DIR=`` or set the environment
variables ``NIFTI1_DIR`` and ``NIFTI2_DIR``.

.. _CMake: https://cmake.org
.. _NIfTI: https://nifti.nimh.nih.gov
__ NIfTI_

There are two methods for building and installing ``pylarid`` from source:
setuptools and manual build.

Build using setuptools (recommended)
------------------------------------

The recommended method for building and installing ``pylarid`` from source uses
setuptools. This method is recommended because installation of official
releases via ``pip`` will use setuptools.

To install build-time and run-time dependencies from PyPI, use the following
(preferably in a Python virtual environment)::

    pip install --upgrade pip setuptools wheel numpy

If CMake is not already installed, the PyPI version can be used::

    pip install cmake

If the NIfTI headers are not already available, they can be downloaded using
the following. In this example, they are downloaded to ``$HOME/include``::

    cd $HOME/include
    curl -O https://nifti.nimh.nih.gov/pub/dist/src/niftilib/nifti1.h
    curl -O https://nifti.nimh.nih.gov/pub/dist/doc/nifti2.h

Add the location of ``nifti1.h`` and ``nifti2.h`` as the ``NIFTI1_DIR`` and
``NIFTI2_DIR`` environment variables, respectively (in this example,
``$HOME/include`` for both). The method for setting environment variables
differs across operating systems and shells. If you are using ``bash``, the
environment variables can be set by adding the following to ``$HOME/.bashrc``::

    export NIFTI1_DIR=$HOME/include
    export NIFTI2_DIR=$HOME/include

To build ``pylarid`` from the latest source code, clone the Github repository::

    git clone git@github.com:jme2041/pylarid.git

Within the ``pylarid`` directory, run the following to build and install in
development mode::

    python setup.py develop

The default build is a release build. For a debug build, use::

    python setup.py develop --debug

Arguments that start with ``-D`` are passed to CMake. As an alternative to
setting environment variables, the locations of ``nifti1.h`` and ``nifti2.h``
can be specified using CMake options::

    python setup.py develop -DNIFTI1_DIR=$HOME/include -DNIFTI2_DIR=$HOME/include

Another alternative is to specify environment variables on the command line::

    NIFTI1_DIR=$HOME/include NIFTI2_DIR=$HOME/include python setup.py develop

To uninstall, use::

    pip uninstall pylarid

The ``test`` folder contains tests. To run individual categories of unit test,
specify the name of the test as an argument to ``python -m unittest``. For
example::

    python -m unittest test.test_larid --verbose
    python -m unittest test.test_dset --verbose
    python -m unittest test.test_morder --verbose
    python -m unittest test.test_cast --verbose
    python -m unittest test.test_rescale --verbose

To run all tests, use::

    python -m unittest --verbose

Manual build using CMake (advanced)
-----------------------------------

To build and install ``pylarid`` directly using CMake (without setuptools),
create a build directory within the ``pylarid`` directory and run ``cmake``,
``make``, and ``make install``::

    mkdir build
    cd build
    cmake ..
    make
    make install

To uninstall, use::

    make uninstall

Debugging pylarid
=================

To debug ``pylarid`` with a debugger, use a manual CMake build with the CMake
options ``-DCMAKE_BUILD_TYPE=Debug`` and ``-DPYLARID_TEST_RUNNER=ON``. The
former specifies a debug build and the latter instructs CMake to build the
``pylarid`` test script runner. The test runner can be run from the ``build``
directory::

    ./run ext_path test_path test_module test_function [...]

``ext_path`` is the path to the directory containing the debug build of the
extension module, ``test_path`` is the path to the directory containing the
test module, ``test_module`` is the name of a Python file containing test code,
and ``test_function`` is the name of the function in ``test_module`` that the
test runner will call. To debug ``pylarid``, insert breakpoints into the
``pylarid`` source code and start ``run`` in a debugger.

This example builds the test runner and runs the function ``debug_new_dset`` in
``debug/debug_dset.py``. The arguments after ``debug_new_dset`` are passed to
``debug_new_dset`` as positional parameters (formatted as strings)::

    cd build
    cmake -DPYLARID_BUILD_TYPE=Debug -DPYLARID_TEST_RUNNER=ON ..
    make
    lldb run . ../debug debug_dset debug_new_dset 128 128 80 300 int16 tkji

This example uses the ``lldb`` debugger; to use other debuggers, modify the
last line of code accordingly. It is also possible to start ``run`` in a GUI
debugger and step through the ``pylarid`` extension module code.

For the ``lldb`` example, the following sets a breakpoint at the entry to the
``Dset___init__`` function in the ``larid`` extension module. The warning is
normal; the breakpoint in the shared library will not be resolved until ``run``
imports the python module (``debug_dset.py``), which in turn imports the
extension module (via ``import larid``). Use ``r`` to run the program to its
first breakpoint. Note that the suffix (``cpython-39-darwin.so``) will differ
between systems::

    lldb ./run . ../debug debug_dset debug_new_dset 128 128 80 300 int16 tkji
    (lldb) break set -r Dset___init__ -s larid.cpython-39-darwin.so
    Breakpoint 1: no locations (pending).
    WARNING:  Unable to resolve breakpoint to any actual locations.
    (lldb) r
    ...

License
=======

Copyright (c) 2021, Jeffrey M. Engelmann

``pylarid`` is released under the revised (3-clause) BSD license.
For details, see LICENSE.txt.

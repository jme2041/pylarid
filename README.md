# pylarid: Load Arrays of Imaging Data

`pylarid` is a Python package that is intended to facilitate analysis of
magnetic resonance imaging (MRI) data.

# Building Pylarid

`pylarid` consists of an extension module built in C++ to run on Python 3.
`pylarid` is currently tested using Python 3.12 on macOS Sonoma using Clang
and on Windows 11 using Visual Studio, and using Python 3.11 on Arch Linux
using GCC.

To build and install `pylarid` from source, [CMake](https://cmake.org) and a
compiler that supports C++20 are required.

## Build using PyPA's build (Recommended)

Preferably, this is done in a virtual environment. If using Visual Studio, use
a 64-bit native tools developer command prompt. First, install the most
up-to-date versions of `pip` and `pipx` in the virtual environment.

```
python -m pip install --upgrade pip
pip install --upgrade pipx
```

Next, obtain the [pylarid](https://github.com/jme2041/pylarid.git) source code,
build, and install.

```
git clone git@github.com:jme2041/pylarid.git
cd pylarid
pipx run build --wheel -Cbuild-dir=build
pip install .
```

Note that `-Cbuild-dir=build` sets the temporary directory to `build`. This is
optional, but recommended for speeding up builds and for avoiding warnings
about the intermediate directory being temporary when building with Visual
Studio (warning MSB8029).

To uninstall, use `pip uninstall pylarid`.

## Manual Build Using CMake (advanced)

To build and install `pylarid` directly using CMake, create a `build` directory
within the `pylarid` directory and follow the normal steps for building and
installing software using CMake.

To build and install on macOS and Linux:

```
mkdir build
cd build
cmake ..
make
make install
```

To uninstall, use `make uninstall`.

To build and install on Windows using Visual Studio:

```
mkdir build
cd build
cmake ..
cmake --build . --config Release
cmake --build . --config Release --target install
```

To uninstall, use `cmake --build . --config Release --target uninstall`.

# Research and Educational Use Only

`pylarid` is for research and educational purposes only. Clinical applications
are not recommended or advised. `pylarid` has not been evaluated by the United
States Food and Drug Administration (FDA), or by any other agency. `pylarid` is
not intended to diagnose, treat, cure, or prevent any disease.

# License

Copyright 2023 Jeffrey M. Engelmann

`pylarid` is released under the MIT license. For details, see
[LICENSE.txt](LICENSE.txt).

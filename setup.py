###############################################################################
#                                 _                  _       _                #
#                                | |                (_)     | |               #
#                 _ __    _   _  | |   __ _   _ __   _    __| |               #
#                | '_ \  | | | | | |  / _` | | '__| | |  / _` |               #
#                | |_) | | |_| | | | | (_| | | |    | | | (_| |               #
#                | .__/   \__, | |_|  \__,_| |_|    |_|  \__,_|               #
#                | |       __/ |                                              #
#                |_|      |___/                                               #
#                                                                             #
#                         Load Arrays of Imaging Data                         #
#                                                                             #
# Copyright (c) 2021, Jeffrey M. Engelmann                                    #
#                                                                             #
# pylarid is released under the revised (3-clause) BSD license.               #
# For details, see LICENSE.txt                                                #
#                                                                             #
###############################################################################

# setup.py: Configure and build the pylarid package

import os
import sys
import platform
import subprocess
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

pylarid_options = []
cmake_cmd_args = []


class CMakeExtension(Extension):
    def __init__(self, name, cmake_lists_dir='.', **kwds):
        Extension.__init__(self, name, sources=[], **kwds)
        self.cmake_lists_dir = os.path.abspath(cmake_lists_dir)


class CMakeBuild(build_ext):
    def build_extensions(self):
        try:
            subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError('Cannot find CMake executable')

        for ext in self.extensions:
            with self._filter_build_errors(ext):
                self.build_extension(ext)

    def build_extension(self, ext):
        d = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cfg = 'Debug' if '--debug' in pylarid_options else 'Release'
        cmake_args = [
            '-DPYLARID_SETUP:BOOL=ON',
            '-DCMAKE_BUILD_TYPE={}'.format(cfg),
            '-D{}_OUTPUT_NAME={}'.format(ext.name.upper(),
                                         self.get_ext_filename(ext.name)),
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), d),
            '-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(),
                                                            self.build_temp),
            '-DPython3_EXECUTABLE={}'.format(sys.executable)
        ]

        if platform.system() == 'Windows':
            cmake_args += [
                '-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE',
                '-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), d)
            ]
            if not self.compiler.compiler_type == 'msvc':
                cmake_args += ['-G', 'MinGW Makefiles']

        cmake_args += cmake_cmd_args

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        # Configure and build the extension
        subprocess.check_call(['cmake', ext.cmake_lists_dir] + cmake_args,
                              cwd=self.build_temp)

        subprocess.check_call(['cmake', '--build', '.', '--config', cfg],
                              cwd=self.build_temp)


def main():
    # Package version
    version_string = '0.1.0.dev0'

    # Github URL
    url = 'https://github.com/jme2041/pylarid'

    # Get the package's long description from README.md
    here = os.path.abspath(os.path.dirname(__file__))
    with open(os.path.join(here, 'README.rst'), encoding='utf-8') as f:
        long_description = f.read()

    # Extract options from sys.argv
    for arg in sys.argv:
        if arg == '--debug':
            pylarid_options.append(arg)

    for arg in pylarid_options:
        sys.argv.remove(arg)

    # Extract CMake arguments (which start with -D) from sys.argv
    for arg in sys.argv:
        if arg.startswith('-D'):
            cmake_cmd_args.append(arg)

    for arg in cmake_cmd_args:
        sys.argv.remove(arg)

    # Pass the version string as a CMake argument
    cmake_cmd_args.append(
        '-DPYLARID_VERSION_STRING:STRING={}'.format(version_string))

    # Add CMake as a build requirement if CMake is not installed or is too low
    # a version. This is the only build requirement that is specified using
    # setup_requires; all others are specified in pyproject.toml

    setup_requires = []
    try:
        cmake_version = subprocess.check_output(['cmake', '--version'])
        cmake_version = cmake_version.decode().splitlines()[0].split(' ')[-1]
        if cmake_version < "3.17":
            setup_requires.append('cmake')
    except(OSError, subprocess.CalledProcessError):
        setup_requires.append('cmake')

    # Call setuptools.setup for the package using custom build_ext
    setup(
        name='pylarid',
        version=version_string,
        description='Load Arrays of Imaging Data',
        long_description=long_description,
        long_description_content_type='text/x-rst',
        license='BSD 3-Clause License',
        url=url,
        author='Jeffrey M. Engelmann',
        author_email='jme2041@icloud.com',
        maintainer='Jeffrey M. Engelmann',
        maintainer_email='jme2041@icloud.com',
        classifiers=[
            'Development Status :: 3 - Alpha',
            'License :: OSI Approved :: BSD License',
            'Programming Language :: C',
            'Programming Language :: Python :: 3',
            'Programming Language :: Python :: 3.9',
            'Programming Language :: Python :: Implementation :: CPython',
            'Operating System :: MacOS :: MacOS X',
            'Operating System :: POSIX :: Linux',
            'Operating System :: Microsoft :: Windows :: Windows 10',
            'Environment :: Console',
            'Natural Language :: English',
            'Intended Audience :: Science/Research',
            'Topic :: Scientific/Engineering :: Bio-Informatics'
        ],
        keywords='mri fmri nifti binary file',
        python_requires='>=3.9.2',
        install_requires=['numpy>=1.20.1'],
        setup_requires=setup_requires,
        ext_modules=[CMakeExtension("larid")],
        cmdclass={'build_ext': CMakeBuild})


if __name__ == '__main__':
    main()

###############################################################################

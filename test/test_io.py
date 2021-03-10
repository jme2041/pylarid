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

# test_io.py: I/O tests for the larid.Dset class

import larid
import os
import shutil
import urllib.request
import hashlib
import numpy
import unittest

# Demonstration dataset attributes

demo3d = {'ni': 224, 'nj': 256, 'nk': 176, 'nt': 1, 'datatype': 'int16'}

demo4d = {'ni': 64, 'nj': 64, 'nk': 10, 'nt': 240, 'datatype': 'int16'}

# List of 3D test files

files3d = [
    ('str1l.nii', '7bde18bc708ebda70f1c3d1d20f81150'),
    ('str1lz.nii.gz', '2f19db1e9739874a54572ab613658c7b'),
    ('str1lp.hdr', '795c21195cda79590afeb065be696936'),
    ('str1lp.img', '09fdca5aef181596e675b518d080ecd6'),
    ('str1lpz.hdr.gz', 'f04fc633ee9e6e7d054f5eb8a675689a'),
    ('str1lpz.img.gz', '9e8d821cc2bea9236036c06591074946'),
    ('str2l.nii', 'f7935255ff8ad10befc4d431c708d576'),
    ('str2lz.nii.gz', '112c0bac88603f1a612ce1c46d8acfaa'),
    ('str2lp.hdr', '113647a6f35ebd0231281ad332c618dd'),
    ('str2lp.img', '09fdca5aef181596e675b518d080ecd6'),
    ('str2lpz.hdr.gz', 'b39fc96e65b916df505781ae6974fe5b'),
    ('str2lpz.img.gz', '08bdf3a34e23c0793eff74ef7f1db978'),
    ('str1b.nii', '6dea4422823c84474a9bc63ed28d908e'),
    ('str1bz.nii.gz', 'ee79d3b8296efb347ef10736e07b9da8'),
    ('str1bp.hdr', '4be4276143be48d026a05bc164164cbe'),
    ('str1bp.img', '7364df085dd714f315383f34fb3507ae'),
    ('str1bpz.hdr.gz', '3c9bc65bb3a869e689cc8deaf73a2bc9'),
    ('str1bpz.img.gz', '74db1dc3b7b1fda7504b1faf5f1b1004'),
    ('str2b.nii', '4ac9817c8ed7eabce37f9269645ee649'),
    ('str2bz.nii.gz', '15ef3fa256d8f6b34b46e335500ad742'),
    ('str2bp.hdr', 'df5a606f95c8757e9cf1842b43f16f0c'),
    ('str2bp.img', '7364df085dd714f315383f34fb3507ae'),
    ('str2bpz.hdr.gz', '58f484b31c091d92cc4c21455ac17f06'),
    ('str2bpz.img.gz', '79d6ad774c84d885dbb9f32f0c100202')
]

# List of 4D test files

files4d = [
    ('fun1l.nii', '91a264da9351cf608d03908e991f2ce7'),
    ('fun1lz.nii.gz', 'af999f96f67042f1fc149f96937b9fc1'),
    ('fun1lp.hdr', '8b6ad2d6298ca1c166c4e61a9debdc84'),
    ('fun1lp.img', 'bad7620415da616ccec6a47385849449'),
    ('fun1lpz.hdr.gz', '75a8e81ad3d62c50168eddb88e567a95'),
    ('fun1lpz.img.gz', '5cd3011291de90c3a36b28ac1fe18ab5'),
    ('fun2l.nii', 'ba881ef69bec4faaad328db0fd6f168c'),
    ('fun2lz.nii.gz', '82df07a81a86443186f3e619d6ff4021'),
    ('fun2lp.hdr', '2ef0f92fa548c674660f7b35dd121a1b'),
    ('fun2lp.img', 'bad7620415da616ccec6a47385849449'),
    ('fun2lpz.hdr.gz', 'af41172b300ebb6b19262788cb18c89a'),
    ('fun2lpz.img.gz', '757d5146fbd1b8ee1c3a71ea61adb1bb'),
    ('fun1b.nii', '939c7aa361caca4dbef567f587f245b3'),
    ('fun1bz.nii.gz', 'b5582d58c68067294018295f17b811f4'),
    ('fun1bp.hdr', 'c3b61aaf18c8142ec89d55540fa30108'),
    ('fun1bp.img', 'f9773a454eb77c6fe75f78a152b21eab'),
    ('fun1bpz.hdr.gz', 'b57cc3f898997156b7c3402df39cc1be'),
    ('fun1bpz.img.gz', '96bb989c86095c21f79db4665cff16ea'),
    ('fun2b.nii', '91a1461e8f586d65a946256d55733580'),
    ('fun2bz.nii.gz', 'ff81bd6c4508de1d215b424ca1f6616b'),
    ('fun2bp.hdr', '38d933fcc01235b26253677d00e4eb08'),
    ('fun2bp.img', 'f9773a454eb77c6fe75f78a152b21eab'),
    ('fun2bpz.hdr.gz', '0e9e58a19f09876413b4ac000db3859c'),
    ('fun2bpz.img.gz', '092cf364c2f98f8de3ea5cbe94d97ff7')
]

datadir = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'data')


def setUpModule():
    if not os.path.isdir(datadir):
        os.mkdir(datadir)

    for f in files3d + files4d:
        fname = f[0]
        md5 = f[1]
        path = os.path.join(datadir, fname)
        if not os.path.isfile(path):
            print('Downloading test file: {}'.format(fname))
            urllib.request.urlretrieve(
                'https://www.jeffengelmann.com/data/{}'.format(fname), path)
        with open(path, 'rb') as ff:
            if hashlib.md5(ff.read()).hexdigest() != md5:
                raise IOError('Bad md5: {}'.format(fname))


class TestIO(unittest.TestCase):
    """Test NIfTI I/O"""

    def check_attribs(self, dset, datatype=None, nt=None):
        # Check dset attributes against a demonstration dataset (self.demo)
        if datatype is None:
            datatype = self.demo['datatype']
        if nt is None:
            nt = self.demo['nt']

        # Check array shape and data type
        self.assertEqual(dset.data.shape, (nt,
                                           self.demo['nk'],
                                           self.demo['nj'],
                                           self.demo['ni']))
        self.assertEqual(dset.data.dtype, numpy.dtype(datatype))

        # Check returned dimensions, data type, and memory order
        self.assertEqual(dset.ni, self.demo['ni'])
        self.assertEqual(dset.nj, self.demo['nj'])
        self.assertEqual(dset.nk, self.demo['nk'])
        self.assertEqual(dset.nt, nt)
        self.assertEqual(dset.datatype, datatype)
        self.assertEqual(dset.morder, 'tkji')


class TestRead(TestIO):
    """Test reading NIfTI files"""

    def check_read(self, path):
        # Check reading a file (given by path) against self.demo

        # Skip img and img.gz
        if path.endswith('.img') or path.endswith('.img.gz'):
            return

        if not larid.zlib and path.endswith('.gz'):
            # On non-zlib builds, check that reading a gzipped file fails
            with self.assertRaisesRegex(larid.LaridError,
                                        'Gzipped files not supported'):
                larid.Dset.like_nifti(path)
        else:
            # Otherwise, check reading with both like_nifti and from_nifti
            for reader in (larid.Dset.like_nifti, larid.Dset.from_nifti):
                with self.subTest(reader=reader):
                    dset = reader(path)
                    self.check_attribs(dset)
                    if reader == larid.Dset.like_nifti:
                        self.assertTrue(numpy.allclose(dset.data, 0))
                    else:
                        self.check_data(dset)

    def check_switch_datatype(self, path):
        for datatype in ('uint8', 'int16', 'int32', 'float32', 'float64'):
            with self.subTest(datatype=datatype):
                dset = larid.Dset.like_nifti(path, datatype=datatype)
                self.check_attribs(dset, datatype=datatype)
                self.assertTrue(numpy.allclose(dset.data, 0))

    def check_switch_nt(self, path):
        for nt in (1, 2, 10):
            with self.subTest(nt=nt):
                dset = larid.Dset.like_nifti(path, nt=nt)
                self.check_attribs(dset, nt=nt)
                self.assertTrue(numpy.allclose(dset.data, 0))

    def check_switch_both(self, path):
        for datatype in ('uint8', 'int16', 'int32', 'float32', 'float64'):
            for nt in (1, 2, 10):
                with self.subTest(datatype=datatype, nt=nt):
                    dset = larid.Dset.like_nifti(path, datatype, nt)
                    self.check_attribs(dset, datatype, nt)
                    self.assertTrue(numpy.allclose(dset.data, 0))

    def test_noarg(self):
        for reader in (larid.Dset.like_nifti, larid.Dset.from_nifti):
            with self.subTest(reader=reader):
                with self.assertRaises(TypeError):
                    reader()

    def test_bad_path_type(self):
        for reader in (larid.Dset.like_nifti, larid.Dset.from_nifti):
            with self.subTest(reader=reader):
                with self.assertRaises(TypeError):
                    reader(1)

    def test_bad_extension(self):
        # Extension check is the same for like_nifti and from_nifti
        with self.assertRaisesRegex(larid.LaridError,
                                    'Invalid NIfTI file extension'):
            larid.Dset.like_nifti('fake.bin')

    def test_bad_datatype(self):
        with self.assertRaisesRegex(ValueError, 'Invalid data type: fake'):
            larid.Dset.like_nifti('fake.nii', datatype='fake')

    def test_bad_nt(self):
        with self.assertRaisesRegex(ValueError,
                                    'Requested nt cannot be less than zero'):
            larid.Dset.like_nifti('fake.nii', nt=-1)

    def test_no_file(self):
        # File check is the same for like_nifti and from_nifti
        with self.assertRaises(IOError):
            larid.Dset.like_nifti('fake.nii')

    def test_no_img(self):
        path = os.path.join(datadir, 'tmp.hdr')
        try:
            if not os.path.isfile(path):
                shutil.copy(os.path.join(datadir, 'str2lp.hdr'), path)
            self.assertTrue(os.path.isfile(path))
            with self.assertRaises(IOError):
                larid.Dset.from_nifti(path)
        finally:
            os.remove(path)


class TestRead3D(TestRead):
    """Test reading the 3D demonstration dataset"""

    def setUp(self):
        self.demo = demo3d

    def check_data(self, dset):
        # Check first and last four voxels against 3D demonstration dataset
        self.assertEqual(dset.data[0, 0, 0, 0], 0)
        self.assertEqual(dset.data[0, 0, 0, 1], 0)
        self.assertEqual(dset.data[0, 0, 0, 2], 0)
        self.assertEqual(dset.data[0, 0, 0, 3], 0)
        self.assertEqual(dset.data[0, 175, 255, 220], 13)
        self.assertEqual(dset.data[0, 175, 255, 221], 11)
        self.assertEqual(dset.data[0, 175, 255, 222], 18)
        self.assertEqual(dset.data[0, 175, 255, 223], 13)

    def test_good(self):
        # Test reading 3D NIfTI files
        for f in files3d:
            with self.subTest(f=f):
                path = os.path.join(datadir, f[0])
                self.check_read(path)

    def test_switch_datatype(self):
        # Test switching datatype
        path = os.path.join(datadir, 'str2l.nii')
        self.check_switch_datatype(path)

    def test_switch_nt(self):
        # Test switching nt
        path = os.path.join(datadir, 'str2l.nii')
        self.check_switch_nt(path)

    def test_switch_both(self):
        # Test switching both datatype and nt
        path = os.path.join(datadir, 'str2l.nii')
        self.check_switch_both(path)


class TestRead4D(TestRead):
    """Test reading the 4D demonstration dataset"""

    def setUp(self):
        self.demo = demo4d

    def check_data(self, dset):
        # Check first and last four voxels against 4D demonstration dataset
        self.assertEqual(dset.data[0, 0, 0, 0], 0)
        self.assertEqual(dset.data[0, 0, 0, 1], 0)
        self.assertEqual(dset.data[0, 0, 0, 2], 0)
        self.assertEqual(dset.data[0, 0, 0, 3], 0)
        self.assertEqual(dset.data[239, 9, 63, 60], 23)
        self.assertEqual(dset.data[239, 9, 63, 61], 42)
        self.assertEqual(dset.data[239, 9, 63, 62], 58)
        self.assertEqual(dset.data[239, 9, 63, 63], 27)

    def test_good(self):
        # Test reading 4D NIfTI files
        for f in files4d:
            with self.subTest(f=f):
                path = os.path.join(datadir, f[0])
                self.check_read(path)

    def test_switch_datatype(self):
        # Test switching datatype
        path = os.path.join(datadir, 'fun2l.nii')
        self.check_switch_datatype(path)

    def test_switch_nt(self):
        # Test switching nt
        path = os.path.join(datadir, 'fun2l.nii')
        self.check_switch_nt(path)

    def test_switch_both(self):
        # Test switching both datatype and nt
        path = os.path.join(datadir, 'fun2l.nii')
        self.check_switch_both(path)


class TestWrite(TestIO):
    """Test writing NIfTI files"""

    source = None
    template = None

    def check_write(self):
        # Check writing TestWrite.source to NIfTI
        for morder in ('tkji', 'kjit'):
            TestWrite.source.morder = morder
            for nifti_ver in (1, 2):
                for fmt in ('.hdr', '.hdr.gz', '.nii', '.nii.gz'):
                    with self.subTest(
                            morder=morder, nifti_ver=nifti_ver, fmt=fmt):
                        path = os.path.join(datadir,
                                            'tmp%d%s' % (nifti_ver, fmt))
                        if larid.zlib == 0 and path.endswith('.gz'):
                            with self.assertRaisesRegex(
                                    larid.LaridError,
                                    'Gzipped files not supported'):
                                TestWrite.source.to_nifti(path, nifti_ver)
                        else:
                            try:

                                TestWrite.source.to_nifti(path, nifti_ver)
                                dset = larid.Dset.from_nifti(path)
                                self.check_attribs(dset)
                                self.assertTrue(numpy.allclose(
                                    TestWrite.template.data,
                                    dset.data))
                            finally:
                                if os.path.isfile(path):
                                    os.remove(path)
                                path = path.replace('.hdr', '.img')
                                if os.path.isfile(path):
                                    os.remove(path)


class TestWrite3D(TestWrite):
    """Test writing the 3D demonstration dataset"""

    @classmethod
    def setUpClass(cls):
        path = os.path.join(datadir, 'str2l.nii')
        TestWrite.source = larid.Dset.from_nifti(path)
        TestWrite.template = TestWrite.source.copy()

    def test_noarg(self):
        with self.assertRaises(TypeError):
            TestWrite.source.to_nifti()

    def test_bad_path_type(self):
        with self.assertRaises(TypeError):
            TestWrite.source.to_nifti(1)

    def test_bad_extension(self):
        with self.assertRaisesRegex(larid.LaridError,
                                    'Invalid NIfTI file extension'):
            TestWrite.source.to_nifti('fake.bin')

    def test_bad_nifti_ver_type(self):
        with self.assertRaises(TypeError):
            TestWrite.source.to_nifti(os.path.join(datadir, 'test.nii'), '1')

    def test_bad_nifti_ver(self):
        with self.assertRaisesRegex(ValueError, 'Invalid nifti_ver: 3'):
            TestWrite.source.to_nifti(os.path.join(datadir, 'test.nii'), 3)

    def test_good(self):
        self.demo = demo3d
        self.check_write()


class TestWrite4D(TestWrite):
    """Test writing the 4D demonstration dataset"""

    @classmethod
    def setUpClass(cls):
        path = os.path.join(datadir, 'fun2l.nii')
        TestWrite.source = larid.Dset.from_nifti(path)
        TestWrite.template = TestWrite.source.copy()

    def test_good(self):
        self.demo = demo4d
        self.check_write()


if __name__ == '__main__':
    unittest.main()

###############################################################################

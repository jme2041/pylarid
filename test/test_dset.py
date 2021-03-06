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

# test_dset.py: Unit tests for the larid.Dset class

import larid
import numpy
import unittest

# Demonstration dataset attributes

demo3d = {'ni': 224, 'nj': 256, 'nk': 176, 'nt': 1, 'datatype': 'int16'}

demo4d = {'ni': 64, 'nj': 64, 'nk': 10, 'nt': 240, 'datatype': 'int16'}


class TestDset(unittest.TestCase):
    """Test basic functionality of the larid.Dset class"""

    def test_init_noarg(self):
        with self.assertRaises(TypeError):
            larid.Dset()

    def test_init_onearg(self):
        with self.assertRaises(TypeError):
            larid.Dset(1)

    def test_init_twoarg(self):
        with self.assertRaises(TypeError):
            larid.Dset(1, 1)

    def test_init_threearg(self):
        with self.assertRaises(TypeError):
            larid.Dset(1, 1, 1)

    def test_init_fourarg(self):
        with self.assertRaises(TypeError):
            larid.Dset(1, 1, 1, 1)

    def test_init_fivearg(self):
        with self.assertRaises(TypeError):
            larid.Dset(1, 1, 1, 1, 'uint8')

    def test_init_dim_str(self):
        x = '1'
        for dim in ((x, 1, 1, 1), (1, x, 1, 1), (1, 1, x, 1), (1, 1, 1, x)):
            with self.assertRaises(TypeError):
                larid.Dset(*dim, datatype='uint8', morder='tkji')

    def test_init_dim_undermin(self):
        x = 0
        for dim in ((x, 1, 1, 1), (1, x, 1, 1), (1, 1, x, 1), (1, 1, 1, x)):
            with self.assertRaisesRegex(
                    ValueError,
                    'Dataset dimensions must be greater than zero'):
                larid.Dset(*dim, datatype='uint8', morder='tkji')

    def test_init_dim_overmax(self):
        x = 2 ** 63
        for dim in ((x, 1, 1, 1), (1, x, 1, 1), (1, 1, x, 1), (1, 1, 1, x)):
            with self.assertRaises(OverflowError):
                larid.Dset(*dim, datatype='uint8', morder='tkji')

    def test_init_overflow(self):
        x = 0xF0000000
        with self.assertRaises(ValueError):
            larid.Dset(x, x, x, x, datatype='uint8', morder='tkji')

    def test_init_datatype_numeric(self):
        with self.assertRaisesRegex(TypeError, 'Data type must be a string'):
            larid.Dset(1, 1, 1, 1, 1, 'tkji')

    def test_init_datatype_bad(self):
        with self.assertRaisesRegex(ValueError, 'Invalid data type: bad'):
            larid.Dset(1, 1, 1, 1, 'bad', 'tkji')

    def test_init_morder_numeric(self):
        with self.assertRaisesRegex(TypeError,
                                    'Memory order must be a string'):
            larid.Dset(1, 1, 1, 1, 'uint8', 1)

    def test_init_morder_bad(self):
        with self.assertRaisesRegex(ValueError, 'Invalid memory order: bad'):
            larid.Dset(1, 1, 1, 1, 'uint8', 'bad')

    def test_init_good(self):
        # Create a Dset object for the 4D demonstration dataset
        obj = larid.Dset(demo4d['ni'], demo4d['nj'], demo4d['nk'],
                         demo4d['nt'], demo4d['datatype'], 'tkji')
        p1 = obj.data.__array_interface__['data'][0]

        # Check array shape and data type
        self.assertEqual(obj.data.shape, (demo4d['nt'], demo4d['nk'],
                                          demo4d['nj'], demo4d['ni']))
        self.assertEqual(obj.data.dtype, numpy.dtype(demo4d['datatype']))

        # Check returned dimensions, data type, and memory order
        self.assertEqual(obj.ni, demo4d['ni'])
        self.assertEqual(obj.nj, demo4d['nj'])
        self.assertEqual(obj.nk, demo4d['nk'])
        self.assertEqual(obj.nt, demo4d['nt'])
        self.assertEqual(obj.datatype, demo4d['datatype'])
        self.assertEqual(obj.morder, 'tkji')

        # Attempt to set read-only attributes
        with self.assertRaises(AttributeError):
            obj.data = numpy.zeros((1, 1, 1, 1), obj.data.dtype)
        with self.assertRaises(AttributeError):
            obj.ni = 1
        with self.assertRaises(AttributeError):
            obj.nj = 1
        with self.assertRaises(AttributeError):
            obj.nk = 1
        with self.assertRaises(AttributeError):
            obj.nt = 1
        with self.assertRaises(AttributeError):
            obj.datatype = 'uint8'

        # Attempt to delete read-write attributes
        with self.assertRaisesRegex(TypeError,
                                    'Cannot delete the morder attribute'):
            delattr(obj, 'morder')

        # Attributes should be preserved after failed re-initialization
        with self.assertRaises(ValueError):
            obj.__init__(demo4d['ni'], demo4d['nj'], demo4d['nk'], 0,
                         demo4d['datatype'], 'tkji')
        p2 = obj.data.__array_interface__['data'][0]
        self.assertEqual(p1, p2)
        self.assertEqual(obj.data.shape, (demo4d['nt'], demo4d['nk'],
                                          demo4d['nj'], demo4d['ni']))
        self.assertEqual(obj.data.dtype, numpy.dtype(demo4d['datatype']))
        self.assertEqual(obj.ni, demo4d['ni'])
        self.assertEqual(obj.nj, demo4d['nj'])
        self.assertEqual(obj.nk, demo4d['nk'])
        self.assertEqual(obj.nt, demo4d['nt'])
        self.assertEqual(obj.datatype, demo4d['datatype'])
        self.assertEqual(obj.morder, 'tkji')

        # Re-initialize the object for the 3D demonstration dataset
        # Attributes should change after successful re-initialization
        obj.__init__(demo3d['ni'], demo3d['nj'], demo3d['nk'], demo3d['nt'],
                     demo3d['datatype'], 'tkji')
        p3 = obj.data.__array_interface__['data'][0]
        self.assertNotEqual(p1, p3)
        self.assertEqual(obj.data.shape, (demo3d['nt'], demo3d['nk'],
                                          demo3d['nj'], demo3d['ni']))
        self.assertEqual(obj.data.dtype, numpy.dtype(demo3d['datatype']))
        self.assertEqual(obj.ni, demo3d['ni'])
        self.assertEqual(obj.nj, demo3d['nj'])
        self.assertEqual(obj.nk, demo3d['nk'])
        self.assertEqual(obj.nt, demo3d['nt'])
        self.assertEqual(obj.datatype, demo3d['datatype'])
        self.assertEqual(obj.morder, 'tkji')

    def test_copy(self):
        # Start with the demonstration 4D dataset
        obj1 = larid.Dset(demo4d['ni'], demo4d['nj'], demo4d['nk'],
                          demo4d['nt'], demo4d['datatype'], 'tkji')
        p1 = obj1.data.__array_interface__['data'][0]

        # Do the copy
        obj2 = obj1.copy()
        p2 = obj2.data.__array_interface__['data'][0]

        # Voxel data should not be the same array object
        self.assertNotEqual(p1, p2)

        # Voxel data should have the same shape, dtype, and data
        self.assertEqual(obj1.data.shape, obj2.data.shape)
        self.assertEqual(obj1.data.dtype, obj2.data.dtype)
        self.assertTrue(numpy.allclose(obj1.data, obj2.data))

        # Attributes should be the same
        self.assertEqual(obj1.ni, obj2.ni)
        self.assertEqual(obj1.nj, obj2.nj)
        self.assertEqual(obj1.nk, obj2.nk)
        self.assertEqual(obj1.nt, obj2.nt)
        self.assertEqual(obj1.datatype, obj2.datatype)
        self.assertEqual(obj1.morder, obj2.morder)


if __name__ == '__main__':
    unittest.main()

###############################################################################

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

# test_cast.py: Unit tests for casting between larid.Dset data types

import larid
import math
import numpy
import unittest


class TestCast(unittest.TestCase):
    """Test casting between data types"""

    def test_upcast_uint8(self):
        # uint8 to same or bigger types
        for datatype in ('uint8', 'int16', 'int32', 'float32', 'float64'):
            with self.subTest(datatype=datatype):
                obj1 = larid.Dset(1, 1, 1, 1, 'uint8', 'tkji')
                obj1.data[:] = 1
                obj2 = obj1.to_datatype(datatype)
                self.assertEqual(obj2.datatype, datatype)
                self.assertEqual(obj2.data.dtype, numpy.dtype(datatype))
                self.assertEqual(obj2.data[0, 0, 0, 0], 1)

    def test_upcast_int16(self):
        # int16 to same or bigger types
        for datatype in ('int16', 'int32', 'float32', 'float64'):
            with self.subTest(datatype=datatype):
                obj1 = larid.Dset(1, 1, 1, 1, 'int16', 'tkji')
                obj1.data[:] = 1
                obj2 = obj1.to_datatype(datatype)
                self.assertEqual(obj2.datatype, datatype)
                self.assertEqual(obj2.data.dtype, numpy.dtype(datatype))
                self.assertEqual(obj2.data[0, 0, 0, 0], 1)

    def test_upcast_int32(self):
        # int32 to same type or floating-point types
        for datatype in ('int32', 'float32', 'float64'):
            with self.subTest(datatype=datatype):
                obj1 = larid.Dset(1, 1, 1, 1, 'int32', 'tkji')
                obj1.data[:] = 1
                obj2 = obj1.to_datatype(datatype)
                self.assertEqual(obj2.datatype, datatype)
                self.assertEqual(obj2.data.dtype, numpy.dtype(datatype))
                self.assertEqual(obj2.data[0, 0, 0, 0], 1)

    def test_upcast_float32(self):
        # float32 to itself or float64
        for datatype in ('float32', 'float64'):
            with self.subTest(datatype=datatype):
                obj1 = larid.Dset(1, 1, 1, 1, 'float32', 'tkji')
                obj1.data[:] = 1
                obj2 = obj1.to_datatype(datatype)
                self.assertEqual(obj2.datatype, datatype)
                self.assertEqual(obj2.data.dtype, numpy.dtype(datatype))
                self.assertEqual(obj2.data[0, 0, 0, 0], 1)

    def test_downcast_float64_to_float32(self):
        # float64 to float32
        obj1 = larid.Dset(1, 1, 1, 1, 'float64', 'tkji')
        obj1.data[:] = 1.2345678
        obj2 = obj1.to_datatype('float32')
        self.assertEqual(obj2.datatype, 'float32')
        self.assertEqual(obj2.data.dtype, numpy.dtype('float32'))
        self.assertTrue(math.isclose(obj2.data[0, 0, 0, 0], 1.2345678,
                                     abs_tol=1e-07))
        self.assertFalse(math.isclose(obj2.data[0, 0, 0, 0], 1.2345678,
                                      abs_tol=1e-08))

    def test_downcast_float64_to_signed(self):
        # float64 to signed integer types
        for datatype in ('int32', 'int16'):
            with self.subTest(datatype=datatype):
                obj1 = larid.Dset(1, 1, 1, 1, 'float64', 'tkji')
                obj1.data[:] = 1.2345678
                obj2 = obj1.to_datatype(datatype)
                self.assertEqual(obj2.datatype, datatype)
                self.assertEqual(obj2.data.dtype, numpy.dtype(datatype))
                self.assertEqual(obj2.data[0, 0, 0, 0], 1)
                obj1.data[:] = -1.2345678
                obj2 = obj1.to_datatype(datatype)
                self.assertEqual(obj2.datatype, datatype)
                self.assertEqual(obj2.data.dtype, numpy.dtype(datatype))
                self.assertEqual(obj2.data[0, 0, 0, 0], -1)

    def test_downcast_float64_to_uint8(self):
        # float64 to uint8
        obj1 = larid.Dset(1, 1, 1, 1, 'float64', 'tkji')
        obj1.data[:] = 1.2345678
        obj2 = obj1.to_datatype('uint8')
        self.assertEqual(obj2.datatype, 'uint8')
        self.assertEqual(obj2.data.dtype, numpy.dtype('uint8'))
        self.assertEqual(obj2.data[0, 0, 0, 0], 1)
        obj1.data[:] = -1.2345678
        obj2 = obj1.to_datatype('uint8')
        self.assertEqual(obj2.datatype, 'uint8')
        self.assertEqual(obj2.data.dtype, numpy.dtype('uint8'))
        self.assertEqual(obj2.data[0, 0, 0, 0], 255)

    def test_downcast_float32_to_signed(self):
        # float32 to signed integer types
        for datatype in ('int32', 'int16'):
            with self.subTest(datatype=datatype):
                obj1 = larid.Dset(1, 1, 1, 1, 'float32', 'tkji')
                obj1.data[:] = 1.2345678
                obj2 = obj1.to_datatype(datatype)
                self.assertEqual(obj2.datatype, datatype)
                self.assertEqual(obj2.data.dtype, numpy.dtype(datatype))
                self.assertEqual(obj2.data[0, 0, 0, 0], 1)
                obj1.data[:] = -1.2345678
                obj2 = obj1.to_datatype(datatype)
                self.assertEqual(obj2.datatype, datatype)
                self.assertEqual(obj2.data.dtype, numpy.dtype(datatype))
                self.assertEqual(obj2.data[0, 0, 0, 0], -1)

    def test_downcast_float32_to_uint8(self):
        # float32 to uint8
        obj1 = larid.Dset(1, 1, 1, 1, 'float32', 'tkji')
        obj1.data[:] = 1.2345678
        obj2 = obj1.to_datatype('uint8')
        self.assertEqual(obj2.datatype, 'uint8')
        self.assertEqual(obj2.data.dtype, numpy.dtype('uint8'))
        self.assertEqual(obj2.data[0, 0, 0, 0], 1)
        obj1.data[:] = -1.2345678
        obj2 = obj1.to_datatype('uint8')
        self.assertEqual(obj2.datatype, 'uint8')
        self.assertEqual(obj2.data.dtype, numpy.dtype('uint8'))
        self.assertEqual(obj2.data[0, 0, 0, 0], 255)

    def test_downcast_int32_to_int16(self):
        # int32 to int16
        obj1 = larid.Dset(1, 1, 1, 1, 'int32', 'tkji')
        obj1.data[:] = (2 ** 15) - 1
        obj2 = obj1.to_datatype('int16')
        self.assertEqual(obj2.datatype, 'int16')
        self.assertEqual(obj2.data.dtype, numpy.dtype('int16'))
        self.assertEqual(obj2.data[0, 0, 0, 0], (2 ** 15) - 1)
        obj1.data[:] = (2 ** 15)
        obj2 = obj1.to_datatype('int16')
        self.assertEqual(obj2.datatype, 'int16')
        self.assertEqual(obj2.data.dtype, numpy.dtype('int16'))
        self.assertEqual(obj2.data[0, 0, 0, 0], -2 ** 15)

    def test_downcast_signed_to_uint8(self):
        # int32 and int16 to uint8
        for datatype in ('int32', 'int16'):
            with self.subTest(datatype=datatype):
                obj1 = larid.Dset(1, 1, 1, 1, datatype, 'tkji')
                obj1.data[:] = 255
                obj2 = obj1.to_datatype('uint8')
                self.assertEqual(obj2.datatype, 'uint8')
                self.assertEqual(obj2.data.dtype, 'uint8')
                self.assertEqual(obj2.data[0, 0, 0, 0], 255)
                obj1.data[:] = 256
                obj2 = obj1.to_datatype('uint8')
                self.assertEqual(obj2.datatype, 'uint8')
                self.assertEqual(obj2.data.dtype, 'uint8')
                self.assertEqual(obj2.data[0, 0, 0, 0], 0)
                obj1.data[:] = -1
                obj2 = obj1.to_datatype('uint8')
                self.assertEqual(obj2.datatype, 'uint8')
                self.assertEqual(obj2.data.dtype, 'uint8')
                self.assertEqual(obj2.data[0, 0, 0, 0], 255)


if __name__ == '__main__':
    unittest.main()

###############################################################################

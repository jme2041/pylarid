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

# test_morder.py: Unit tests for larid.Dset memory orders

import larid
import numpy
import unittest

# Demonstration dataset attributes

demo4d = {'ni': 64, 'nj': 64, 'nk': 10, 'nt': 240, 'datatype': 'int16'}


class TestMorder(unittest.TestCase):
    """Test switching memory orders"""

    def test_ramp(self):
        for datatype in ('uint8', 'int16', 'int32', 'float32', 'float64'):
            with self.subTest(datatype=datatype):

                obj = larid.Dset(demo4d['ni'], demo4d['nj'], demo4d['nk'],
                                 demo4d['nt'], datatype, 'tkji')
                p1 = obj.data.__array_interface__['data'][0]

                # Start with a ramp image
                for j in range(0, demo4d['nj']):
                    obj.data[:, :, j, :] = j

                # Transpose to kjit
                obj.morder = 'kjit'
                p2 = obj.data.__array_interface__['data'][0]
                self.assertEqual(p1, p2)

                # Check
                for j in range(0, demo4d['nj']):
                    self.assertTrue(numpy.allclose(obj.data[:, j, :, :], j))

                # Transpose back to tkji
                obj.morder = 'tkji'
                p3 = obj.data.__array_interface__['data'][0]
                self.assertEqual(p1, p3)

                # Check
                for j in range(0, demo4d['nj']):
                    self.assertTrue(numpy.allclose(obj.data[:, :, j, :], j))

                # Redundant request should have no effect
                obj.morder = 'tkji'
                p4 = obj.data.__array_interface__['data'][0]
                self.assertEqual(p1, p4)

                # Check
                for j in range(0, demo4d['nj']):
                    self.assertTrue(numpy.allclose(obj.data[:, :, j, :], j))

    def test_tseries(self):
        for datatype in ('uint8', 'int16', 'int32', 'float32', 'float64'):
            with self.subTest(datatype=datatype):

                obj = larid.Dset(demo4d['ni'], demo4d['nj'], demo4d['nk'],
                                 demo4d['nt'], datatype, 'kjit')
                p1 = obj.data.__array_interface__['data'][0]

                # Start with an ascending time series
                for t in range(0, demo4d['nt']):
                    obj.data[:, :, :, t] = t

                # Transpose to tkji
                obj.morder = 'tkji'
                p2 = obj.data.__array_interface__['data'][0]
                self.assertEqual(p1, p2)

                # Check
                for t in range(0, demo4d['nt']):
                    self.assertTrue(numpy.allclose(obj.data[t, :, :, :], t))

                # Transpose back to kjit
                obj.morder = 'kjit'
                p3 = obj.data.__array_interface__['data'][0]
                self.assertEqual(p1, p3)

                # Check
                for t in range(0, demo4d['nt']):
                    self.assertTrue(numpy.allclose(obj.data[:, :, :, t], t))

                # Redundant request should have no effect
                obj.morder = 'kjit'
                p4 = obj.data.__array_interface__['data'][0]
                self.assertEqual(p1, p4)

                # Check
                for t in range(0, demo4d['nt']):
                    self.assertTrue(numpy.allclose(obj.data[:, :, :, t], t))


if __name__ == '__main__':
    unittest.main()

###############################################################################

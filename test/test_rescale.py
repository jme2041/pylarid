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

# test_rescale.py: Unit tests for larid.Dset.rescale

import larid
import numpy
import unittest

# Demonstration dataset attributes

demo3d = {'ni': 224, 'nj': 256, 'nk': 176, 'nt': 1, 'datatype': 'int16'}


class TestRescale(unittest.TestCase):
    """Test rescaling a dataset"""

    def setUp(self):
        self.obj = larid.Dset(demo3d['ni'], demo3d['nj'], demo3d['nk'],
                              demo3d['nt'], demo3d['datatype'], 'tkji')

    def test_rescale_noarg(self):
        with self.assertRaises(TypeError):
            self.obj.rescale()

    def test_rescale_onearg(self):
        with self.assertRaises(TypeError):
            self.obj.rescale(1)

    def test_rescale_min_bad(self):
        with self.assertRaises(TypeError):
            self.obj.rescale('1')

    def test_rescale_max_bad(self):
        with self.assertRaises(TypeError):
            self.obj.rescale(1, '2')

    def test_rescale_min_eq_max(self):
        with self.assertRaisesRegex(ValueError,
                                    'new_min must be less than new_max'):
            self.obj.rescale(1, 1)

    def test_rescale_min_gt_max(self):
        with self.assertRaisesRegex(ValueError,
                                    'new_min must be less than new_max'):
            self.obj.rescale(2, 1)

    def test_rescale_good(self):
        # Start with a ramp image [0, 255]
        for j in range(0, self.obj.nj):
            self.obj.data[:, :, j, :] = j

        # Rescale up to [-255, 255]
        self.obj.rescale(-255, 255)
        for j in range(0, self.obj.nj):
            self.assertTrue(numpy.allclose(
                self.obj.data[:, :, j, :], -255 + 2 * j))

        # Rescale up to [-510, 510]
        self.obj.rescale(-510, 510)
        for j in range(0, self.obj.nj):
            self.assertTrue(
                numpy.allclose(self.obj.data[:, :, j, :], -510 + 4 * j))

        # Rescale back to [0, 255]
        self.obj.rescale(0, 255)
        for j in range(0, self.obj.nj):
            self.assertTrue(numpy.allclose(self.obj.data[:, :, j, :], j))


if __name__ == '__main__':
    unittest.main()

###############################################################################

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

# test_larid.py: Unit tests for extension module larid

import larid
import unittest


class TestLarid(unittest.TestCase):
    """Test the larid module"""

    def test_larid_error(self):
        with self.assertRaisesRegex(larid.LaridError, 'TEST'):
            raise larid.LaridError('TEST')


if __name__ == '__main__':
    unittest.main()

###############################################################################

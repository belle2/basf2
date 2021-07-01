#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import unittest
import variables.collections as vc
from variables import variables as vm


class TestVariableCollections(unittest.TestCase):
    """Test case for the variables.collections """

    def test_collections(self):
        """ Loads all variables from collections into variable manager """
        excluded = ['utils', 'name', 'value']
        for collection in dir(vc):
            if (collection.startswith('__') or collection in excluded):
                continue
            var_collection = getattr(vc, collection)
            for variable_name in var_collection:
                variable = vm.getVariable(variable_name)
                self.assertNotEqual(variable, None)


if __name__ == '__main__':
    unittest.main()

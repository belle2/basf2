#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import shutil
import tempfile
from unittest import TestCase, main

import basf2
from basf2 import pickle_path as b2pp

# @cond internal_test


class PathSerialisationTestCase(TestCase):

    def setUp(self):
        # Create a temporary directory
        self.test_dir = tempfile.mkdtemp()
        self.pickle_file = os.path.join(self.test_dir, "output.pkl")

    def tearDown(self):
        # Remove the directory after the test
        shutil.rmtree(self.test_dir)

    def check_if_paths_equal(self, path1, path2):
        self.assertEqual(len(path1.modules()), len(path2.modules()))

        for module1, module2 in zip(path1.modules(), path2.modules()):
            self.assertEqual(module1.name(), module2.name())
            self.assertEqual(module1.type(), module2.type())
            self.assertEqual(module1.description(), module2.description())
            self.assertEqual(module1.package(), module2.package())

            self.assertEqual(len(module1.available_params()), len(module2.available_params()))
            for param1, param2 in zip(module1.available_params(), module2.available_params()):
                self.assertEqual(param1.name, param2.name)
                self.assertEqual(param1.description, param2.description)
                self.assertEqual(param1.values, param2.values)

            self.assertEqual(len(module1.get_all_conditions()), len(module2.get_all_conditions()))
            for cond1, cond2 in zip(module1.get_all_conditions(), module2.get_all_conditions()):
                self.assertEqual(cond1.get_value(), cond2.get_value())
                self.assertEqual(cond1.get_operator(), cond2.get_operator())
                self.assertEqual(cond1.get_after_path(), cond2.get_after_path())

                self.check_if_paths_equal(cond1.get_path(), cond2.get_path())

    def pickle_and_check(self, path):
        b2pp.write_path_to_file(path, self.pickle_file)
        pickled_path = b2pp.get_path_from_file(self.pickle_file)

        self.check_if_paths_equal(pickled_path, path)

    def test_simple_path(self):
        path = basf2.create_path()
        path.add_module("EventInfoSetter", evtNumList=[42])

        self.pickle_and_check(path)

    def test_condition_path(self):
        path = basf2.create_path()
        module = path.add_module("EventInfoSetter", evtNumList=[42])

        condition_path_1 = basf2.create_path()
        condition_path_1.add_module("Geometry", components=["SVD"])
        condition_path_2 = basf2.create_path()
        condition_path_2.add_module("Geometry", components=["CDC"])

        module.if_value("<3", condition_path_1)
        module.if_value(">5", condition_path_2)

        self.pickle_and_check(condition_path_1)
        self.pickle_and_check(condition_path_2)
        self.pickle_and_check(path)

    def test_high_level_things(self):
        self.assertNotEqual(0, os.system('basf2 --execute-path /this/path/doesnot/exist'))

        pathFile = tempfile.NamedTemporaryFile(prefix='b2pathtest_')

        # uses Python modules, this should fail
        steeringFile = basf2.find_file('framework/tests/module_paths.py')
        self.assertNotEqual(0, os.system('basf2 --dump-path ' + pathFile.name + ' ' + steeringFile))

        # test actual execution
        outputFile = tempfile.NamedTemporaryFile(prefix='b2pathtest_')

        path = basf2.create_path()
        path.add_module('EventInfoSetter', evtNumList=[2, 1], expList=[1, 1], runList=[1, 2])
        path.add_module('RootOutput', outputFileName=outputFile.name)

        # equivalent to --dump-path
        basf2.set_pickle_path(pathFile.name)
        basf2.process(path)

        # path dumped, but not executed
        pathSize = os.stat(pathFile.name).st_size
        self.assertNotEqual(0, pathSize)
        self.assertEqual(0, os.stat(outputFile.name).st_size)

        # equivalent to --execute-path
        self.assertEqual(basf2.get_pickle_path(), pathFile.name)
        basf2.process(None)

        # path unmodified, output file created
        self.assertEqual(pathSize, os.stat(pathFile.name).st_size)
        self.assertNotEqual(0, os.stat(outputFile.name).st_size)


if __name__ == '__main__':
    main()

# @endcond

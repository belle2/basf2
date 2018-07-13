#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from basf2 import create_path
import stdCharged


class TestStdCharged(unittest.TestCase):
    """Test case for charged standard particle lists"""

    #: the basic std functions
    _base_functions = [
        stdCharged.stdPi,
        stdCharged.stdK,
        stdCharged.stdPr,
        stdCharged.stdE,
        stdCharged.stdMu
    ]

    #: the functions with 95% lists
    _95_functions = [
        stdCharged.stdPi,
        stdCharged.stdK,
        stdCharged.stdMu,
        stdCharged.stdE
    ]

    #: the functions with 90% lists
    _90_functions = [
        stdCharged.stdPi,
        stdCharged.stdK,
        stdCharged.stdPr
    ]

    #: the functions with 95% lists
    _85_functions = [
        stdCharged.stdPi,
        stdCharged.stdK
    ]

    #: the stdLoose functions
    _loose_functions = [
        stdCharged.stdLoosePi,
        stdCharged.stdLooseK,
        stdCharged.stdLoosePr,
        stdCharged.stdLooseE,
        stdCharged.stdLooseMu
    ]

    def test_all(self):
        """check that all functions can be called with no argument"""
        for f in self._base_functions + self._loose_functions:
            testpath = create_path()
            f(path=testpath)
            self.assertEqual(len(testpath.modules()), 1,
                             "Function " + f.__name__ + " doesn't work")
            self.assertTrue(any(module.type() == "ParticleLoader" for module in testpath.modules()))

    def _test_listtype(self, listtype):
        """check that a given listtype function works"""
        for f in self._base_functions:
            testpath = create_path()
            f(listtype, path=testpath)
            self.assertEqual(len(testpath.modules()), 1,
                             "List %s doesn't work with function %s" % (f.__name__, listtype))
            self.assertTrue(any(module.type() == "ParticleLoader" for module in testpath.modules()))

    def test_all_list(self):
        """check that the builder functions all work with the all list"""
        self._test_listtype("all")

    def test_good_list(self):
        """check that the builder functions all work with the good list"""
        self._test_listtype("good")

    def test_higheff(self):
        """check that the builder functions all work with the higheff list"""
        self._test_listtype("higheff")

    def test_90(self):
        """check that the builder functions all work with the 90eff list"""
        for f in self._90_functions:
            testpath = create_path()
            f("90eff", path=testpath)
            self.assertEqual(len(testpath.modules()), 1,
                             "Can't call the 90\% eff list for " + f.__name__)
            self.assertTrue(any(module.type() == "ParticleLoader" for module in testpath.modules()))

    def test_95(self):
        """check that the builder functions all work with the 95eff list"""
        for f in self._95_functions:
            testpath = create_path()
            f("95eff", path=testpath)
            self.assertEqual(len(testpath.modules()), 1,
                             "Can't call the 95\% eff list for " + f.__name__)
            self.assertTrue(any(module.type() == "ParticleLoader" for module in testpath.modules()))


# suite = unittest.TestLoader().loadTestsFromTestCase(TestStdCharged)
# unittest.TextTestRunner(verbosity=2).run(suite)


if __name__ == '__main__':
    unittest.main()

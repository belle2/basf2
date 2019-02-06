#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from basf2 import create_path
import stdV0s


class TestStdV0(unittest.TestCase):
    """Test case for standard V0 lists"""

    def _check_list(
            self,
            std_function=stdV0s.stdKshorts,
            expected_modules=[
                "ParticleLoader",
                "TreeFitter",
                "ParticleSelector"],
            expected_lists=['all']):
        """check that a given function works"""
        testpath = create_path()
        std_function(path=testpath)

        # verify that we load the correct amount of modules
        self.assertEqual(
            len(testpath.modules()), len(expected_modules),
            "Function %s does not load the expected number of modules" % (std_function.__name__))

        #
        loaded_modules = []
        built_lists = []
        for module in testpath.modules():
            loaded_modules.append(module.type())
            for param in module.available_params():
                if module.type() == 'ParticleLoader' and param.name == 'decayStringsWithCuts':
                    name = param.values[0][0].split(':')[1].split(' -> ')[0]
                    built_lists.append(name)
                if module.type() == 'ParticleListManipulator' and param.name == 'outputListName':
                    name = str(param.values).split(':')[1].split(' -> ')[0]
                    built_lists.append(name)
                if module.type() == 'ParticleCombiner' and param.name == 'decayString':
                    name = param.values.split(':')[1].split(' -> ')[0]
                    built_lists.append(name)

        # we have the modules we expect
        for a, b in zip(loaded_modules, expected_modules):
            self.assertEqual(a, b, "Loaded module \'%s\' instead of \'%s\' with function %s" % (a, b, std_function.__name__))

        # we have the particle lists we expect
        for a, b in zip(built_lists, expected_lists):
            self.assertEqual(a, b, "Loaded list \'%s\' instead of \'%s\' with function %s" % (a, b, std_function.__name__))

    def test_stdkshorts_list(self):
        """check that the builder function works with the stdKshorts list"""
        self._check_list()

    def test_mergedkshorts_list(self):
        """check that the builder function works with the merged Kshorts list"""
        expected_modules = [
            "ParticleLoader",
            "ParticleLoader",
            "ParticleCombiner",
            "ParticleListManipulator",
            "TreeFitter",
            "DuplicateVertexMarker",
            "ParticleSelector"]
        expected_lists = ['V0', 'all', 'RD', 'merged']
        self._check_list(std_function=stdV0s.mergedKshorts, expected_modules=expected_modules, expected_lists=expected_lists)

    def test_belle_list(self):
        """check that the builder function works with the legacy Belle Kshorts list"""
        self._check_list(std_function=stdV0s.goodBelleKshort, expected_lists=["legacyGoodKS"])

    def test_stdlambdas_list(self):
        """check that the builder function works with the stdLambdas list"""
        expected_modules = ["ParticleLoader", "TreeFitter", "ParticleSelector"]
        expected_lists = ['all']
        self._check_list(std_function=stdV0s.stdLambdas, expected_modules=expected_modules, expected_lists=expected_lists)

    def test_mergedlambdas_list(self):
        """check that the builder function works with the merged Lambdas list"""
        expected_modules = [
            "ParticleLoader",
            "ParticleLoader",
            "ParticleLoader",
            "ParticleCombiner",
            "ParticleListManipulator",
            "TreeFitter",
            "DuplicateVertexMarker",
            "ParticleSelector"]
        expected_lists = ['V0', 'all', 'all', 'RD', 'merged']
        self._check_list(std_function=stdV0s.mergedLambdas, expected_modules=expected_modules, expected_lists=expected_lists)


if __name__ == '__main__':
    unittest.main()

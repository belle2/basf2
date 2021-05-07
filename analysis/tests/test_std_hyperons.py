#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from basf2 import create_path
from stdHyperons import stdXi, stdXi0, stdOmega, goodXi, goodXi0, goodOmega
from b2bii import setB2BII, unsetB2BII


class TestStdHyperons(unittest.TestCase):
    """Test case for standard hyperon lists"""

    def remove_spaces(self, s):
        """Remove all spaces within a string"""
        return "".join(s.split(' '))

    def _check_list(
            self,
            std_function,
            expected_lists=[]):
        """
        Check the given particle lists are created.
        The std_function ONLY takes one path argument.
        """
        testpath = create_path()
        std_function(path=testpath)

        built_lists = []
        for module in testpath.modules():
            for param in module.available_params():
                if module.type() == 'ParticleLoader' and param.name == 'decayStringsWithCuts':
                    listname = param.values[0][0].split('->')[0]
                elif module.type() == 'ParticleListManipulator' and param.name == 'outputListName':
                    listname = str(param.values).split(' -> ')[0]
                elif module.type() == 'ParticleCombiner' and param.name == 'decayString':
                    listname = param.values.split(' -> ')[0]
                else:
                    continue
                listname = self.remove_spaces(listname)
                if listname not in built_lists:
                    built_lists.append(listname)
        expected_lists = [self.remove_spaces(listname) for listname in expected_lists]

        for expected in expected_lists:
            if expected not in built_lists:
                return False
        return True

    def test_stdXi(self):
        """Check stdXi"""
        for fitter in ['KFit', 'TreeFit']:
            self.assertTrue(self._check_list(lambda path: stdXi(fitter=fitter, path=path), expected_lists=['Xi-:std']))
        # Also serves as a test for _check_list
        self.assertFalse(
            self._check_list(
                lambda path: stdXi(
                    fitter='TreeFit',
                    path=path),
                expected_lists=['Xi-:good']))
        # Allow spaces in particle list
        self.assertTrue(
            self._check_list(
                lambda path: stdXi(
                    fitter='TreeFit',
                    path=path),
                expected_lists=['Xi-  : std']))

    def test_stdXi_b2bii(self):
        """Check stdXi for B2BII settings"""
        setB2BII()
        for fitter in ['KFit', 'TreeFit']:
            self.assertTrue(self._check_list(lambda path: stdXi(fitter=fitter, path=path), expected_lists=['Xi-:std']))
        unsetB2BII()

    def test_stdXi0(self):
        """Check stdXi0"""
        for gamma_efficiency in ['eff20', 'eff30', 'eff40', 'eff50', 'eff60']:
            self.assertTrue(
                self._check_list(
                    lambda path: stdXi0(
                        gammatype=gamma_efficiency,
                        path=path),
                    expected_lists=['Xi0:std']))

    def test_stdXi0_b2bii(self):
        """Check stdXi0 for B2BII settings"""
        setB2BII()
        for gamma_efficiency in ['eff20', 'eff30', 'eff40', 'eff50', 'eff60']:
            self.assertTrue(
                self._check_list(
                    lambda path: stdXi0(
                        gammatype=gamma_efficiency,
                        path=path),
                    expected_lists=['Xi0:std']))
        unsetB2BII()

    def test_stdOmega(self):
        """Check stdOmega"""
        for fitter in ['KFit', 'TreeFit']:
            self.assertTrue(
                self._check_list(
                    lambda path: stdOmega(
                        fitter=fitter,
                        path=path),
                    expected_lists=['Omega-:std']))

    def test_stdOmega_b2bii(self):
        """Check stdOmega for B2BII settings"""
        setB2BII()
        for fitter in ['KFit', 'TreeFit']:
            self.assertTrue(
                self._check_list(
                    lambda path: stdOmega(
                        fitter=fitter,
                        path=path),
                    expected_lists=['Omega-:std']))
        unsetB2BII()

    def test_goodXi(self):
        """Check goodXi lists: veryloose, loose, tight"""
        for xitype in ['veryloose', 'loose', 'tight']:
            def create_list(path):
                goodXi(xitype, path)
            self.assertTrue(self._check_list(create_list, expected_lists=['Xi-:std', f'Xi-:{xitype}']))
            # Should be no 'Xi' list. Make sure we did not make typos.
            self.assertFalse(self._check_list(create_list, expected_lists=[f'Xi:{xitype}']))

    def test_goodXi_with_std_added_before(self):
        """Check goodXi lists: veryloose, loose, tight. Test behavior if std list was already added before."""
        for xitype in ['veryloose', 'loose', 'tight']:
            def create_list(path):
                stdXi(path=path)
                goodXi(xitype, path)
            self.assertTrue(self._check_list(create_list, expected_lists=['Xi-:std', f'Xi-:{xitype}']))
            # Should be no 'Xi' list. Make sure we did not make typos.
            self.assertFalse(self._check_list(create_list, expected_lists=[f'Xi:{xitype}']))

    def test_goodXi0(self):
        """Check goodXi0 lists: veryloose, loose, tight"""
        for xitype in ['veryloose', 'loose', 'tight']:
            def create_list(path):
                goodXi0(xitype, path)
            self.assertTrue(self._check_list(create_list, expected_lists=['Xi0:std', f'Xi0:{xitype}']),
                            f"xitype = {xitype}")
            self.assertFalse(self._check_list(create_list, expected_lists=[f'Xi:{xitype}']))

    def test_goodXi0_with_std_added_before(self):
        """Check goodXi0 lists: veryloose, loose, tight. Test behavior if std list was already added before."""
        for xitype in ['veryloose', 'loose', 'tight']:
            def create_list(path):
                stdXi0(path=path)
                goodXi0(xitype, path)
            self.assertTrue(self._check_list(create_list, expected_lists=['Xi0:std', f'Xi0:{xitype}']),
                            f"xitype = {xitype}")
            self.assertFalse(self._check_list(create_list, expected_lists=[f'Xi:{xitype}']))

    def test_goodOmega(self):
        """Check goodOmega lists: veryloose, loose, tight"""
        for omegatype in ['veryloose', 'loose', 'tight']:
            def create_list(path):
                goodOmega(omegatype, path)
            self.assertTrue(self._check_list(create_list, expected_lists=['Omega-:std', f'Omega-:{omegatype}']))
            self.assertFalse(self._check_list(create_list, expected_lists=[f'Omega:{omegatype}']))

    def test_goodOmega_with_std_added_before(self):
        """Check goodOmega lists: veryloose, loose, tight. Test behavior if std list was already added before."""
        for omegatype in ['veryloose', 'loose', 'tight']:
            def create_list(path):
                stdOmega(path=path)
                goodOmega(omegatype, path)
            self.assertTrue(self._check_list(create_list, expected_lists=['Omega-:std', f'Omega-:{omegatype}']))
            self.assertFalse(self._check_list(create_list, expected_lists=[f'Omega:{omegatype}']))


if __name__ == '__main__':
    unittest.main()

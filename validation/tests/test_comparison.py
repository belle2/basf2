#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# std
import unittest
import random
from typing import List, Tuple

# 3rd
import ROOT

# ours
import validationcomparison
import metaoptions


class TestGetComparison(unittest.TestCase):
    """ Test get_comparison """

    def setUp(self):
        """ Set up testing of get_comparison """
        #: Mapping test case -> Metaoptions
        self.test_options = {
            "chi2": "",
            "kolmogorov": "kolmogorov",
            "andersondarling": "andersondarling",
        }
        gaus_th1f = ROOT.TH1F("gaus", "gaus", 5, -3, 3)
        gaus_th1f.FillRandom("gaus", 1000)
        exponential_th1f = ROOT.TH1F("expo", "expo", 5, -3, 3)
        exponential_th1f.FillRandom("expo", 1000)
        #: ROOT objects used to check if comparison executes
        self.obj_pairs: List[Tuple[ROOT.TObject, ROOT.TObject, str]] = [
            (gaus_th1f, gaus_th1f, "equal"),
            (gaus_th1f, exponential_th1f, "error"),
        ]

    def test_get_comparison(self):
        """ Use get_tester on the metaoptions to get the requested
        comparison object and run that on two identical ROOT object.
        Check that this indeed returns 'equal'.
        """
        for tester_name in self.test_options:
            for objs in self.obj_pairs:
                names: Tuple[str, str] = (objs[0].GetName(), objs[1].GetName())
                with self.subTest(
                    tester=tester_name, obj1=names[0], obj2=names[1],
                ):
                    tester = validationcomparison.get_comparison(
                        objs[0],
                        objs[1],
                        metaoptions.MetaOptionParser(
                            self.test_options[tester_name].split(",")
                        ),
                    )
                    print(
                        f"{names[0]}, {names[1]}: "
                        f"{tester.comparison_result_long}. "
                        f"Short result: {tester.comparison_result}. "
                        f"Expectation: {objs[2]}"
                    )
                    self.assertEqual(tester.comparison_result, objs[2])


class TestComparison(unittest.TestCase):
    """
    Various test cases for the Chi2Test wrapper class
    """

    @staticmethod
    def create_profile(
        name, entries=5000, mu=10, sigma=0.3, max_fill=50, fixed_number=None
    ):
        """
        Create a TProfile object with various content
        """
        p = ROOT.TProfile(name, name, 50, 0, 50.0)
        entries_per_bin = int(entries / max_fill)
        for i in range(0, max_fill + 1):
            for e in range(0, entries_per_bin):
                if fixed_number is None:
                    p.Fill(i + 0.5, random.gauss(mu, sigma))
                else:
                    p.Fill(i + 0.5, fixed_number)
        return p

    @staticmethod
    def create_teff(name, bins=20, eff=0.9):
        """
        Creates and fills a root TEfficiency plot
        """
        p = ROOT.TEfficiency(name, name, bins, 0, 50)

        for i in range(0, 5000):
            passed = random.uniform(0, 1.0) < eff
            bin_content = random.uniform(0.0, 50.0)
            p.Fill(passed, bin_content)

        return p

    # todo: Can't I use FillRandom for this?
    @staticmethod
    def create_histogram(name, entries=5000, mu=10, sigma=3):
        """
        Create a TH1D and fill with random content
        """
        p = ROOT.TH1D(name, name, 50, 0, 20.0)
        for i in range(0, entries):
            p.Fill(random.gauss(mu, sigma))
        return p

    def root_name(self, name):
        """
        Generates unique names for ROOT objects
        """
        return f"{name}_{self.call_iteration}"

    def setUp(self):
        """
        Setup method to generate profiles and histograms for tests
        """
        random.seed(23)

        #: if we would at some point later want to implement several runs, we
        #: use this as a counter variable to set up different names.
        #: However not implemented yet.
        self.call_iteration = 0

        #: Profile A
        self.profileA = self.create_profile(self.root_name("profileA"))

        #: Profile B
        self.profileB = self.create_profile(self.root_name("profileB"))

        #: Profile C
        self.profileC = self.create_profile(
            self.root_name("profileC"), 5000, 5, 3
        )

        #: Profile with bins with 0 error
        self.profileZeroErrorBins = self.create_profile(
            self.root_name("profileZeroErrorBins"), max_fill=49
        )
        self.profileZeroErrorBins.SetBinError(35, 0.0)

        #: Profile with bins with 0 error
        self.profileZeroErrorBinsTwo = self.create_profile(
            self.root_name("profileZeroErrorBinsTwo"), max_fill=49
        )
        self.profileZeroErrorBinsTwo.SetBinError(35, 0.0)

        #: Histogram A
        self.histogramA = self.create_histogram(
            self.root_name("histogramA"), 5000, 5, 3
        )

        #: Histogram B (should be almost equal to profile A)
        self.histogramB = self.create_histogram(
            self.root_name("histogramB"), 5000, 5, 3
        )

        #: Profile should be almost equal to A
        self.profileAequal = self.create_profile(
            self.root_name("profileA_almostequal"), sigma=0.4
        )

        #:  Profile should be almost equal to B
        self.profileBequal = self.create_profile(
            self.root_name("profileB_almostequal"), sigma=0.4
        )

        #: Profile with different bins
        self.profileDifferentBins = ROOT.TProfile(
            self.root_name("profileDifferentBins"),
            self.root_name("profileDifferentBins"),
            40,
            0,
            50.0,
        )

        #: TEfficiemcy A
        self.teffA = self.create_teff(self.root_name("teffA"))

        #: TEfficiency B
        self.teffB = self.create_teff(self.root_name("teffB"))

    def test_compare_profiles(self):
        """
        Test if comparing two similar TProfiles works
        """
        c = validationcomparison.Chi2Test(self.profileA, self.profileB)

        self.assertTrue(c.can_compare())
        c.ensure_compute()
        self.assertAlmostEqual(c._pvalue, 0.22495088947037362)

    def test_compare_profiles_identical(self):
        """
        Test if comparing two identical TProfiles works
        """
        c = validationcomparison.Chi2Test(self.profileA, self.profileA)

        self.assertTrue(c.can_compare())
        c.ensure_compute()
        self.assertAlmostEqual(c._pvalue, 1)

    def test_compare_identical_profiles_kolmogorov(self):
        """ Test if comparing to identical TProfiles with Kolmo Test works"""
        c = validationcomparison.KolmogorovTest(self.profileA, self.profileA)
        self.assertTrue(c.can_compare())
        c.ensure_compute()
        self.assertAlmostEqual(c._pvalue, 1)

    def test_compare_profiles_almost_equal(self):
        """
        Test if the comparison of two TProfiles with very similar content works
        """
        c = validationcomparison.Chi2Test(
            self.profileAequal, self.profileBequal
        )

        self.assertTrue(c.can_compare())
        c.ensure_compute()
        self.assertAlmostEqual(c._pvalue, 0.43093514577898634)
        self.assertAlmostEqual(c._ndf, 49)

    def test_compare_zero_error_profiles(self):
        """
        Test if bins with zero error are treated properly
        """

        # add entry in the last bin, which will increase the bin weight,
        # but leave the error at zero
        self.profileZeroErrorBins.Fill(49.8, 1.0)
        self.profileZeroErrorBins.Fill(49.8, 1.0)

        # the comparison should set the bin content of this bin to zero
        # to disable comparison of this bin instead of
        # not doing the comparison at all
        c = validationcomparison.Chi2Test(
            self.profileZeroErrorBins, self.profileZeroErrorBinsTwo
        )

        self.assertTrue(c.can_compare())

        c.ensure_compute()

        self.assertAlmostEqual(c._pvalue, 0.4835651485797353)
        # should still be only 49 ndf
        self.assertEqual(c._ndf, 49)

    def test_compare_histograms(self):
        """
        Test comparison of regular histograms
        """

        c = validationcomparison.Chi2Test(self.histogramA, self.histogramB)

        self.assertTrue(c.can_compare())
        c.ensure_compute()
        self.assertAlmostEqual(c._pvalue, 0.371600562118221)
        self.assertAlmostEqual(c._chi2, 42.308970111484086)
        self.assertAlmostEqual(c._chi2ndf, 1.0577242527871022)
        self.assertEqual(c._ndf, 40)

    def test_compare_unsupported_object(self):
        """
        Test if unsupported ROOT objects are treated properly
        """
        obj_not_supported = ROOT.TH2D("h2d", "h2d", 50, 0, 50, 50, 0, 50)
        c = validationcomparison.Chi2Test(self.profileA, obj_not_supported)
        self.assertFalse(c.can_compare())

    def test_compare_differing_objects(self):
        """
        Test if the comparison of differing objects is rejected
        """
        c = validationcomparison.Chi2Test(self.profileA, self.histogramA)
        self.assertFalse(c.can_compare())

        with self.assertRaises(validationcomparison.ObjectsNotSupported):
            c._compute()

    def test_compare_tefficiencies(self):
        """
        Test if two TEfficiency objects can be compared. Is a bit tricky
        as TEfficiency does not support
        """

        c = validationcomparison.Chi2Test(self.teffA, self.teffB)

        self.assertTrue(c.can_compare())
        c.ensure_compute()
        self.assertAlmostEqual(c._pvalue, 0.9760318312199932)
        self.assertAlmostEqual(c._chi2, 8.16784873)
        self.assertAlmostEqual(c._chi2ndf, 0.45376937)
        self.assertEqual(c._ndf, 18)

    def test_compare_tefficiencies_same(self):
        """
        Test if two TEfficiency objects can be compared. Is a bit tricky as
        TEfficiency does not support Comparing the exact same TEfficiency
        object should give back 100% agreement
        """

        c = validationcomparison.Chi2Test(self.teffA, self.teffA)
        self.assertTrue(c.can_compare())
        c.ensure_compute()
        self.assertAlmostEqual(c._pvalue, 1.0)
        self.assertAlmostEqual(c._chi2, 0.0)
        self.assertAlmostEqual(c._chi2ndf, 0.0)
        self.assertEqual(c._ndf, 18)

    def test_compare_differing_bins(self):
        """
        Test if the comparison attempt of profiles with differing bin count
        fails properly
        """
        c = validationcomparison.Chi2Test(
            self.profileA, self.profileDifferentBins
        )
        self.assertFalse(c.can_compare())

        with self.assertRaises(validationcomparison.DifferingBinCount):
            c._compute()


if __name__ == "__main__":
    unittest.main(verbosity=2)

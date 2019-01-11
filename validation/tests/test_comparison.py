#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import validationcomparison
import ROOT
import random


class TestComparison(unittest.TestCase):

    """
    Various test cases for the Chi2Test wrapper class
    """

    @staticmethod
    def create_profile(name, entries=5000, mu=10, sigma=0.3, max_fill=50,
                       fixed_number=None):
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
            p.Fill(passed, i)

        return p

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
        return "{}_{}".format(name, self.call_iteration)

    def setUp(self):
        """
        Setup method to generate profiles and histograms for tests
        """
        random.seed(23)
        self.call_iteration = 0

        p_a = self.create_profile(self.root_name("profileA"))
        #: store for later use
        self.profileA = p_a

        p_b = self.create_profile(self.root_name("profileB"))
        #: store for later use
        self.profileB = p_b

        p_c = self.create_profile(self.root_name("profileC"), 5000, 5, 3)
        #: store for later use
        self.profileC = p_c

        p_zero_error_bins = self.create_profile(
            self.root_name("profileZeroErrorBins"),
            max_fill=49
        )
        p_zero_error_bins.SetBinError(35, 0.0)
        #: store for later use
        self.profileZeroErrorBins = p_zero_error_bins

        p_zero_error_bins_two = self.create_profile(
            self.root_name("profileZeroErrorBinsTwo"),
            max_fill=49
        )
        p_zero_error_bins_two.SetBinError(35, 0.0)
        #: store for later use
        self.profileZeroErrorBinsTwo = p_zero_error_bins_two

        h_a = self.create_histogram(self.root_name("histogramA"), 5000, 5, 3)
        #: store for later use
        self.histogramA = h_a

        h_b = self.create_histogram(self.root_name("histogramB"), 5000, 5, 3)
        #: store for later use
        self.histogramB = h_b

        p_aequal = self.create_profile(
            self.root_name("profileA_almostequal"),
            sigma=0.4
        )
        #: store for later use
        self.profile_aequal = p_aequal

        p_bequal = self.create_profile(
            self.root_name("profileB_almostequal"),
            sigma=0.4
        )
        #: store for later use
        self.profile_bequal = p_bequal

        #: store for later use
        self.profile_different_bins = \
            ROOT.TProfile(
                self.root_name("profileDifferentBins"),
                self.root_name("profileDifferentBins"),
                40, 0, 50.0
            )

        p_a = self.create_teff(self.root_name("teffA"))
        #: store for later use
        self.teffA = p_a

        p_b = self.create_teff(self.root_name("teffB"))
        #: store for later use
        self.teffB = p_b

        self.call_iteration += 1

    def test_compare_profiles(self):
        """
        Test if comparing two similar TProfiles works
        """
        c = validationcomparison.Chi2Test(self.profileA, self.profileB)

        self.assertTrue(c.can_compare())
        self.assertAlmostEqual(c.pvalue(), 0.22495088947037362)

    def test_compare_profiles_almost_equal(self):
        """
        Test if the comparison of two TProfiles with very similar content works
        """
        c = validationcomparison.Chi2Test(
            self.profile_aequal,
            self.profile_bequal
        )

        self.assertTrue(c.can_compare())
        self.assertAlmostEqual(c.pvalue(), 0.43093514577898634)
        self.assertAlmostEqual(c.ndf(), 49)

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
            self.profileZeroErrorBins,
            self.profileZeroErrorBinsTwo
        )

        self.assertTrue(c.can_compare())
        pvalue = c.pvalue()
        # chi2 = c.chi2()
        # chi2ndf = c.chi2ndf()

        self.assertAlmostEqual(pvalue, 0.4835651485797353)
        # should still be only 49 ndf
        self.assertAlmostEqual(c.ndf(), 49)

    def test_compare_histograms(self):
        """
        Test comparison of regular histograms
        """

        c = validationcomparison.Chi2Test(self.histogramA, self.histogramB)

        self.assertTrue(c.can_compare())
        self.assertAlmostEqual(c.pvalue(), 0.371600562118221)
        self.assertAlmostEqual(c.chi2(), 42.308970111484086)
        self.assertAlmostEqual(c.chi2ndf(), 1.0577242527871022)
        self.assertAlmostEqual(c.ndf(), 40)

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
            c.pvalue()

    def test_compare_tefficiencies(self):
        """
        Test if two TEfficiency objects can be compared. Is a bit tricky
        as TEfficiency does not support
        """

        c = validationcomparison.Chi2Test(self.teffA, self.teffB)
        self.assertTrue(c.can_compare())

        self.assertAlmostEqual(c.pvalue(), 0.9999987236358295)
        self.assertAlmostEqual(c.chi2(), 2.0313602336641985)
        self.assertAlmostEqual(c.chi2ndf(), 0.11285334631467769)
        self.assertAlmostEqual(c.ndf(), 18)

    def test_compare_tefficiencies_same(self):
        """
        Test if two TEfficiency objects can be compared. Is a bit tricky as
        TEfficiency does not support Comparing the exact same TEfficiency
        object should give back 100% agreement
        """

        c = validationcomparison.Chi2Test(self.teffA, self.teffA)
        self.assertTrue(c.can_compare())

        self.assertAlmostEqual(c.pvalue(), 1.0)
        self.assertAlmostEqual(c.chi2(), 0.0)
        self.assertAlmostEqual(c.chi2ndf(), 0.0)
        self.assertAlmostEqual(c.ndf(), 18)

    def test_compare_differing_bins(self):
        """
        Test if the comparison attempt of profiles with differing bin count
        fails properly
        """
        c = validationcomparison.Chi2Test(
            self.profileA,
            self.profile_different_bins
        )
        self.assertFalse(c.can_compare())

        with self.assertRaises(validationcomparison.DifferingBinCount):
            c.pvalue()


if __name__ == "__main__":
    unittest.main()

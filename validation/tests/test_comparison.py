#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import validationcomparison
import ROOT
import random
import math

call_iteration = 0


class TestComparison(unittest.TestCase):

    """
    Various test cases for the Chi2Test wrapper class
    """

    def create_profile(self, name, entries=5000, mu=10, sigma=0.3, max_fill=50, fixed_number=None):
        """
        Create a TProfile object with various content
        """
        p = ROOT.TProfile(name, name, 50, 0, 50.0)
        entries_per_bin = math.floor(entries / max_fill)
        for i in range(0, max_fill + 1):
            for e in range(0, entries_per_bin):
                if fixed_number is None:
                    p.Fill(i + 0.5, random.gauss(mu, sigma))
                else:
                    p.Fill(i + 0.5, fixed_number)
        return p

    def create_histogram(self, name, entries=5000, mu=10, sigma=3):
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
        global call_iteration
        return "{}_{}".format(name, call_iteration)

    def setUp(self):
        """
        Setup method to generate profiles and histograms for tests
        """
        random.seed(23)
        global call_iteration

        pA = self.create_profile(self.root_name("profileA"))
        #: store for later use
        self.profileA = pA

        pB = self.create_profile(self.root_name("profileB"))
        #: store for later use
        self.profileB = pB

        pC = self.create_profile(self.root_name("profileC"), 5000, 5, 3)
        #: store for later use
        self.profileC = pC

        pZeroErrorBins = self.create_profile(self.root_name("profileZeroErrorBins"), max_fill=49)
        pZeroErrorBins.SetBinError(35, 0.0)
        #: store for later use
        self.profileZeroErrorBins = pZeroErrorBins

        pZeroErrorBinsTwo = self.create_profile(self.root_name("profileZeroErrorBinsTwo"), max_fill=49)
        pZeroErrorBinsTwo.SetBinError(35, 0.0)
        #: store for later use
        self.profileZeroErrorBinsTwo = pZeroErrorBinsTwo

        hA = self.create_histogram(self.root_name("histogramA"), 5000, 5, 3)
        #: store for later use
        self.histogramA = hA

        hB = self.create_histogram(self.root_name("histogramB"), 5000, 5, 3)
        #: store for later use
        self.histogramB = hB

        pAequal = self.create_profile(self.root_name("profileA_almostequal"), sigma=0.4)
        #: store for later use
        self.profileAequal = pAequal

        pBequal = self.create_profile(self.root_name("profileB_almostequal"), sigma=0.4)
        #: store for later use
        self.profileBequal = pBequal

        #: store for later use
        self.profileDifferentBins = ROOT.TProfile(self.root_name("profileDifferentBins"),
                                                  self.root_name("profileDifferentBins"), 40, 0, 50.0)
        call_iteration += 1

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
        c = validationcomparison.Chi2Test(self.profileAequal, self.profileBequal)

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
        c = validationcomparison.Chi2Test(self.profileZeroErrorBins, self.profileZeroErrorBinsTwo)

        self.assertTrue(c.can_compare())
        pvalue = c.pvalue()
        chi2 = c.chi2()
        chi2ndf = c.chi2ndf()

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
        objNotSupported = ROOT.TH2D("h2d", "h2d", 50, 0, 50, 50, 0, 50)
        c = validationcomparison.Chi2Test(self.profileA, objNotSupported)
        self.assertFalse(c.can_compare())

    def test_compare_differing_objects(self):
        """
        Test if the comparison of differing objects is rejected
        """
        c = validationcomparison.Chi2Test(self.profileA, self.histogramA)
        self.assertFalse(c.can_compare())

        with self.assertRaises(validationcomparison.ObjectsNotSupported):
            c.pvalue()

    def test_compare_differing_bins(self):
        """
        Test if the comparison attempt of profiles with differing bin count
        fails properly
        """
        c = validationcomparison.Chi2Test(self.profileA, self.profileDifferentBins)
        self.assertFalse(c.can_compare())

        with self.assertRaises(validationcomparison.DifferingBinCount):
            c.pvalue()


if __name__ == "__main__":
    unittest.main()

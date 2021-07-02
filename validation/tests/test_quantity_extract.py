#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import unittest
import ROOT

from quantity_extract import RootQuantityExtract, default_extractor


class TestQuantityExtract(unittest.TestCase):

    """
    Various test cases for the Quantity Extractor class
    """

    def test_h1d(self):
        """
        Test getting the quantities of a TH1d
        """

        h1 = ROOT.TH1D("ext_test1", "ext_test1", 40, 0, 40.0)
        h1.Fill(10.0)
        h1.Fill(20.0)
        h1.Fill(30.0)

        rext = RootQuantityExtract(default_extractor())
        res = rext.extract(h1)

        self.assertIn("mean_x", res)
        self.assertAlmostEqual(res["mean_x"], 20)
        self.assertIn("mean_y", res)
        self.assertAlmostEqual(res["mean_y"], 0.075)
        self.assertIn("mean_y_zero_suppressed", res)
        self.assertAlmostEqual(res["mean_y_zero_suppressed"], 1.0)
        self.assertIn("entries", res)

    def test_profile(self):
        """
        Test getting the quantities of a TProfile
        """

        h1 = ROOT.TProfile("ext_test1", "ext_test1", 40, 0, 40.0)
        h1.Fill(10.0, 4.0)
        h1.Fill(20.0, 5.0)
        h1.Fill(30.0, 6.0)

        rext = RootQuantityExtract(default_extractor())
        res = rext.extract(h1)

        self.assertIn("mean_y", res)
        self.assertAlmostEqual(res["mean_y"], 0.375)
        self.assertIn("mean_y_zero_suppressed", res)
        self.assertAlmostEqual(res["mean_y_zero_suppressed"], 5.0)

    def test_ntuple(self):
        """
        Test getting the quanities contained in a TNtuple
        """

        tn = ROOT.TNtuple("particle_list", "particle_list", "x:y:z:energy")
        tn.Fill(5, 6, 7, 10)

        rext = RootQuantityExtract(default_extractor())
        res = rext.extract(tn)
        self.assertEqual(len(res), 4)
        self.assertIn("particle_list_z", res)
        self.assertAlmostEqual(res["particle_list_z"], 7.0)


if __name__ == "__main__":
    unittest.main()

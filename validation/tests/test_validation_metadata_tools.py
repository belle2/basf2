#!/usr/bin/env python3

# std
import unittest
import tempfile
import pathlib

# 3rd
import ROOT
import basf2 as b2
import modularAnalysis as ma
import generators as ge

# ours
from validation_tools.metadata import create_validation_histograms


TRIVIAL_DECFILE = """
Alias MyB+ B+
Alias MyB- B-

Decay Upsilon(4S)
1.0 MyB+ MyB- VSS;
Enddecay

End
"""


class TestValidationMetadataSetter(unittest.TestCase):
    """ This test tests the ValidationMetadataSetter module via its interface
    :func:`validation_tools.metadata.create_validation_histograms`
    """
    def setUp(self):
        self.tmp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.tmp_dir.cleanup()

    def test(self):
        tmp_dir_path = pathlib.Path(self.tmp_dir.name)

        dec_path = tmp_dir_path / "test_y4s_trivial.dec"
        with dec_path.open("w") as decfile:
            decfile.write(TRIVIAL_DECFILE)

        # Run basf2
        # ----------------------------------------------------------------------

        path = b2.create_path()
        ma.setupEventInfo(noEvents=10, path=path)
        ge.add_evtgen_generator(
            path=path,
            finalstate='signal',
            signaldecfile=b2.find_file(str(dec_path))
        )

        ma.loadGearbox(path=path)

        ma.fillParticleListFromMC("Upsilon(4S)", "", path=path)

        out_file_path = tmp_dir_path / "out.root"
        create_validation_histograms(
            path,
            out_file_path,
            "Upsilon(4S)",
            variables_1d=[
                (
                    "M", 100, 5, 15, "mass", "me <wontreply@dont.try>",
                    "description of M", "nothing to check",
                    "x label"
                )
            ], variables_2d=[
                (
                    "M", 100, 5, 15, "M", 100, 5, 15, "mass vs mass",
                    "me <wontreply@dont.try>", "some description nobody reads",
                    "nothing to check", "x label", "why label?", "mop1, mop2"
                )
            ]
        )

        b2.process(path=path)

        # Test outcome
        # ----------------------------------------------------------------------

        tf = ROOT.TFile(str(out_file_path))

        # 1D Histogram
        # ************

        m = tf.Get("M")
        self.assertEqual(
            m.FindObject("Description").GetTitle(), "description of M"
        )
        self.assertEqual(
            m.FindObject("Check").GetTitle(), "nothing to check"
        )
        self.assertEqual(
            m.FindObject("MetaOptions").GetTitle(), ""
        )
        self.assertEqual(
            m.FindObject("Contact").GetTitle(), "me <wontreply@dont.try>"
        )

        # 2D Histogram
        # ************

        mm = tf.Get("MM")
        self.assertEqual(
            mm.FindObject("Description").GetTitle(),
            "some description nobody reads"
        )
        self.assertEqual(
            mm.FindObject("Check").GetTitle(), "nothing to check"
        )
        self.assertEqual(
            mm.FindObject("MetaOptions").GetTitle(), "mop1, mop2"
        )
        self.assertEqual(
            mm.FindObject("Contact").GetTitle(), "me <wontreply@dont.try>"
        )


if __name__ == "__main__":
    unittest.main()

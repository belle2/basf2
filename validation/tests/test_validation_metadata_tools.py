#!/usr/bin/env python3

# std
import unittest
import tempfile
import pathlib

# 3rd
import ROOT
import basf2
import modularAnalysis
import generators as ge

# ours
from validation_tools.metadata import create_validation_histograms
from validationplots import get_metadata


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
        """ Open temporary directory to work in. """
        #: Temporary directory
        self.tmp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        """ Clean up temporary directory """
        self.tmp_dir.cleanup()

    def test(self):
        """ Perform tests """
        tmp_dir_path = pathlib.Path(self.tmp_dir.name)

        dec_path = tmp_dir_path / "test_y4s_trivial.dec"
        with dec_path.open("w") as decfile:
            decfile.write(TRIVIAL_DECFILE)

        # Run basf2
        # ----------------------------------------------------------------------

        path = basf2.create_path()
        modularAnalysis.setupEventInfo(noEvents=10, path=path)
        ge.add_evtgen_generator(
            path=path,
            finalstate="signal",
            signaldecfile=basf2.find_file(str(dec_path)),
        )

        modularAnalysis.loadGearbox(path=path)

        modularAnalysis.fillParticleListFromMC("Upsilon(4S)", "", path=path)

        out_file_path = tmp_dir_path / "out.root"
        create_validation_histograms(
            path,
            out_file_path,
            "Upsilon(4S)",
            variables_1d=[
                (
                    "M",
                    100,
                    5,
                    15,
                    "mass",
                    "me <wontreply@dont.try>",
                    "description of M",
                    "nothing to check",
                    "x label",
                )
            ],
            variables_2d=[
                (
                    "M",
                    100,
                    5,
                    15,
                    "M",
                    100,
                    5,
                    15,
                    "mass vs mass",
                    "me <wontreply@dont.try>",
                    "some description nobody reads",
                    "nothing to check",
                    "x label",
                    "why label?",
                    "mop1, mop2",
                )
            ],
            description="Overall description of plots in this package.",
        )

        basf2.process(path=path)

        # Test outcome
        # ----------------------------------------------------------------------

        tf = ROOT.TFile(str(out_file_path))

        # Overall
        # *******

        d = tf.Get("Description")
        self.assertEqual(
            d.GetTitle(), "Overall description of plots in this package."
        )

        # 1D Histogram
        # ************

        md = get_metadata(tf.Get("M"))
        self.assertEqual(md["description"], "description of M")
        self.assertEqual(md["check"], "nothing to check")
        self.assertEqual(md["metaoptions"], [])
        self.assertEqual(md["contact"], "me <wontreply@dont.try>")

        # 2D Histogram
        # ************

        md = get_metadata(tf.Get("MM"))
        self.assertEqual(md["description"], "some description nobody reads")
        self.assertEqual(md["check"], "nothing to check")
        self.assertEqual(md["metaoptions"], ["mop1", "mop2"])
        self.assertEqual(md["contact"], "me <wontreply@dont.try>")


if __name__ == "__main__":
    unittest.main()

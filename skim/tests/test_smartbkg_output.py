#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test that the SmartBackground module runs without error and produces expected results
"""

import unittest
from basf2 import create_path, find_file
import modularAnalysis as ma
from variables import variables as var
from skim.WGs.fei import feiHadronic
from skim.WGs.lowMulti import LowMassOneTrack
from skim import smartbkg as sbg
import b2test_utils
from ROOT import TFile


def get_smartbkg_path_with_params(file_path, skim, event_type, variable_name):
    """
    Returns a simple basf2 path that reads an mdst file with test events,
    runs the SmartBackground module, and writes the predictions to a
    test root file.
    """

    path = create_path()
    ma.inputMdstList([file_path], path=path)

    sbg.add_smartbkg_filtering(
        skim=skim,
        path=path,
        debug_mode=True,
        event_type=event_type
    )

    ma.variablesToNtuple(
        "",
        [variable_name],
        treename="predictions",
        filename="test_predictions.root",
        path=path
    )

    return path


class TestSmartbkgOutput(unittest.TestCase):

    def test_smartbkg_outputs(self):
        """
        Check that SmartBackground fails if provided unknown skim or no event type, succeds
        if passed everything correctly and produces expected predictions on a tiny dataset.
        """

        file_path = b2test_utils.require_file("skim/tests/smartbkg_test_events.mdst.root", py_case=self)

        skim1 = feiHadronic()
        skim2 = LowMassOneTrack()

        var.addAlias("smartBkgPrediction1", "eventExtraInfo(SmartBKG_Prediction_feiHadronic)")
        var.addAlias("smartBkgPrediction2", "eventExtraInfo(SmartBKG_Prediction_LowMassOneTrack)")

        parameters = [
            [file_path, skim1, None, "smartBkgPrediction1"],       # BAD: event type cannot be inferred
            [file_path, skim2, "charged", "smartBkgPrediction2"],  # BAD: skim not known to model
            [file_path, skim1, "charged", "smartBkgPrediction1"]   # GOOD: should run without error
        ]

        results_expected = [1, 1, 0]
        values_expected = [0.06365067, 1., 0.01, 0.5620032, 0.0962249, 0.14172044, 0.01308167, 1., 0.69878286, 1.]

        with b2test_utils.clean_working_directory():
            results = [
                b2test_utils.safe_process(get_smartbkg_path_with_params(*parameters[i])) for i in range(3)
            ]

            self.assertEqual(
                results,
                results_expected,
                msg=f"Smart Background module does not produce correct exit codes: expected {results_expected} " +
                f"but got {results}."
            )

            output_file_path = find_file("test_predictions.root", silent=True)
            self.assertNotEqual(output_file_path, "", msg="Could not find Smart Background module output file.")

            file = TFile(output_file_path)
            values = [entry.smartBkgPrediction1 for entry in file.Get("predictions")]

            for i in range(len(values)):
                self.assertAlmostEqual(
                    values[i],
                    values_expected[i],
                    places=5,
                    msg="Smart Background module does not produce correct predictions on feiHadronic skim: " +
                    f"expected {values_expected} but got {values}"
                )


if __name__ == "__main__":
    unittest.main()

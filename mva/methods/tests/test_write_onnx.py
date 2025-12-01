#!/usr/bin/env python3

from pathlib import Path
import unittest

import basf2
import b2test_utils
from basf2_mva_util import create_onnx_mva_weightfile
import torch
from torch import nn


def save_onnx(model, filename, variables, **kwargs):
    """
    Export a torch model to onnx and write it into a MVA weightfile
    """
    import ROOT  # noqa

    print("convert to onnx")
    torch.onnx.export(
        model,
        (torch.zeros(1, len(variables)),),
        "model.onnx",
        input_names=["input"],
        output_names=["output"],
    )
    weightfile = create_onnx_mva_weightfile("model.onnx", variables=variables, **kwargs)
    print(f"save to {filename}")
    weightfile.save(filename)


class TestWriteONNX(unittest.TestCase):
    """
    Tests for writing ONNX MVA weightfiles. In addition to testing the writing
    mechanism, these serve the purpose of creating test files for other unit tests.
    """

    #: show long diff in case of mismatching xml files
    maxDiff = None

    def create_and_save(self, n_outputs, filename, weights):
        """
        Setup some example options for an ONNX MVA weightfile, save it and compare to reference
        """

        # like in mva/tests/all_classifiers.py
        variables = [
            "p",
            "pz",
            "daughter(0, p)",
            "daughter(0, pz)",
            "daughter(1, p)",
            "daughter(1, pz)",
            "chiProb",
            "dr",
            "dz",
            "daughter(0, dr)",
            "daughter(1, dr)",
            "daughter(0, chiProb)",
            "daughter(1, chiProb)",
            "daughter(0, kaonID)",
            "daughter(0, pionID)",
            "daughterAngle(0, 1)",
        ]
        model = nn.Linear(len(variables), n_outputs)
        model.load_state_dict(weights)
        if n_outputs > 1:
            nClasses = n_outputs
        else:
            nClasses = 2
        with b2test_utils.clean_working_directory():
            save_onnx(
                model,
                filename,
                variables=variables,
                datafiles=["dummy"],
                identifier="Simple",
                treename="tree",
                nClasses=nClasses,
            )
            with open(filename) as f:
                xml_new = f.read()
        ref_path = Path(basf2.find_file("mva/methods/tests")) / filename
        try:
            with open(ref_path) as f:
                xml_ref = f.read()
        except FileNotFoundError:
            # if the file does not exist, recreate it, but still fail the test
            #
            # This has to be done when new options are added to ONNXOptions and
            # therefore the xml changes. In this case, just delete the xmls and
            # rerun the test to generate new reference files.
            with open(ref_path, "w") as f:
                f.write(xml_new)
            raise Exception(f"Wrote new reference file {str(ref_path)}")
        self.assertEqual(xml_new, xml_ref)

    def test_singleclass(self):
        """
        Write example for single output
        """
        self.create_and_save(
            1,
            "ONNX.xml",
            {
                "weight": torch.tensor(
                    [[0.1911,  0.2075, -0.0586,  0.2297, -0.0548,  0.0504, -0.1217,  0.1468,
                      0.2204, -0.1834,  0.2173,  0.0468,  0.1847,  0.0339,  0.1205, -0.0353]]
                ),
                "bias": torch.tensor([0.1927])
            }
        )

    def test_multiclass(self):
        """
        Write example for multiclass outputs
        """
        self.create_and_save(
            2,
            "ONNX_multiclass.xml",
            {
                "weight": torch.tensor(
                    [[0.1911,  0.2075, -0.0586,  0.2297, -0.0548,  0.0504, -0.1217,  0.1468,
                      0.2204, -0.1834,  0.2173,  0.0468,  0.1847,  0.0339,  0.1205, -0.0353],
                     [0.1927,  0.0370, -0.1167,  0.0637, -0.1152, -0.0293, -0.1015,  0.1658,
                      -0.1973, -0.1153, -0.0706, -0.1503,  0.0236, -0.2469,  0.2258, -0.2124]]
                ),
                "bias": torch.tensor([0.1930, 0.0416])
            }
        )

    def test_multiclass_3(self):
        """
        Write example for 3-class outputs
        """
        self.create_and_save(
            3,
            "ONNX_multiclass_3.xml",
            {
                "weight": torch.tensor(
                    [[-0.1648,  0.2103,  0.0204, -0.1267, -0.0719, -0.2464, -0.1342, -0.0418,
                      -0.0362, -0.0801,  0.0587, -0.1121, -0.1560, -0.1602,  0.1597,  0.1568],
                     [-0.2297, -0.1780, -0.0301, -0.2094, -0.1600,  0.1508,  0.1964,  0.1261,
                      -0.0792,  0.0605, -0.0064,  0.0450,  0.0671, -0.2036,  0.0768,  0.0442],
                     [-0.1490, -0.2286,  0.2232, -0.1404,  0.2207, -0.0696, -0.2392,  0.1917,
                      0.0795, -0.1850,  0.0989, -0.0802,  0.0483,  0.0772,  0.1347, -0.1316]]
                ),
                "bias": torch.tensor([-0.1484,  0.1209, -0.0164])
            }
        )


if __name__ == "__main__":
    unittest.main()

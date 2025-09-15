#!/usr/bin/env python3

from pathlib import Path
import unittest

import basf2
import b2test_utils
import basf2_mva
import torch
from torch import nn


def save_onnx(model, general_options, specific_options, identifier):
    """
    Export a torch model to onnx and write it into a MVA weightfile
    """
    import ROOT  # noqa

    print("convert to onnx")
    torch.onnx.export(
        model,
        (torch.zeros(1, len(general_options.m_variables)),),
        "model.onnx",
        input_names=["input"],
        output_names=["output"],
    )
    wf = ROOT.Belle2.MVA.Weightfile()
    general_options.m_method = specific_options.getMethod()
    wf.addOptions(general_options)
    wf.addOptions(specific_options)
    wf.addFile("ONNX_Modelfile", "model.onnx")
    print(f"save to {identifier}")
    ROOT.Belle2.MVA.Weightfile.save(wf, identifier)


class TestWriteONNX(unittest.TestCase):
    """
    Tests for writing ONNX MVA weightfiles. In addition to testing the writing
    mechanism, these serve the purpose of creating test files for other unit tests.
    """

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
        general_options = basf2_mva.GeneralOptions()
        general_options.m_datafiles = basf2_mva.vector("dummy")
        general_options.m_identifier = "Simple"
        general_options.m_treename = "tree"
        general_options.m_variables = basf2_mva.vector(*variables)
        if n_outputs > 1:
            general_options.m_nClasses = n_outputs
        specific_options = basf2_mva.ONNXOptions()
        with b2test_utils.clean_working_directory():
            save_onnx(model, general_options, specific_options, filename)
            with open(filename) as f:
                xml_new = f.read()
        ref_path = Path(basf2.find_file("mva/methods/tests")) / filename
        try:
            with open(ref_path) as f:
                xml_ref = f.read()
        except FileNotFoundError:
            # if the file does not exist, recreate it, but still fail the test
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

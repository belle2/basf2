#!/usr/bin/env python3

import unittest
import basf2
import basf2_mva_util

import numpy as np
import pandas as pd
import uproot


def write_dummy_file(variables):
    data = np.random.normal(size=[10, len(variables) + 1])
    tree = {}
    for i, name in enumerate(variables):
        tree[name] = data[:, i]
    tree["mcPDG"] = data[:, -1] > 0.5
    with uproot.recreate('dummy.root') as outfile:
        outfile['tree'] = pd.DataFrame(tree, dtype=np.float64)


class TestPyTorch(unittest.TestCase):
    def test_load_and_apply_existing_torch(self):
        method = basf2_mva_util.Method(basf2.find_file("mva/methods/tests/KLMMuonIDDNNWeightFile.xml"))
        write_dummy_file(list(method.general_options.m_variables))
        out1, out2 = method.apply_expert(
            method.general_options.m_datafiles, method.general_options.m_treename
        )
        self.assertEqual(out1.shape, (10,))
        self.assertEqual(out2.shape, (10,))


if __name__ == '__main__':
    import b2test_utils
    with b2test_utils.clean_working_directory():
        unittest.main()

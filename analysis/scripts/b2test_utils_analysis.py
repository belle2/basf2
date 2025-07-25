#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import sys
import subprocess
from contextlib import nullcontext
import unittest
import glob
from basf2 import find_file
from b2test_utils import configure_logging_for_tests, clean_working_directory


class ExamplesTest(unittest.TestCase):
    """Test to run all example scripts."""

    @unittest.skipIf(not os.getenv('BELLE2_EXAMPLES_DATA_DIR'),
                     "$BELLE2_EXAMPLES_DATA_DIR not found.")
    @unittest.skipIf(not os.getenv('BELLE2_VALIDATION_DATA_DIR'),
                     "$BELLE2_VALIDATION_DATA_DIR not found.")
    def _test_examples_dir(self, path_to_glob, broken=None, filepattern="", nevents=10, cleanup=False):
        """
        Internal function to test a directory full of example scripts with an optional list of broken scripts to be skipped.

        Parameters:
            path_to_glob (str): the path to search for scripts
            broken (list(str)): (optional) scripts that are known to be broken and can be skipped
        """
        if broken is None:
            broken = []
        configure_logging_for_tests()
        all_egs = sorted(glob.glob(find_file(path_to_glob) + f"/{filepattern}*.py"))
        for eg in all_egs:
            with (clean_working_directory() if cleanup else nullcontext()):
                filename = os.path.basename(eg)
                if filename not in broken:
                    with self.subTest(msg=filename):
                        outputfilename = filename.replace('.py', '.root')
                        result = subprocess.run(['basf2', '-n', f'{nevents}', eg, '-o',
                                                outputfilename], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                        if result.returncode != 0:
                            # failure running example so let's print the output
                            # on stderr so it's not split from output of unittest
                            # done like this since we don't want to decode/encode utf8
                            sys.stdout.buffer.write(result.stdout)
                        self.assertEqual(result.returncode, 0)

                        if os.path.exists(outputfilename) and isNtuple(outputfilename):
                            scanTTree(outputfilename)


def scanTTree(filename):
    from ROOT import TFile

    tfile = TFile(filename, "READ")
    print(f"TFile: {filename}")

    # get lists of TTree in the filename
    ttrees = [key.GetName() for key in tfile.GetListOfKeys() if key.GetClassName() == "TTree"]

    for ttree_name in ttrees:
        if ttree_name == "persistent":
            continue
        print(f"TTree: {ttree_name}")

        # get TTree object
        ttree = tfile.Get(ttree_name)
        num_entries = ttree.GetEntries()

        if num_entries == 0:
            print("No entry found")
            # print name all TBranches in the TTree
            for branch in ttree.GetListOfBranches():
                branch_name = branch.GetName()
                print(f"TBranch: {branch_name}")

        else:
            ttree.GetEntry(0)
            # print name and value of all TBranches in the TTree
            for branch in ttree.GetListOfBranches():
                branch_name = branch.GetName()
                branch_value = getattr(ttree, branch_name)

                if isinstance(branch_value, float):
                    print(f"TBranch: {branch_name}, {branch_value:.4g}")
                else:
                    print(f"TBranch: {branch_name}, {branch_value}")

    tfile.Close()


def isNtuple(filename):
    from ROOT import Belle2, TFile

    tfile = TFile(filename, "READ")
    ttree = tfile.Get("persistent")
    if not ttree:
        return False
    ttree.GetEntry(0)

    metadata = Belle2.FileMetaData(ttree.FileMetaData)

    return metadata.getDataDescription()["isNtupleMetaData"]

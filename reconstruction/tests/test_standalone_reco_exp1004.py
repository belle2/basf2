##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import pathlib
import subprocess

import basf2 as b2
import b2test_utils as b2tu


if __name__ == "__main__":

    prepare_path = os.getenv("BELLE2_PREPARE_PATH", "")
    if prepare_path == "":
        parent_path = pathlib.Path(b2.find_file('framework')).parent.absolute()
        prepare_path = os.path.join(parent_path, "prepare_tests")
        # Let's throw an error if prepare_path is not found
        b2.find_file(prepare_path)

    # Reconstruct few events for exp. 1004
    steering = b2.find_file(os.path.join('reconstruction', 'tests', 'reco.py_noexec'))
    input_file = b2.find_file(os.path.join(prepare_path, 'test_standalone_reco_bbbar_exp1004.root'))
    output_file = 'output_exp1004.root'

    with b2tu.clean_working_directory():
        subprocess.check_call(
            ['basf2', steering, '-i', input_file, '-o', output_file]
        )
        subprocess.check_call(
            ['b2file-check', '-n', '3', output_file]
        )

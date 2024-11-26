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


if __name__ == "__main__":

    prepare_path = os.getenv("BELLE2_PREPARE_PATH", "")
    if prepare_path == "":
        parent_path = pathlib.Path(b2.find_file('framework')).parent.absolute()
        prepare_path = os.path.join(parent_path, "prepare_tests")
        try:
            os.mkdir(prepare_path)
        except FileExistsError:
            pass

    # Generate and simulate few events without MC information for exp. 1003
    steering = b2.find_file(os.path.join('reconstruction', 'tests', 'prepare', 'evtgen_no_mc.py_noexec'))
    output_file = os.path.join(prepare_path, 'test_standalone_reco_no_mc_bbbar_exp1003.root')
    subprocess.check_call(
        ['basf2', steering, '--experiment', '1003', '--run', '0', '-n', '3', '-o', output_file]
    )

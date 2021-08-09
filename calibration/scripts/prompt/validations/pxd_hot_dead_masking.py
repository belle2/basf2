# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Validation of PXD hot/dead pixel calibration.
'''


import basf2
from prompt import ValidationSettings
import ROOT
import sys
import subprocess
import math
from glob import glob


#: Tells the automated system some details of this script
settings = ValidationSettings(name='PXD hot/dead pixel calibration',
                              description=__doc__,
                              download_files=['PXDRawHotPixelMaskCollectordebug.root'],
                              expert_config={})


def run_validation(job_path, input_data_path, requested_iov, expert_config):
    '''
    Run the validation.
    '''
    from conditions_db.testing_payloads import parse_testing_payloads_file
    txt_files = glob(f"{job_path}/*PXDHot*/outputdb/database.txt")
    cmds = ["b2pxd-db-check", "--only-hist", "--maps",
            "PXDMaskedPixelPar", "PXDDeadPixelPar", "PXDOccupancyInfoPar"]
    for txt_file in txt_files:
        iov_entries = parse_testing_payloads_file(txt_file)
        exp = iov_entries[0].firstRun["exp"]
        run = iov_entries[0].firstRun["run"]
        cmds_add = ["--tag", txt_file,
                    "--exp", f"{exp}", "--runs", f"{run}-9999",
                    "-o", f"conditions_e{exp}_r{run}.root"]
        subprocess.run(cmds + cmds_add, check=True)


if __name__ == "__main__":
    run_validation(*sys.argv[1:])

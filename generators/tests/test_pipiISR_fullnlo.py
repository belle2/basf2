##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Check stdout of generators/tests/test_pipiISR_fullnlo.py_noexec
'''

import basf2
import b2test_utils
import subprocess


if __name__ == '__main__':
    b2test_utils.configure_logging_for_tests()

    steering_file = basf2.find_file('generators/tests/test_pipiISR_fullnlo.py_noexec')

    with b2test_utils.clean_working_directory():
        process = subprocess.run(
                ['basf2', steering_file],
                capture_output=True,
                text=True,
                check=True)
        if "Rerun with positive mu2" in process.stdout:
            basf2.B2FATAL('Phokhara fails to generate pipiISR with FullNLO.')
        print(process.stdout)

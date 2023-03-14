##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Simple launcher for generators/tests/check_rho_mass_shift.py_noexec.
'''

import subprocess

import basf2
import b2test_utils

if __name__ == '__main__':

    b2test_utils.configure_logging_for_tests()

    steering_file = basf2.find_file('generators/tests/check_rho_mass_shift.py_noexec')

    for i in [1, 8, 9, 11, 13]:
        with b2test_utils.clean_working_directory():

            # This may look strange, but configure_logging_for_tests() does not act on the path of the steering file,
            # that is the first line printed by basf2.
            # So, we capture the log of the subprocess and we print it: in this way, the configure_loggging_etc
            # in this process will fix the log of the subprocess, otherwise this test fails on our CI systems.
            process = subprocess.run(['basf2', steering_file, str(i)],
                                     capture_output=True,
                                     text=True,
                                     check=True)
            print(process.stdout)

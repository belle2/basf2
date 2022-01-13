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

    steering_file = basf2.find_file('generators/tests/check_rho_mass_shift.py_noexec')

    for i in [1, 8, 9, 11, 13]:
        with b2test_utils.clean_working_directory():
            subprocess.check_call(['basf2', steering_file, str(i)])

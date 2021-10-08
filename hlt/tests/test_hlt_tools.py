##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test for checking if the main tools of the hlt package works.
'''

import glob
import subprocess

import basf2 as b2
import b2test_utils as b2tu

if __name__ == '__main__':
    with b2tu.clean_working_directory():

        # Check if b2hlt_print_result.py works.
        mdst_files = glob.glob(f'{b2.find_file("mdst/tests")}/mdst-v*.root')
        mdst_files.sort(reverse=True)
        subprocess.check_call(['b2hlt_print_result.py', mdst_files[0]])

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

        # Check if b2hlt_triggers works.
        # 1: b2hlt_triggers print
        subprocess.check_call(['b2hlt_triggers', 'print', '--database', 'online'])
        # 2: b2hlt_triggers download
        subprocess.check_call(['b2hlt_triggers', 'download', '--database', 'online'])
        # 3: b2hlt_triggers add_cut
        subprocess.check_call(['b2hlt_triggers', 'add_cut', 'filter', 'accept_goats', '[nTrkLoose > 0]', '10', 'False'])
        # 4: b2hlt_triggers remove_cut
        subprocess.check_call(['b2hlt_triggers', 'remove_cut', 'skim', 'accept_bhabha'])

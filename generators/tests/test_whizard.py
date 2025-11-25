##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test if WHIZARD works.
'''

import subprocess

import basf2 as b2
import b2test_utils as b2tu


if __name__ == '__main__':

    if not b2tu.is_ci():
        b2tu.skip_test("The test can not be properly run on buildbot.")

    with b2tu.clean_working_directory() as working_dir:

        steering_file = b2.find_file('generators/examples/Whizard.py')
        subprocess.check_call(['basf2', steering_file, '--experiment', '0', '--run', '0', '-n', '100'])

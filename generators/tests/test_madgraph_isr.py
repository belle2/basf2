##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test if MadGraph works when running a simple BSM model.
'''


import os
import subprocess

import basf2 as b2
import b2test_utils as b2tu


if __name__ == '__main__':

    # FIXME: this test fails on buildbot with the error:
    # PermissionError : [Errno 13] Permission denied: '{working_dir}/Dark_photon_mass_1_isr/bin/internal/ufomodel/__pycache__'
    # os.chmod below is supposed to fix this, but it doesn't work.
    if not b2tu.is_ci():
        b2tu.skip_test("The test can not be properly run on buildbot.")

    with b2tu.clean_working_directory() as working_dir:

        os.chmod(working_dir, 0o744)

        steering_file = b2.find_file('generators/madgraph/examples/MadGraph_darkphoton_isr.py')
        subprocess.check_call(['basf2', steering_file, '--random-seed', 'madgraph'])

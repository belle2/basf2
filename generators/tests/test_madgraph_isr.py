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


import subprocess

import basf2 as b2
import b2test_utils as b2tu


if __name__ == '__main__':

    with b2tu.clean_working_directory():

        steering_file = b2.find_file('generators/madgraph/examples/MadGraph_darkphoton_isr.py')
        subprocess.check_call(['basf2', steering_file, '--random-seed', 'madgraph'])

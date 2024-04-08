##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test if the command-line tool "b2generators-create-beam-payloads" works.
'''


import subprocess

import b2test_utils as b2tu


if __name__ == '__main__':

    with b2tu.clean_working_directory():

        subprocess.check_call(['b2generators-create-beam-payloads'])

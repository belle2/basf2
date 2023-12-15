##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test for checking b2conditionsdb iov and b2conditionsdb diff commands and option --run-range:
'''


import subprocess
import shlex


if __name__ == '__main__':

    tags = ['main_tag_merge_test_1', 'main_tag_merge_test_2', 'main_tag_merge_test_3']

    subprocess.check_call(shlex.split('b2conditionsdb iov main_tag_merge_test_2'))
    subprocess.check_call(shlex.split('b2conditionsdb iov main_tag_merge_test_2 --run-range 5 200 5 300'))
    subprocess.check_call(shlex.split('b2conditionsdb diff main_tag_merge_test_2 main_tag_merge_test_3'))
    subprocess.check_call(shlex.split('b2conditionsdb diff main_tag_merge_test_2 main_tag_merge_test_3 --run-range 5 200 5 300'))

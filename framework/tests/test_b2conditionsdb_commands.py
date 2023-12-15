##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test for checking:
    - b2conditionsdb iov
    - b2conditionsdb iov --run-range
    - b2conditionsdb diff
    - b2conditionsdb diff --run-range
    - b2conditionsdb legacydownload --run-range
'''


import subprocess
import shlex
import tempfile


if __name__ == '__main__':

    tags = ['main_tag_merge_test_1', 'main_tag_merge_test_2', 'main_tag_merge_test_3']

    subprocess.check_call(shlex.split('b2conditionsdb iov main_tag_merge_test_2'))
    subprocess.check_call(shlex.split('b2conditionsdb iov main_tag_merge_test_2 --run-range 5 200 5 300'))
    subprocess.check_call(shlex.split('b2conditionsdb diff main_tag_merge_test_2 main_tag_merge_test_3'))
    subprocess.check_call(shlex.split('b2conditionsdb diff main_tag_merge_test_2 main_tag_merge_test_3 --run-range 5 200 5 300'))

    # As the output of b2conditionsdb legacydownload contains some
    # irreproducible elements (times, tempfolder location) I check only the
    # number of lines of its output that should be = number of downloaded
    # payloads + 1
    with tempfile.TemporaryDirectory() as tmpdirname:
        output = subprocess.check_output(
            shlex.split(f'b2conditionsdb legacydownload -c main_tag_merge_test_3 {tmpdirname} --run-range 5 0 5 1000'),
            encoding='utf-8').strip().split('\n')
        print(f"Downloaded {len(output)-1} payloads")

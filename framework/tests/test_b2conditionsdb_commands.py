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
    - b2conditionsdb iovs delete --run-range
    - b2conditionsdb iovs delete --run-range --fully-contained
    - b2conditionsdb iovs copy --replace
'''


import subprocess
import shlex
import tempfile

import b2test_utils as b2tu


def call_command(command):
    """Call command print output after having removed some lines from output

    Args:
        command (string): command to be called
    """
    output = subprocess.check_output(shlex.split(command), encoding='utf-8').strip().split('\n')
    for line in output:
        if "created" not in line and "modified" not in line:
            print(line.strip())


if __name__ == '__main__':

    b2tu.skip_test('Test currently disabled due to CDB troubles')

    tags = ['main_tag_merge_test_1', 'main_tag_merge_test_2', 'main_tag_merge_test_3']

    call_command('b2conditionsdb iov main_tag_merge_test_2')
    call_command('b2conditionsdb iov main_tag_merge_test_2 --run-range 5 200 5 300')
    call_command('b2conditionsdb diff main_tag_merge_test_2 main_tag_merge_test_3')
    call_command('b2conditionsdb diff main_tag_merge_test_2 main_tag_merge_test_3 --run-range 5 200 5 300')

    # As the output of b2conditionsdb legacydownload contains some
    # irreproducible elements (times, tempfolder location) I remove those
    with tempfile.TemporaryDirectory() as tmpdirname:
        output = subprocess.check_output(
            shlex.split(f'b2conditionsdb legacydownload -c main_tag_merge_test_3 {tmpdirname} --run-range 5 0 5 1000'),
            encoding='utf-8').strip().split('\n')
        for line in output:
            print(line.replace(tmpdirname, "centraldb")[7:].strip())

    call_command('b2conditionsdb iovs delete --dry-run main_tag_merge_test_2 --run-range 5 200 5 300')
    call_command('b2conditionsdb iovs delete --dry-run main_tag_merge_test_2 --run-range 5 200 5 300 --fully-contained')
    call_command('b2conditionsdb iovs copy --replace --dry-run main_tag_merge_test_2 main_tag_merge_test_1')

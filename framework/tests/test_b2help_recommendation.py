##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test for checking:
    - b2help-recommendation
    - b2help-recommendation -t gamma
'''

import basf2
import b2test_utils
import subprocess
import shlex


def call_command(command):
    """Call command print output after having removed some lines from output

    Args:
        command (string): command to be called
    """
    output = subprocess.check_output(shlex.split(command), encoding='utf-8').strip().split('\n')
    for line in output:
        if "provider = " not in line:
            print(line)


if __name__ == '__main__':
    b2test_utils.configure_logging_for_tests()
    check_tags = ['all', 'gamma', 'Tracking']
    filename = basf2.find_file('framework/tests/test_b2help_recommendation_payload.json')
    # List available tags
    call_command(f'b2help-recommendation -l {filename} -f rst')
    # Call the script for tags
    for tag in check_tags:
        call_command(f'b2help-recommendation -t {tag} -l {filename} -f rst')
    # Call convert to payload
    call_command(f'b2help-recommendation -l {filename} -f rst -t {check_tags[1]} -c -p test')

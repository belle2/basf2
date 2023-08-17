##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
We generate, simulate and reconstruct few events with two different processes
and we check the two processes give exactly the same output.
'''


import difflib
import subprocess

import basf2 as b2
import b2test_utils as b2tu


def _unified_diff(expected, actual):
    '''
    Helper function. Returns a string containing the unified diff of two multiline strings.
    '''
    expected = expected.splitlines(1)
    actual = actual.splitlines(1)
    diff = difflib.unified_diff(expected, actual)
    return ''.join(diff)


if __name__ == '__main__':

    steering = b2.find_file('reconstruction/tests/test_randomness.py_noexec')
    command = f'basf2 {steering} -n 5 --random-seed aSeed'

    b2.B2INFO('Executing first job...')
    with b2tu.clean_working_directory():
        job1 = subprocess.run(command.split(), check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)

    b2.B2INFO('Executing second job...')
    with b2tu.clean_working_directory():
        job2 = subprocess.run(command.split(), check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)

    diff = _unified_diff(job1.stdout, job2.stdout)
    if diff:
        b2.B2ERROR('!!! The two outputs are different !!!')
        print(diff)
        b2.B2FATAL('!!! basf2 produces irreproducible results !!!')

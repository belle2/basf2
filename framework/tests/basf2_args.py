#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import b2test_utils
import subprocess
import os

# disable pager when running these interactively
os.environ['PAGER'] = "cat"

success = [
    ['--info'],
    ['--version'],
    ['--help'],
    ['-m', 'RootOutput'],
    ['--dry-run', 'root_input.py', '-i', 'miep.root'],
    ['--dry-run', 'root_input.py'],
]
error = [
    ['-m', 'NonExistingModule'],
    ['--thisdoesntexist'],
    ['/this/path/doesnt/exist.py'],
    ['h͌̉e̳̞̞͆ͨ̏͋̕c͟o͛҉̟̰̫͔̟̪̠m̴̀ͯ̿͌ͨ̃͆e̡̦̦͖̳͉̗ͨͬ̑͌̃ͅt̰̝͈͚͍̳͇͌h̭̜̙̦̣̓̌̃̓̀̉͜'],
]

test_dir = basf2.find_file("/framework/tests")
with b2test_utils.working_directory(test_dir):
    for arguments in success:
        assert 0 == subprocess.run(['basf2'] + arguments).returncode

    for arguments in error:
        assert 0 != subprocess.run(['basf2'] + arguments).returncode

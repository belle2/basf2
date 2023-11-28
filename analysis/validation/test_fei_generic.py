#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>wsut@uni-bonn.de</contact>
</header>
"""

import os
import subprocess
import shutil
import basf2
import sys

if __name__ == '__main__':
    cwd = os.getcwd()
    tempdir = 'tmp_fei_generic'
    os.mkdir(tempdir)
    os.chdir(tempdir)

    steering_file = basf2.find_file('analysis/validation/test_fei_generic.py_noexec')
    process = subprocess.run(['basf2', steering_file])

    os.chdir(cwd)
    shutil.rmtree(tempdir)

    sys.exit(process.returncode)

#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This file contains multiple utility functions which can be used by
validation steering files. """

import subprocess
import pickle
import sys
import os


def update_env():
    """Update the environment with b2setup

    This is very annoying as there is no way for sub processes to modify the
    parent environment. And also b2setup ist not inherited by sub shells. So we
    have to run bash, source the tools, run b2setup and get all the environment
    variables from the output.
    """
    # the no_tools_check variable speeds up the process as it does not check
    # git to see if the tools are up to date.
    cmd = (
        "BELLE2_NO_TOOLS_CHECK=1 source {}/b2setup > /dev/null && "
        '{} -c "import sys,os,pickle; '
        'sys.stdout.buffer.write(pickle.dumps(dict(os.environ)))"'.format(
            os.environ["BELLE2_TOOLS"], sys.executable
        )
    )
    output = subprocess.check_output(["bash", "-c", cmd])
    newenv = pickle.loads(output)
    os.environ.update(newenv)

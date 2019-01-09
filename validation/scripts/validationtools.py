#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This file contains multiple utility functions which can be used by validation steering files.
"""

import subprocess
import pickle
import sys
import os

from basf2 import B2FATAL


def get_background_files():
    """ Deprecated. Use background.get_background_files()
    """
    B2FATAL("This methd is deprecated, use background.get_background_files() from now on.")


def update_env():
    """Update the environment with b2setup

    This is very annoying as there is no way for sub processes to modify the
    parent environment. And also b2setup ist not inherited by sub shells. So we
    have to run bash, source the tools, run b2setup and get all the environment
    variables from the output.
    """
    # the no_tools_check variable speeds up the process as it does not check git
    # to see if the tools are up to date.
    cmd = 'BELLE2_NO_TOOLS_CHECK=1 source %s/b2setup > /dev/null && ' \
        '%s -c "import sys,os,pickle; sys.stdout.buffer.write(pickle.dumps(dict(os.environ)))"' % (
            os.environ['BELLE2_TOOLS'], sys.executable
        )
    output = subprocess.check_output(['bash', '-c', cmd])
    newenv = pickle.loads(output)
    os.environ.update(newenv)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import subprocess
import pickle
import sys
import os


def update_env():
    """Update the environment with setuprel

    This is very annoying as there is no way for sub processes to modify the
    parent environment. And also setupel ist not inherited by sub shells. So we
    have to run bash, source the tools, run setuprel and get all the environment
    variables from the output.
    """
    # the no_tools_check variable speeds up the process as it does not check svn
    # to see if the tools are up to date.
    cmd = 'BELLE2_NO_TOOLS_CHECK=1 source %s/setup_belle2 > /dev/null && setuprel > /dev/null && ' \
        '%s -c "import sys,os,pickle; sys.stdout.buffer.write(pickle.dumps(dict(os.environ)))"' % (
            os.environ['BELLE2_TOOLS'], sys.executable
        )
    output = subprocess.check_output(['bash', '-c', cmd])
    newenv = pickle.loads(output)
    os.environ.update(newenv)

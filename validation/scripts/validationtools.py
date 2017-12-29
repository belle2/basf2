#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This file contains multiple utility functions which can be used by validation steering files.
"""

import subprocess
import pickle
import sys
import os
import glob


def get_background_files():
    """ Ensures that background files exist and returns the list of background files which
     can be directly used with add_simulation() :

     bg = validationtools.get_background_files()
     add_simulation(main, bkgfiles=bg)

     Will fail with an assert if no background folder set or if no background file was
     found in the set folder.
    """

    env_name = 'BELLE2_BACKGROUND_DIR'
    bg = None
    bg_folder = ""
    if env_name in os.environ:
        bg_folder = os.environ[env_name]
        bg = glob.glob(bg_folder + '/*.root')

    if bg is None:
        print("Environment variable {} for backgound files not set. Terminanting this script.".format(env_name))
        assert False

    if len(bg) == 0:
        print("No background files found in folder {} . Terminating this script.".format(bg_folder))
        assert False

    print("Background files loaded from folder {}".format(bg_folder))

    # sort for easier comparison
    bg = sorted(bg)

    print("{: >65} {: >65} ".format("- Background file name -", "- file size -"))
    for f in bg:
        fsize = os.path.getsize(f)
        print("{: >65} {: >65} ".format(f, fsize))

    return bg


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

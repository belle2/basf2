#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import signal
from pybasf2 import *


def _avoidPyRootHang():
    """
    workaround for possible hang with PyROOT on SL5
    see https://belle2.cc.kek.jp/redmine/issues/1236
    note: platform.libc_ver() is _really_ broken, so i'm checking the
    version via ldd (ships with libc)
    """
    import subprocess
    ldd_ver = subprocess.check_output(['ldd', '--version'])
    sl5_libc_string = b"ldd (GNU libc) 2.5"
    if sl5_libc_string in ldd_ver:
        try:
            from ROOT import PyConfig
            PyConfig.StartGuiThread = False
        except:
            print("PyRoot not set up, this will cause problems.")


_avoidPyRootHang()


# -----------------------------------------------
#             Set basf2 information
# -----------------------------------------------
basf2label = 'BASF2 (Belle Analysis Software Framework 2)'
basf2version = os.environ.get('BELLE2_RELEASE', 'unknown')
basf2copyright = 'Copyright(C) 2010-2016  Belle II Collaboration'

# -----------------------------------------------
#               Prepare basf2
# -----------------------------------------------

# Reset the signal handler to allow the framework execution
# to be stopped with Ctrl-c (Python installs own handler)
signal.signal(signal.SIGINT, signal.SIG_DFL)


# -----------------------------------------------
#       Create default framework object
# -----------------------------------------------

# Create default framework, also initialises environment
fw = Framework()

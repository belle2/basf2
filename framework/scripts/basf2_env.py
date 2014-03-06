#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import signal
from pybasf2 import *

# -----------------------------------------------
#             Set basf2 information
# -----------------------------------------------
basf2label = 'BASF2 (Belle Analysis Framework 2)'
basf2version = os.environ.get('BELLE2_RELEASE', 'unknown')
basf2copyright = 'Copyright(C) 2010-2014  Belle II Collaboration'

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


# -----------------------------------------------
#             Print output
# -----------------------------------------------
print '>>> basf2 Python environment set'
print '>>> Framework object created: fw'

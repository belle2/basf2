#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# now let's make sure we actually run in python 3
import sys
if sys.version_info[0] < 3:
    print("basf2 requires python3. Please run the steering files using basf2 "
          "(or python3), not python")
    sys.exit(1)

# import to override print function
import basf2_print

# -----------------------------------------------
#          Import basf2 environment
# -----------------------------------------------
from basf2_env import *

# -----------------------------------------------
#    Import global basf2 function definitions
# -----------------------------------------------
from basf2_def import *

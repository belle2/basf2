#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Database Retriever.
Simple example for testing the SVD calibrations access.
"""

from basf2 import *
import ROOT

import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

use_central_database("development")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Event info setter - execute single event
svddatabaseTest = register_module('SVDDatabaseTest')

main.add_module(svddatabaseTest)
print("svd database accessed")

# process single event
process(main)


print("done: FINISH!!!")

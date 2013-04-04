#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *
from PXDHitErrors import PXDHitErrors

logging.log_level = LogLevel.WARNING

# Read input file
input = register_module('RootInput')
input.param('inputFileName', 'HitErrorGenOutput.root')
# Show progress of processing
progress = register_module('Progress')

# create processing path
main = create_path()
main.add_module(input)
main.add_module(progress)
main.add_module(PXDHitErrors())

# process events
process(main)

# show call statistics
print statistics

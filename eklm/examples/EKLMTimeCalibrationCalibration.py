#!/usr/bin/env python
# -*- coding: utf-8 -*-

# EKLM time calibration.

from basf2 import *
from reconstruction import *
import sys

# Set the global log level
set_log_level(LogLevel.INFO)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])

gearbox = register_module('Gearbox')

eklmtimecalibration = register_module('EKLMTimeCalibration')
eklmtimecalibration.param('PerformDataCollection', False)
eklmtimecalibration.param('DataOutputFile', sys.argv[1])

# Create the main path and add the modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eklmtimecalibration)

# generate events
process(main)

# show call statistics
print statistics

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Study of EKLM alignment limits.

import os
import random
from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)

# EventInfoSetter - generate event meta data
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])

# XML reader
xmldata = register_module('Gearbox')

# EKLM displacement generator
eklmalignment = register_module('EKLMDisplacementGenerator')
eklmalignment.param('Mode', 'Limits')
eklmalignment.param('OutputFile', 'EKLMAlignmentLimits.root')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(xmldata)
main.add_module(eklmalignment)

# Run
process(main)

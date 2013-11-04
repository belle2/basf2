#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Regeneration of EKLM alignment and transformation data.

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

# EKLM alignment
eklmalign = register_module('EKLMAlignment')
eklmalign.param('OutputFile', 'eklm_alignment.dat')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(xmldata)
main.add_module(eklmalign)

# Run
process(main)


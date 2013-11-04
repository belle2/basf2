#!/usr/bin/env python
# -*- coding: utf-8 -*-

# EKLM ADC study.

import os
import random
from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.WARNING)

# EventInfoSetter - generate event meta data
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])

# XML reader
xmldata = register_module('Gearbox')

# EKLM ADC
eklmadc = register_module('EKLMADC')
eklmadc.param('Mode', 'Shape')
eklmadc.param('OutputFile', 'FPGA.root')
# For generation of the histograms for strips.
# eklmadc.param('Mode', 'Strips')
# eklmadc.param('OutputFile', 'EKLMADC.root')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(xmldata)
main.add_module(eklmadc)

# Run
process(main)


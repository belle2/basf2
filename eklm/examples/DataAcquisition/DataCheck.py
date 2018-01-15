#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Check collected data.

from basf2 import *
import sys

set_log_level(LogLevel.INFO)

input = register_module('RootInput')
input.param('inputFileName', sys.argv[1])

gearbox = register_module('Gearbox')

eklmdatachecker = register_module('EKLMDataChecker')

# Create the main path and add the modules
main = create_path()
main.add_module(input)
main.add_module(gearbox)
main.add_module(eklmdatachecker)

# Process the data
process(main)

# show call statistics
print(statistics)

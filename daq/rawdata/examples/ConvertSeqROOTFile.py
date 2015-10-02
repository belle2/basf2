#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import *

set_log_level(LogLevel.INFO)

# input
input = register_module('SeqRootInput')

# dump
dump = register_module('SeqRootOutput')

# progress monitor
progress = register_module('Progress')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(dump)
main.add_module(progress)

# Process all events
process(main)

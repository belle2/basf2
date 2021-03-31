#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
#
# Example steering file - 2011 Belle II Collaboration
######################################################

import basf2 as b2

b2.set_log_level(b2.LogLevel.INFO)

# input
input = b2.register_module('SeqRootInput')

# dump
dump = b2.register_module('SeqRootOutput')

# progress monitor
progress = b2.register_module('Progress')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(input)
main.add_module(dump)
main.add_module(progress)

# Process all events
b2.process(main)

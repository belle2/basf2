#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys

# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
# set_debug_level(1000)

# Create main path
main = create_path()

# Modules
inroot = register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', sys.argv[1])
main.add_module(inroot)

v0a = register_module('DQMHistAnalysisV0')
main.add_module(v0a)

outroot = register_module('DQMHistAnalysisOutputFile')
outroot.param('SaveHistos', False)  # don't save histograms
outroot.param('SaveCanvases', True)  # save canvases
main.add_module(outroot)

print_path(main)
# Process all events
process(main)

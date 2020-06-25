#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

import sys
argv = sys.argv


# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
# set_debug_level(1000)

# Create main path
main = create_path()

# Modules

# input root file with DQM histograms
inroot = register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', sys.argv[1])
main.add_module(inroot)

# add DQM analysis module
mirabelle = register_module('DQMHistAnalysisMiraBelle')
main.add_module(mirabelle)

# output created MonitoringObject to the root file
outroot = register_module('DQMHistAnalysisOutputMonObj')
outroot.param('ProcID', 'online')  # set processing ID
main.add_module(outroot)


print_path(main)
# Process all events
process(main)

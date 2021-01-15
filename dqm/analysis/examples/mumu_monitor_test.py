#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2

import sys
argv = sys.argv


# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
# set_debug_level(1000)

# Create main path
main = b2.create_path()

# Modules

# input root file with DQM histograms
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', sys.argv[1])
main.add_module(inroot)

# add DQM analysis module
mirabelle = b2.register_module('DQMHistAnalysisMiraBelle')
main.add_module(mirabelle)

# output created MonitoringObject to the root file
outroot = b2.register_module('DQMHistAnalysisOutputMonObj')
outroot.param('ProcID', 'online')  # set processing ID
main.add_module(outroot)


b2.print_path(main)
# Process all events
b2.process(main)

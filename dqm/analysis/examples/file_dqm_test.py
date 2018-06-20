#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

# Create main path
main = create_path()

# Modules
input = register_module('DQMHistAnalysisInputFile')
input.param('SelectFolders', ['TOP'])  # leave blank to include all folders
input.param('InputRootFile', "dqm_e0003r003772.root")
main.add_module(input)

top = register_module('DQMHistAnalysisTOP')
main.add_module(top)

output = register_module('DQMHistAnalysisOutputFile')
output.param('SaveHistos', False)  # don't save histograms
output.param('SaveCanvases', True)  # save canvases
main.add_module(output)

# Process all events
process(main)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

# Create main path
main = create_path()

# Modules
input = register_module('DQMHistAnalysisInput')
input.param('HistMemoryPath', argv[1])
main.add_module(input)

output = register_module('DQMHistAnalysisOutputFile')
output.param('SaveCanvases', True)
output.param('SaveHistos', False)
main.add_module(output)

# Process all events
process(main)

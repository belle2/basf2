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

# example = register_module('DQMHistAnalysisExample')
# example.param("HistoName", "DAQ/h_Gaus_2")
# example.param("Function", "gaus(2)")
# main.add_module(example)

output = register_module('DQMHistAnalysisOutputRelayMsg')
main.add_module(output)

# nsm = register_module('DQMHistAnalysisOutputNSM')
# nsm.param('NSMNodeName', "DQMH1")
# main.add_module(nsm)

# Process all events
process(main)

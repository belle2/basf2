#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)

# Create main path
main = b2.create_path()

# Modules
input = b2.register_module('DQMHistAnalysisInput')
input.param('HistMemoryPath', argv[1])
main.add_module(input)

# example = register_module('DQMHistAnalysisExample')
# example.param("HistoName", "DAQ/h_Gaus_2")
# example.param("Function", "gaus(2)")
# main.add_module(example)

output = b2.register_module('DQMHistAnalysisOutputRelayMsg')
main.add_module(output)

# nsm = register_module('DQMHistAnalysisOutputNSM')
# nsm.param('NSMNodeName', "DQMH1")
# main.add_module(nsm)

# Process all events
b2.process(main)

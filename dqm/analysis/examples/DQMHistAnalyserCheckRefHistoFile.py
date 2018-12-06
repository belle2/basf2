#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)

# Create main path
main = create_path()

# Modules
input = register_module('DQMHistAnalysisInput')
input.param('HistMemoryPath', argv[1])
input.param('AutoCanvas', False)
main.add_module(input)

checker = register_module('DQMHistComparitor')
checker.param('HistoList', [
    ['FirstDet/h_HitXPositionCh01', 'ref/FirstDet/h_HitXPositionCh01', 'xdet/test1', '100', '0.9', '0.6', 'test1'],
    ['FirstDet/h_HitYPositionCh01', 'ref/FirstDet/h_HitYPositionCh01', 'ydet/test2', '100', '0.9', '0.6', 'test2'],
    ['SecondDet/h_HitCh01', 'ref/SecondDet/h_HitCh01', 'test3', '100', '0.9', '0.6', 'test3']
]
)
main.add_module(checker)


output = register_module('DQMHistAnalysisOutputRelayMsg')
# check that port fit your root canvas server
output.param('Port', 9192)
main.add_module(output)

# Process all events
process(main)

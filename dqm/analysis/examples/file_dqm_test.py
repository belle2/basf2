#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)

# Create main path
main = b2.create_path()

# Modules
input = b2.register_module('DQMHistAnalysisInputRootFile')
# use full histogram names (wildcard characters supported), leave blank to include all histograms
input.param('SelectHistograms', ['TOP/recoTime', 'TOP/recoPull', 'TOP/good_hits_xy_*'])
input.param('FileList', ["root_file1.root", "root_file2.root"])
input.param('RunList', [3000, 3001])
input.param('EventsList', [10, 5])
input.param('Experiment', 12)
input.param('EventInterval', 10)
input.param('NullHistogramMode', False)  # Set to True to use the null histogram test mode, in which no histogram will be loaded
main.add_module(input)

top = b2.register_module('DQMHistAnalysisTOP')
main.add_module(top)

# Output to root file
# output = register_module('DQMHistAnalysisOutputFile')
# output.param('SaveHistos', False)  # don't save histograms
# output.param('SaveCanvases', True)  # save canvases
# main.add_module(output)

# Output to canvas server
output = b2.register_module('DQMHistAnalysisOutputRelayMsg')
output.param("Port", 9192)  # This port must match the port used in canvasserver.C
main.add_module(output)

# Process all events
b2.process(main)

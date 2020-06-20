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
input = register_module('DQMHistAnalysisInputRootFile')
input.param('SelectFolders', ['TOP'])  # leave blank to include all folders
# use full histogram names, leave blank to include all histograms
input.param('SelectHistograms', ['TOP/recoTime', 'TOP/recoPull', 'TOP/good_hits_xy_*'])
input.param('FileList', ["root_file1.root", "root_file2.root"])
input.param('RunList', [3000, 3001])
input.param('EventsList', [10, 5])
input.param('Experiment', 12)
input.param('EventInterval', 10)
main.add_module(input)

top = register_module('DQMHistAnalysisTOP')
main.add_module(top)

# output = register_module('DQMHistAnalysisOutputFile')
# output.param('SaveHistos', False)  # don't save histograms
# output.param('SaveCanvases', True)  # save canvases
# main.add_module(output)

output = register_module('DQMHistAnalysisOutputRelayMsg')
output.param("Port", 9192)
main.add_module(output)

# Process all events
process(main)

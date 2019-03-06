#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
argv = sys.argv

if len(argv) < 3:
    print()
    print('Usage: %s input_filename output_filename' % argv[0])
    print()
    exit(1)

set_log_level(LogLevel.INFO)

# Create main path
main = create_path()

# Modules
inroot = register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', argv[1])
main.add_module(inroot)

ecl = register_module('DQMHistAnalysisECL')
main.add_module(ecl)

outroot = register_module('DQMHistAnalysisOutputFile')
outroot.param('SaveHistos', False)  # don't save histograms
outroot.param('SaveCanvases', True)  # save canvases
outroot.param('HistoFile', argv[2])
main.add_module(outroot)

print_path(main)
# Process all events
process(main)

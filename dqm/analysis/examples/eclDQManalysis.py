#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import sys
argv = sys.argv

if len(argv) < 3:
    print()
    print('Usage: %s input_filename output_filename' % argv[0])
    print()
    exit(1)

b2.set_log_level(b2.LogLevel.INFO)

# Create main path
main = b2.create_path()

# Modules
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', argv[1])
main.add_module(inroot)

ecl = b2.register_module('DQMHistAnalysisECL')
main.add_module(ecl)

outroot = b2.register_module('DQMHistAnalysisOutputFile')
outroot.param('SaveHistos', False)  # don't save histograms
outroot.param('SaveCanvases', True)  # save canvases
outroot.param('HistoFile', argv[2])
main.add_module(outroot)

b2.print_path(main)
# Process all events
b2.process(main)

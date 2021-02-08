#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
argv = sys.argv

if len(argv) < 3:
    print()
    print('v 1.0 / Feb. 7, 2021 / by your colleague')
    print()
    print('This script processes histograms from selected (or all) branches/folders and prepares canvases')
    print()
    print('To make this script work, first build dqm package b2code-package-add daq; scons')
    print()
    print('For more information see https://confluence.desy.de/display/BI/Develop+DQM+Analysis+Modules')
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

# you can select which branches/folders to process, leave blank to process all branches/folders
# inroot.param('SelectFolders', ['TRGTOP'])

# you can select which histograms to process, leave blank to include all
# inroot.param('SelectHistograms', ['TRGTOP/h_*'])

main.add_module(inroot)

outroot = register_module('DQMHistAnalysisOutputFile')

outroot.param('SaveHistos', False)  # don't save histograms
outroot.param('SaveCanvases', True)  # save canvases

outroot.param('HistoFile', argv[2])
main.add_module(outroot)

print_path(main)

# Process all events
process(main)

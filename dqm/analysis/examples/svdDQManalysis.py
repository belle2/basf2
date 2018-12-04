#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)

# Create main path
main = create_path()

# Modules
inroot = register_module('DQMHistAnalysisInputRootFile')
# inroot.param('SelectFolders', ['SVDExpReco','SVVDUnpacker'])  # SVD folders
inroot.param('InputRootFile', "SVDDQM.root")
main.add_module(inroot)

main.add_module('Gearbox')
main.add_module('Geometry', components=['SVD'])
main.add_module('DQMHistAnalysisSVDGeneral', printCanvas=True)

outroot = register_module('DQMHistAnalysisOutputFile')
outroot.param('SaveHistos', False)  # don't save histograms
outroot.param('SaveCanvases', True)  # save canvases
main.add_module(outroot)

# Process all events
process(main)

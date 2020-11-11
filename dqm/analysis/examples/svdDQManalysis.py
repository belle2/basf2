#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
# set_debug_level(1000)

# Create main path
main = b2.create_path()

# Modules
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
# inroot.param('SelectFolders', ['SVDExpReco','SVVDUnpacker'])  # SVD folders
inroot.param('InputRootFile', "SVDDQM.root")
main.add_module(inroot)

main.add_module('Gearbox')
main.add_module('Geometry')

svd = b2.register_module('DQMHistAnalysisSVDGeneral')
svd.param('printCanvas', True)
svd.param('unpackerErrorLevel', 0.00001)
svd.param('RefHistoFile', "SVDrefHisto.root")
main.add_module(svd)

outroot = b2.register_module('DQMHistAnalysisOutputFile')
outroot.param('SaveHistos', False)  # don't save histograms
outroot.param('SaveCanvases', True)  # save canvases
main.add_module(outroot)

b2.print_path(main)
# Process all events
b2.process(main)

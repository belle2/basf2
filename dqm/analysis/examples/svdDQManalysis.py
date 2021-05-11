#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import conditions as b2conditions
import basf2 as b2
import sys
import re
argv = sys.argv

# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
# set_debug_level(1000)

inputFile = sys.argv[1]
exp_nr = int(re.findall(r'\d+', inputFile)[0])
run_nr = int(re.findall(r'\d+', inputFile)[1])
nevt = int(sys.argv[2])  # number of events

# setup database
b2conditions.reset()
b2conditions.override_globaltags()
b2conditions.globaltags = ["online"]


# Create main path
main = b2.create_path()

# Modules
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('FileList', inputFile)
inroot.param('SelectHistograms', ['SVD*/*'])
inroot.param('Experiment', exp_nr)
inroot.param('RunList', [run_nr])
inroot.param('EventsList', [nevt])
main.add_module(inroot)

main.add_module('Gearbox')
main.add_module('Geometry')

svd = b2.register_module('DQMHistAnalysisSVDGeneral')
svd.param('printCanvas', False)
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

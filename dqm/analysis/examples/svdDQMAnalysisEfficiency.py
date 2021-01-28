# !/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import sys

mypath = b2.Path()
inputFile = sys.argv[1]
outputFile = sys.argv[2]

b2.conditions.prepend_globaltag('data_reprocessing_prompt')

inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', inputFile)
mypath.add_module(inroot)

mypath.add_module('Gearbox')
mypath.add_module('Geometry')

dqm = b2.register_module('DQMHistAnalysisSVDEfficiency')
dqm.set_log_level(b2.LogLevel.INFO)
dqm.param("printCanvas", True)
mypath.add_module(dqm)

outroot = b2.register_module('DQMHistAnalysisOutputFile')
outroot.param('SaveHistos', False)
outroot.param('SaveCanvases', True)
outroot.param('HistoFile', outputFile)
mypath.add_module(outroot)

# Process the events
b2.print_path(mypath)
b2.process(mypath)

# print out the summary
print(b2.statistics)

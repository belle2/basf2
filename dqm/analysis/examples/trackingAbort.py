# !/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import sys

mypath = b2.Path()
inputFile = sys.argv[1]
outputFile = sys.argv[2]

'''
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('FileList', [inputFile])
inroot.param('EventInterval', 0)
inroot.param('EventsList',0)
inroot.param('Experiment', 33)
inroot.param('RunType', 'physics')
mypath.add_module(inroot)
'''

inroot = b2.register_module('DQMHistAnalysisInput2')
inroot.param('HistMemoryPath', inputFile)
mypath.add_module(inroot)

dqm = b2.register_module('DQMHistAnalysisTrackingAbort')
dqm.set_log_level(b2.LogLevel.INFO)
dqm.param("printCanvas", True)
mypath.add_module(dqm)

mypath.add_module('DQMHistAnalysisTrackingHLT')

outroot = b2.register_module('DQMHistAnalysisOutputFile')
outroot.param('SaveHistos', True)
outroot.param('SaveCanvases', True)
# outroot.param('HistoFile', outputFile)
outroot.param('Filename', outputFile)
outroot.param('OutputFolder', '.')
mypath.add_module(outroot)

# Process the events
b2.print_path(mypath)
b2.process(mypath)

# print out the summary
print(b2.statistics)

# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys

mypath = Path()
inputFile = sys.argv[1]
outputFile = sys.argv[2]

reset_database()
use_database_chain()
use_central_database('data_reprocessing_prompt_bucket4b')
# mypath.add_module('SetupGenfitExtrapolation')

inroot = register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', inputFile)
mypath.add_module(inroot)

mypath.add_module('Gearbox')
mypath.add_module('Geometry')

dqm = register_module('DQMHistAnalysisSVDEfficiency')
dqm.set_log_level(LogLevel.INFO)
dqm.param("RefHistoFile", "")
dqm.param("effLevel_Error", 0.5)
dqm.param("effLevel_Warning", 0.7)
dqm.param("effLevel_Empty", 0)
dqm.param("printCanvas", True)
mypath.add_module(dqm)

outroot = register_module('DQMHistAnalysisOutputFile')
outroot.param('SaveHistos', False)
outroot.param('SaveCanvases', True)
outroot.param('HistoFile', outputFile)
mypath.add_module(outroot)

# Process the events
print_path(mypath)
process(mypath)

# print out the summary
print(statistics)

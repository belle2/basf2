# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
from basf2 import conditions as b2conditions

mypath = Path()
inputFile = sys.argv[1]
outputFile = sys.argv[2]


# setup database
b2conditions.reset()
b2conditions.override_globaltags()
b2conditions.globaltags = [
    'klm_alignment_testing',
    'data_reprocessing_proc11',
    "data_reprocessing_prompt",
    "online_proc11",
    "Reco_master_patch_rel5"]

inroot = register_module('DQMHistAnalysisInputRootFile')
inroot.param('FileList', inputFile)
inroot.param('EventsList', [1])
inroot.param('SelectHistograms', ['SVD*/*'])
mypath.add_module(inroot)

mypath.add_module('Gearbox')
mypath.add_module('Geometry')

dqmEff = register_module('DQMHistAnalysisSVDEfficiency')
dqmEff.set_log_level(LogLevel.INFO)
dqmEff.param("printCanvas", True)
mypath.add_module(dqmEff)

dqmGen = register_module('DQMHistAnalysisSVDGeneral')
dqmGen.set_log_level(LogLevel.INFO)
mypath.add_module(dqmGen)

outroot = register_module('DQMHistAnalysisOutputMonObj')
outroot.param('ProcID', 'online')  # set processing ID
outroot.param('TreeFile', outputFile)
mypath.add_module(outroot)

# Process the events
print_path(mypath)
process(mypath)

# print out the summary
print(statistics)

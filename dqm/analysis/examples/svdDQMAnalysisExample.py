# !/usr/bin/env python3
# -*- coding: utf-8 -*-

# illustrative run:
# basf2 svdDQMAnalysisExample.py dqm_e0014r000921.root

from basf2 import *
import sys
import re

mypath = Path()
inputFile = sys.argv[1]
exp_nr = int(re.findall(r'\d+', inputFile)[0])
run_nr = int(re.findall(r'\d+', inputFile)[1])

inroot = register_module('DQMHistAnalysisInputRootFile')
inroot.param('FileList', inputFile)
inroot.param('SelectHistograms', ['SVD*/*'])
inroot.param('Experiment', exp_nr)
inroot.param('RunList', [run_nr])
mypath.add_module(inroot)

dqmSVD = register_module('DQMHistAnalysisSVDOnMiraBelle')
dqmSVD.set_log_level(LogLevel.INFO)
mypath.add_module(dqmSVD)

outroot = register_module('DQMHistAnalysisOutputMonObj')
outroot.param('ProcID', 'online')  # set processing ID
outroot.param('exp', exp_nr)
outroot.param('run', run_nr)
mypath.add_module(outroot)

# Process the events
print_path(mypath)
process(mypath)

# print out the summary
print(statistics)

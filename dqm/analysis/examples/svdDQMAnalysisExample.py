# !/usr/bin/env python3
# -*- coding: utf-8 -*-

# illustrative run:
# basf2 svdDQMAnalysisExample.py dqm_e0014r000921.root 1

import basf2 as b2
import sys
import re

mypath = b2.Path()
inputFile = sys.argv[1]
exp_nr = int(re.findall(r'\d+', inputFile)[0])
run_nr = int(re.findall(r'\d+', inputFile)[1])
nevt = int(sys.argv[2])  # number of events

# setup database
b2.conditions.reset()
b2.conditions.override_globaltags()
b2.conditions.globaltags = ["online"]

inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('FileList', inputFile)
inroot.param('SelectHistograms', ['SVD*/*'])
inroot.param('Experiment', exp_nr)
inroot.param('RunList', [run_nr])
inroot.param('EventsList', [nevt])
mypath.add_module(inroot)

dqmSVD = b2.register_module('DQMHistAnalysisSVDOnMiraBelle')
mypath.add_module(dqmSVD)

outroot = b2.register_module('DQMHistAnalysisOutputMonObj')
outroot.param('ProcID', 'online')  # set processing ID
outroot.param('exp', exp_nr)
outroot.param('run', run_nr)
outroot.param('nevt', nevt)
mypath.add_module(outroot)

# Process the events
b2.print_path(mypath)
b2.process(mypath)

# print out the summary
print(b2.statistics)

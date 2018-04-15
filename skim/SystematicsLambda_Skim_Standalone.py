#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# HIGH RETENTION RATE NOT PRODUCED
#######################################################
#
# Charm skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-01-00'

import sys
import os
import glob

fileList = ['generic_phase3_trackhypo.root']


inputMdstList('default', fileList)
loadStdCharged()

from SystematicsLambda_List import *
SysList = SystematicsList()

if 'Validation' in sys.argv and len(sys.argv) > 2:
    skimOutputUdst('SystematicsLambda_%s' % (sys.argv[sys.argv.index('Validation') + 1]), SysList)
else:
    skimOutputUdst('SystematicsLambda', SysList)

summaryOfLists(SysList)

if 'Validation' in sys.argv:
    ntupleFile('rec_Lambda.root')
    toolsdstar = ['EventMetaData', '^Lambda0 -> p+ pi-']
    toolsdstar += ['InvMass', '^Lambda0 -> p+ pi-']
    toolsdstar += ['Kinematics', '^Lambda0 -> p+ pi-']
    toolsdstar += ['Track', '^Lambda0 -> p+ pi-']
    toolsdstar += ['Vertex', '^Lambda0 -> p+ pi-']
    toolsdstar += ['MCTruth', '^Lambda0 -> p+ pi-']
    toolsdstar += ['CMSKinematics', '^Lambda0 -> p+ pi-']
    ntupleTree('Lambda0', 'Lambda0:syst0', toolsdstar)

process(analysis_main)

print(statistics)

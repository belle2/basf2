#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Systematic Tracking
# Y. Kato, Apr/2018
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdPhotons import *
from stdPi0s import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
set_log_level(LogLevel.INFO)
gb_setuprel = 'release-03-02-00'

import sys
import os
import glob

argvs = sys.argv
argc = len(argvs)

skimpath = Path()
fileList = get_test_file("mixedBGx1", "MC12")


inputMdstList('default', fileList, path=skimpath)
stdPi('loose', path=skimpath)
stdK('loose', path=skimpath)
stdPi0s('loose', path=skimpath)

skimCode = encodeSkimName('SystematicsTracking')

from skim.systematics import SystematicsTrackingList
SysList = SystematicsTrackingList(skimpath)
if 'Validation' in argvs and argc > 2:
    skimOutputUdst('%s_%s' % (skimCode, argvs[argvs.index('Validation') + 1]), SysList, path=skimpath)
else:
    skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

if 'Validation' in argvs:
    if argc > 2:
        ntupleFile('Validation_%s_%s.root' % (skimCode, argvs[argvs.index('Validation') + 1]), path=skimpath)
    else:
        ntupleFile('Validation_%s.root' % (skimCode), path=skimpath)

    toolsb = ['EventMetaData', '^B0']
    toolsb += ['InvMass', '^B0 -> ^D*- pi+']
    toolsb += ['Kinematics', '^B0 -> ^D*- ^pi+']
    toolsb += ['DeltaEMbc', '^B0']
    toolsb += ['MCTruth', '^B0 -> ^D*- pi+']
    toolsb += ['CustomFloats[massDifference(0)]', 'B0 -> ^D*- pi+']
    toolsb += ['CustomFloats[extraInfo(decayModeID)]', 'B0 -> [D*- -> ^anti-D0  pi+] pi+ ']
    ntupleTree('B0', 'B0:sys0', toolsb, path=skimpath)


setSkimLogging(path=skimpath)
process(skimpath)

print(statistics)

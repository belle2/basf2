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
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)
gb_setuprel = 'release-02-00-01'

import sys
import os
import glob

argvs = sys.argv
argc = len(argvs)

skimpath = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList, path=skimpath)
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


setSkimLogging(skim_path=skimpath)
process(skimpath)

print(statistics)

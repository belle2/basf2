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
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-00'

import sys
import os
import glob

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList)
loadStdCharged()

from SystematicsLambda_List import *
SysList = SystematicsLambdaList()

skimCode = encodeSkimName('SystematicsLambda')

argc = len(sys.argv)
argvs = sys.argv

if 'Validation' in sys.argv and len(sys.argv) > 2:
    skimOutputUdst('%s_%s' % (skimCode, argvs[argvs.index('Validation') + 1]), SysList)
else:
    skimOutputUdst(skimCode, SysList)

summaryOfLists(SysList)

if 'Validation' in sys.argv:
    if argc > 2:
        ntupleFile('Validation_%s_%s.root' % (skimCode, (argvs[argvs.index('Validation') + 1])))
    else:
        ntupleFile('Validation_%s.root' % (skimCode))

    toolsdstar = ['EventMetaData', '^Lambda0 -> p+ pi-']
    toolsdstar += ['InvMass', '^Lambda0 -> p+ pi-']
    toolsdstar += ['Kinematics', '^Lambda0 -> p+ pi-']
    toolsdstar += ['Track', '^Lambda0 -> p+ pi-']
    toolsdstar += ['Vertex', '^Lambda0 -> p+ pi-']
    toolsdstar += ['MCTruth', '^Lambda0 -> p+ pi-']
    toolsdstar += ['CMSKinematics', '^Lambda0 -> p+ pi-']
    ntupleTree('Lambda0', 'Lambda0:syst0', toolsdstar)


setSkimLogging()
process(analysis_main)

print(statistics)

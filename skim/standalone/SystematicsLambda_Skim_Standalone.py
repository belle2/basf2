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
from stdPhotons import *
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-03'


skimpath = Path()

fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=skimpath)

from skim.systematics import *
SysList = SystematicsLambdaList(path=skimpath)

skimCode = encodeSkimName('SystematicsLambda')

argc = len(sys.argv)
argvs = sys.argv

if 'Validation' in sys.argv and len(sys.argv) > 2:
    skimOutputUdst('%s_%s' % (skimCode, argvs[argvs.index('Validation') + 1]), SysList, path=skimpath)
else:
    skimOutputUdst(skimCode, SysList, path=skimpath)

summaryOfLists(SysList, path=skimpath)

if 'Validation' in sys.argv:
    if argc > 2:
        ntupleFile('Validation_%s_%s.root' % (skimCode, (argvs[argvs.index('Validation') + 1])), path=skimpath)
    else:
        ntupleFile('Validation_%s.root' % (skimCode), path=skimpath)

    toolsdstar = ['EventMetaData', '^Lambda0 -> p+ pi-']
    toolsdstar += ['InvMass', '^Lambda0 -> p+ pi-']
    toolsdstar += ['Kinematics', '^Lambda0 -> p+ pi-']
    toolsdstar += ['Track', '^Lambda0 -> p+ pi-']
    toolsdstar += ['Vertex', '^Lambda0 -> p+ pi-']
    toolsdstar += ['MCTruth', '^Lambda0 -> p+ pi-']
    toolsdstar += ['CMSKinematics', '^Lambda0 -> p+ pi-']
    ntupleTree('Lambda0', 'Lambda0:syst0', toolsdstar, path=skimpath)


setSkimLogging(path=skimpath)
process(skimpath)

print(statistics)

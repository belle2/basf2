#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# HIGH RETENTION RATE NOT PRODUCED
#######################################################
#
# Charm skims
# P. Urquijo, 6/Jan/2015
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdPhotons import *
import skimExpertFunctions as expert
b2.set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'


skimpath = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=skimpath)

from skim.systematics import *
SysList = SystematicsLambdaList(path=skimpath)

skimCode = expert.encodeSkimName('SystematicsLambda')

argc = len(sys.argv)
argvs = sys.argv

expert.skimOutputUdst(skimCode, SysList, path=skimpath)
ma.summaryOfLists(SysList, path=skimpath)

expert.setSkimLogging(path=skimpath)
b2.process(skimpath)

print(statistics)

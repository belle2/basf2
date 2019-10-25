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
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'


skimpath = Path()

fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=skimpath)

from skim.systematics import *
SysList = SystematicsLambdaList(path=skimpath)

skimCode = encodeSkimName('SystematicsLambda')

argc = len(sys.argv)
argvs = sys.argv

skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

setSkimLogging(path=skimpath)
process(skimpath)

print(statistics)

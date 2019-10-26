#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Modified by Y. Kato, Mar/2018
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPhotons import *

from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-04-00-00'
set_log_level(LogLevel.INFO)

skimpath = Path()

skimCode = encodeSkimName('Systematics')
argvs = sys.argv
argc = len(argvs)

fileList = get_test_file("mixedBGx1", "MC12")

inputMdstList('default', fileList, path=skimpath)

stdE('loose', path=skimpath)
stdMu('loose', path=skimpath)
stdPi('all', path=skimpath)
stdK('all', path=skimpath)
stdE('all', path=skimpath)
stdMu('all', path=skimpath)

from skim.systematics import SystematicsList
SysList = SystematicsList(skimpath)
skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

setSkimLogging(path=skimpath)
process(skimpath)

print(statistics)

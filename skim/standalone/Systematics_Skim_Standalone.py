#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Modified by R. Cheaib, April/2020
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
import sys
import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)

skimpath = b2.Path()

skimCode = expert.encodeSkimName('Systematics')
argvs = sys.argv
argc = len(argvs)

fileList = expert.get_test_file("MC13_ccbarBGx1")

ma.inputMdstList('default', fileList, path=skimpath)

# stdE('loose', path=skimpath)
# stdMu('loose', path=skimpath)
stdPi('all', path=skimpath)
stdK('all', path=skimpath)
# stdE('all', path=skimpath)
# stdMu('all', path=skimpath)

from skim.systematics import SystematicsList
SysList = SystematicsList(skimpath)
expert.skimOutputUdst(skimCode, SysList, path=skimpath)
ma.summaryOfLists(SysList, path=skimpath)

expert.setSkimLogging(path=skimpath)
b2.process(skimpath)

print(b2.statistics)

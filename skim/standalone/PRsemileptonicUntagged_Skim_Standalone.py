#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################
#
#
#   Partial Reconstruction of D* in charm B decays
#
#  L. Cremaldi, R. Godang, R.Cheaib
#
#
#
################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skimExpertFunctions import *
from skim.standardlists.charm import *
set_log_level(LogLevel.INFO)

gb2_setuprel = 'release-03-00-03'

skimCode = encodeSkimName('PRsemileptonicUntagged')

fileList = get_test_file("mixedBGx1", "MC11")
# create a new path
PRSLpath = Path()

inputMdstList('default', fileList, path=PRSLpath)
stdPi('all', path=PRSLpath)
stdE('all', path=PRSLpath)
stdMu('all', path=PRSLpath)
# PR Skim
from skim.semileptonic import PRList
PRList = PRList(path=PRSLpath)
skimOutputUdst(skimCode, PRList, path=PRSLpath)

summaryOfLists(PRList, path=PRSLpath)


setSkimLogging(path=PRSLpath)
process(path=PRSLpath)
# print out the summary
print(statistics)

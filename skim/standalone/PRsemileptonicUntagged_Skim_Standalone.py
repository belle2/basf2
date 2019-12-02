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

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdMu, stdPi
import skimExpertFunctions as expert
from skim.standardlists.charm import loadStdD0, loadStdDplus, loadStdDStar0, loadStdDstarPlus
b2.set_log_level(LogLevel.INFO)

gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('PRsemileptonicUntagged')

fileList = expert.get_test_file("mixedBGx1", "MC12")

# create a new path
PRSLpath = b2.Path()

ma.inputMdstList('default', fileList, path=PRSLpath)
stdPi('all', path=PRSLpath)
stdE('all', path=PRSLpath)
stdMu('all', path=PRSLpath)
# PR Skim
from skim.semileptonic import PRList
PRList = PRList(path=PRSLpath)
expert.skimOutputUdst(skimCode, PRList, path=PRSLpath)

ma.summaryOfLists(PRList, path=PRSLpath)


expert.setSkimLogging(path=PRSLpath)
b2.process(path=PRSLpath)
# print out the summary
print(statistics)

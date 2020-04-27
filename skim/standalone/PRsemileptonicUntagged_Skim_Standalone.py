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
b2.set_log_level(b2.LogLevel.INFO)

gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('PRsemileptonicUntagged')

fileList = expert.get_test_file("MC12_mixedBGx1")

# create a new path
path = b2.Path()

ma.inputMdstList('default', fileList, path=path)
stdPi('all', path=path)
stdE('all', path=path)
stdMu('all', path=path)
# PR Skim
from skim.semileptonic import PRList
PRList, path2 = PRList(path)
expert.skimOutputUdst(skimCode, PRList, path=path2)

ma.summaryOfLists(PRList, path=path2)


expert.setSkimLogging(path=path2)
b2.process(path)
# print out the summary
print(b2.statistics)

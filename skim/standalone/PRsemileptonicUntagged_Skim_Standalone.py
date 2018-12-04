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

gb2_setuprel = 'release-02-00-01'

skimCode = encodeSkimName('PRsemileptonicUntagged')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

# create a new path
PRSLpath = Path()

inputMdstList('MC9', fileList, path=PRSLpath)
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

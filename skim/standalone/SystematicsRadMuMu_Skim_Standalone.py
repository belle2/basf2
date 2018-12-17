#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Systematics Skims for radiative muon pairs
# Torben Ferber (torben.ferber@desy.de), 2018
# Sam Cunliffe (sam.cunliffe@desy.de)
#
#######################################################

from basf2 import process, statistics, Path, set_log_level, LogLevel
from modularAnalysis import inputMdstList, skimOutputUdst, summaryOfLists
from stdCharged import stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-01'

# create a path to build skim lists
skimpath = Path()

# input test data
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002314/e0000/4S/r00000/mumu_ecldigits/sub00/' +
    'mdst_000001_prod00002314_task00000001.root']
inputMdstList('MC9', fileList, path=skimpath)
stdMu('all', path=skimpath)

# setup the skim get the skim code
from skim.systematics import SystematicsRadMuMuList
SysList = SystematicsRadMuMuList(skimpath)
skimCode = encodeSkimName('SystematicsRadMuMu')
skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

# silence noisy modules
setSkimLogging(path=skimpath)

# process the path (run the skim)
process(skimpath)
print(statistics)

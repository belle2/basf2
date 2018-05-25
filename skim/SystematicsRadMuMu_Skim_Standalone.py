#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Systematics Skims for radiative muon pairs
# Torben Ferber (torben.ferber@desy.de), 2018
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from skimExpertFunctions import encodeSkimName

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-01-00-00'

import sys
import os
import glob

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002314/e0000/4S/r00000/mumu_ecldigits/sub00/' +
    'mdst_000001_prod00002314_task00000001.root']


inputMdstList('default', fileList)

loadStdCharged()

from SystematicsRadMuMu_List import *
SysList = SystematicsList()
skimCode = encodeSkimName('SystematicsRadMuMu')
skimOutputUdst(skimCode, SysList)
summaryOfLists(SysList)

setSkimLogging()
process(analysis_main)

print(statistics)

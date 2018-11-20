#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdV0s import *

from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-01'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
skimCode = encodeSkimName('Charm3BodyHadronic2')


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)

stdKshorts()
loadStdCharged()
from skim.charm import DstToD0PiD0ToHpHmKsList
DstToD0PiD0ToHpHmKsList = DstToD0PiD0ToHpHmKs()
skimOutputUdst(skimCode, DstToD0PiD0ToHpHmKsList)

summaryOfLists(DstToD0PiD0ToHpHmKsList)


setSkimLogging()
process(analysis_main)

print(statistics)

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
from stdCharged import stdPi, stdK, stdE, stdMu
from stdV0s import *
from stdPi0s import *
from skimExpertFunction import *
gb2_setuprel = 'release-02-00-01'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
skimCode = encodeSkimName('Charm3BodyHadronicD0')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)


loadStdSkimPi0()
stdKshorts()
stdPi('loose')
stdK('loose')
stdE('loose')
stdMu('loose')
stdPi('all')
stdK('all')
stdE('all')
stdMu('all')

from skim.charm import D0ToHpJmPi0List
D0ToHpJmPi0List = D0ToHpJmPi0()
skimOutputUdst(skimCode, D0ToHpJmPi0List)

summaryOfLists(D0ToHpJmPi0List)

setSkimLogging()
process(analysis_main)

print(statistics)

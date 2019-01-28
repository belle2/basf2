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
from skimExpertFunctions import *
gb2_setuprel = 'release-03-00-00'
set_log_level(LogLevel.INFO)
import sys
import os
import glob
skimCode = encodeSkimName('Charm3BodyHadronic3')

c3bh3path = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path=c3bh3path)


loadStdSkimPhoton(path=c3bh3path)
loadStdSkimPi0(path=c3bh3path)
stdPi('loose', path=c3bh3path)
stdK('loose', path=c3bh3path)
stdE('loose', path=c3bh3path)
stdMu('loose', path=c3bh3path)
stdPi('all', path=c3bh3path)
stdK('all', path=c3bh3path)
stdE('all', path=c3bh3path)
stdMu('all', path=c3bh3path)
stdKshorts(path=c3bh3path)

from skim.charm import DstToD0PiD0ToHpJmEta
DstToD0PiD0ToHpJmEtaList = DstToD0PiD0ToHpJmEta(c3bh3path)
skimOutputUdst(skimCode, DstToD0PiD0ToHpJmEtaList, path=c3bh3path)

summaryOfLists(DstToD0PiD0ToHpJmEtaList, path=c3bh3path)


setSkimLogging(path=c3bh3path)
process(c3bh3path)

print(statistics)

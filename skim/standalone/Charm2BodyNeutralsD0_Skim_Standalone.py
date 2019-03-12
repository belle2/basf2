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
import os
import sys
import glob
skimCode = encodeSkimName('Charm2BodyNeutralsD0')

c2bndpath = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path=c2bndpath)


loadStdSkimPi0(path=c2bndpath)
stdPi('loose', path=c2bndpath)
stdK('loose', path=c2bndpath)
stdPi('all', path=c2bndpath)
stdK('all', path=c2bndpath)
stdKshorts(path=c2bndpath)
mergedKshorts(path=c2bndpath)

from skim.charm import D0ToNeutrals

D0ToNeutralsList = D0ToNeutrals(c2bndpath)
skimOutputUdst(skimCode, D0ToNeutralsList, path=c2bndpath)

summaryOfLists(D0ToNeutralsList, path=c2bndpath)


setSkimLogging(path=c2bndpath)
process(c2bndpath)

print(statistics)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#

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
gb2_setuprel = 'release-02-00-01'

import os
import sys
import glob
skimCode = encodeSkimName('Charm2BodyNeutrals2')

c2bn2path = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path=c2bn2path)

stdPi('loose', path=c2bn2path)
stdK('loose', path=c2bn2path)
stdE('loose', path=c2bn2path)
stdMu('loose', path=c2bn2path)
stdPi('all', path=c2bn2path)
stdK('all', path=c2bn2path)
stdE('all', path=c2bn2path)
stdMu('all', path=c2bn2path)
stdKshorts(path=c2bn2path)
mergedKshorts(path=c2bn2path)
loadStdSkimPi0(path=c2bn2path)

from skim.charm import DstToD0PiD0ToKsOmega
DstList = DstToD0PiD0ToKsOmega(c2bn2path)
skimOutputUdst(skimCode, DstList, path=c2bn2path)

summaryOfLists(DstList, path=c2bn2path)

setSkimLogging(path=c2bn2path)
process(c2bn2path)

print(statistics)

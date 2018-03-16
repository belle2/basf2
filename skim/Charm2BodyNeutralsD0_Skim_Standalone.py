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
from stdPi0s import *

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('default', fileList)


loadStdSkimPi0()
loadStdCharged()
loadStdKS()
stdPi0s()

from Charm2BodyNeutralsD0_List import *

D0ToNeutralsList = D0ToNeutrals()
skimOutputUdst('Charm2BodyNeutralsD0', D0ToNeutralsList)

summaryOfLists(D0ToNeutralsList)


process(analysis_main)

print(statistics)

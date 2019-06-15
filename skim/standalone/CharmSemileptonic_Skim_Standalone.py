#!/usr/bin/evn python3
# -*- coding: utf-8 -*-

##################################################
#
# Charm SL skims
# J. Bennett, 5/Oct/2016
#
##################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdPi0s import *
from stdCharged import stdPi, stdK, stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'

import os
import sys
import glob
skimCode = encodeSkimName("CharmSemileptonic")

cslpath = Path()
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=cslpath)


stdPi('95eff', path=cslpath)
stdK('95eff', path=cslpath)
stdE('95eff', path=cslpath)
stdMu('95eff', path=cslpath)
loadStdSkimPi0(path=cslpath)

reconstructDecay('K_S0:all -> pi-:95eff pi+:95eff', '0.4 < M < 0.6', 1, True, path=cslpath)
vertexKFit('K_S0:all', 0.0, path=cslpath)
applyCuts('K_S0:all', '0.477614 < M < 0.517614', path=cslpath)


# CSL Skim
from skim.charm import CharmSemileptonicList
CSLList = CharmSemileptonicList(cslpath)
skimOutputUdst(skimCode, CSLList, path=cslpath)
summaryOfLists(CSLList, path=cslpath)

setSkimLogging(path=cslpath)
process(cslpath)

# print out the summary
print(statistics)

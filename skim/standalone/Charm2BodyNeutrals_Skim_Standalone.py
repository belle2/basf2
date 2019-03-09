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
gb2_setuprel = 'release-03-00-03'

import os
import sys
import glob
skimCode = encodeSkimName('Charm2BodyNeutrals')

c2bnpath = Path()

fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=c2bnpath)


stdPi('loose', path=c2bnpath)
stdK('loose', path=c2bnpath)
stdE('loose', path=c2bnpath)
stdMu('loose', path=c2bnpath)
stdPi('all', path=c2bnpath)
stdK('all', path=c2bnpath)
stdE('all', path=c2bnpath)
stdMu('all', path=c2bnpath)
stdKshorts(path=c2bnpath)
mergedKshorts(path=c2bnpath)
loadStdSkimPi0(path=c2bnpath)

from skim.charm import DstToD0Neutrals
DstList = DstToD0Neutrals(c2bnpath)
skimOutputUdst(skimCode, DstList, path=c2bnpath)

summaryOfLists(DstList, path=c2bnpath)

setSkimLogging(path=c2bnpath)
process(path=c2bnpath)

print(statistics)

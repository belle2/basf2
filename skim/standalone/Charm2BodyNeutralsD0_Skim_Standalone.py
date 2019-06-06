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
gb2_setuprel = 'release-03-00-03'
skimCode = encodeSkimName('Charm2BodyNeutralsD0')

c2bndpath = Path()

fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=c2bndpath)


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

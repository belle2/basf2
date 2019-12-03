#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdPi0s import loadStdSkimPi0
from stdV0s import mergedKshorts, stdKshorts
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'
skimCode = expert.encodeSkimName('XToD0_D0ToNeutrals')

c2bndpath = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=c2bndpath)


loadStdSkimPi0(path=c2bndpath)
stdPi('loose', path=c2bndpath)
stdK('loose', path=c2bndpath)
stdPi('all', path=c2bndpath)
stdK('all', path=c2bndpath)
stdKshorts(path=c2bndpath)
mergedKshorts(path=c2bndpath)

from skim.charm import D0ToNeutrals

D0ToNeutralsList = D0ToNeutrals(c2bndpath)
expert.skimOutputUdst(skimCode, D0ToNeutralsList, path=c2bndpath)

ma.summaryOfLists(D0ToNeutralsList, path=c2bndpath)


expert.setSkimLogging(path=c2bndpath)
b2.process(c2bndpath)

print(b2.statistics)

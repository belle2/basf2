#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#

# G. Casarosa, 7/Oct/2016
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPi0s import loadStdSkimPi0
from stdV0s import stdKshorts
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('DstToD0Pi_D0ToNeutrals')

c2bnpath = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=c2bnpath)


stdPi('loose', path=c2bnpath)
stdK('loose', path=c2bnpath)
stdE('loose', path=c2bnpath)
stdMu('loose', path=c2bnpath)
stdPi('all', path=c2bnpath)
stdK('all', path=c2bnpath)
stdE('all', path=c2bnpath)
stdMu('all', path=c2bnpath)
stdKshorts(path=c2bnpath)
loadStdSkimPi0(path=c2bnpath)

from skim.charm import DstToD0Neutrals
DstList = DstToD0Neutrals(c2bnpath)
expert.skimOutputUdst(skimCode, DstList, path=c2bnpath)

ma.summaryOfLists(DstList, path=c2bnpath)

expert.setSkimLogging(path=c2bnpath)
b2.process(path=c2bnpath)

print(b2.statistics)

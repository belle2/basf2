#!/usr/bin/evn python3
# -*- coding: utf-8 -*-

##################################################
#
# Charm SL skims
# J. Bennett, 5/Oct/2016
#
##################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPi0s import loadStdSkimPi0
import skimExpertFunctions as expert
import vertex
gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName("DstToD0Pi_D0ToSemileptonic")

cslpath = b2.Path()
fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=cslpath)


stdPi('95eff', path=cslpath)
stdK('95eff', path=cslpath)
stdE('95eff', path=cslpath)
stdMu('95eff', path=cslpath)
loadStdSkimPi0(path=cslpath)

ma.reconstructDecay('K_S0:all -> pi-:95eff pi+:95eff', '0.4 < M < 0.6', 1, True, path=cslpath)
vertex.vertexKFit('K_S0:all', 0.0, path=cslpath)
ma.applyCuts('K_S0:all', '0.477614 < M < 0.517614', path=cslpath)


# CSL Skim
from skim.charm import CharmSemileptonic
CSLList = CharmSemileptonic(cslpath)
expert.skimOutputUdst(skimCode, CSLList, path=cslpath)
ma.summaryOfLists(CSLList, path=cslpath)

expert.setSkimLogging(path=cslpath)
b2.process(cslpath)

# print out the summary
print(b2.statistics)

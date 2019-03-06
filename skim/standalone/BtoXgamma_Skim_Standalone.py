#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP standalone skim steering
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from stdPhotons import *
from skimExpertFunctions import setSkimLogging, encodeSkimName

gb2_setuprel = 'release-03-00-03'


skimCode = encodeSkimName('BtoXgamma')
fileList = get_test_file("mixedBGx1", "MC11")
path = Path()
inputMdstList('default', fileList, path=path)
stdPi0s('loose', path=path)
stdPhotons('tight', path=path)  # also builds loose list
loadStdSkimPhoton(path=path)
loadStdSkimPi0(path=path)
stdPi('loose', path=path)
stdK('loose', path=path)
stdK('95eff', path=path)
stdPi('95eff', path=path)
stdKshorts(path=path)
loadStdLightMesons(path=path)

cutAndCopyList('gamma:ewp', 'gamma:loose', 'E > 0.1', path=path)
reconstructDecay('eta:ewp -> gamma:ewp gamma:ewp', '0.505 < M < 0.580', path=path)
# EWP Skim
from skim.ewp import B2XgammaList
XgammaList = B2XgammaList(path=path)
skimOutputUdst(skimCode, XgammaList, path=path)
summaryOfLists(XgammaList, path=path)


setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)

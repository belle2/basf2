#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
# B -> D(Kpi) h skims
# Niharika Rout
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdPi0s import *
from stdV0s import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-04'

path = Path()

skimCode = encodeSkimName('BtoDh_Kpi')

fileList = get_test_file("mixedBGx1", "MC12")

inputMdstList('default', fileList, path=path)


# create and fill pion and kaon ParticleLists
stdPi('all', path=path)
stdK('all', path=path)

applyCuts(list_name='pi+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)
applyCuts(list_name='K+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)

# B+ to D(->h+h-)h+ Skim
from skim.HadronicBValidation import loadDKpi, BsigToDhToKpiList
loadDKpi(path=path)
BtoDhList = BsigToDhToKpiList(path=path)
skimOutputUdst(skimCode, BtoDhList, path=path)
summaryOfLists(BtoDhList, path=path)

setSkimLogging(path)
process(path)

# print out the summary
print(statistics)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(Kpipipi) h skims
# Niharika Rout
#
######################################################
from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK
from skim.standardlists.charm import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-04'

path = b2.create_path()

skimCode = encodeSkimName('BtoDh_Kpipipi')

fileList = get_test_file("mixedBGx1", "MC12")

ma.inputMdstList('default', fileList, path=path)

stdPi('all', path=path)
stdK('all', path=path)

ma.applyCuts(list_name='pi+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)
ma.applyCuts(list_name='K+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)

# B- to D(->Kpipipi)h- Skim
from skim.btohadron_validation import loadD0_Kpipipi, BsigToDhToKpipipiList
loadD0_Kpipipi(path=path)
BtoDhList = BsigToDhToKpipipiList(path=path)
skimOutputUdst(skimCode, BtoDhList, path=path)
summaryOfLists(BtoDhList, path=path)


setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)

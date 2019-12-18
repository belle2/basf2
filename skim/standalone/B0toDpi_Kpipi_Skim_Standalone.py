#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# neutral B to charm mode:
# B0 -> D-(K+ pi- pi-)pi+
#
# Chiara La Licata 2019/Oct/18
#
######################################################

from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file, skimOutputUdst

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-04'

mypath = b2.Path()

skimCode = encodeSkimName('B0toDpi_Kpipi')

fileList = get_test_file("MC12_mixedBGx1")

ma.inputMdstList('default', fileList, path=mypath)

# create and fill pion and kaon ParticleLists
stdPi('all', path=mypath)
stdK('all', path=mypath)

ma.applyCuts(list_name='pi+:all', cut='abs(dr) < 2 and abs(dz) < 5',
             path=mypath)
ma.applyCuts(list_name='K+:all', cut='abs(dr) < 2 and abs(dz) < 5',
             path=mypath)


# B0 to D-(K+ pi- pi-)pi+ Skim
from skim.btocharm import loadB0toDpi_Kpipi
from skim.standardlists.charm import loadStdDplus_Kpipi
loadStdDplus_Kpipi(path=mypath)
B0toDpiList = loadB0toDpi_Kpipi(path=mypath)

skimOutputUdst(skimCode, B0toDpiList, path=mypath)
ma.summaryOfLists(B0toDpiList, path=mypath)

setSkimLogging(mypath)
b2.process(mypath)

# print out the summary
print(b2.statistics)

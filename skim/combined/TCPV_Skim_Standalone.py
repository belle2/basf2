#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# TCPV skims
# P. Urquijo, 29/Sep/2016
#
#####################################################

__author__ = " Reem Rasheed"


import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import loadStdLightMesons
from skim.standardlists.dileptons import loadStdDiLeptons, loadStdJpsiToee, loadStdJpsiTomumu
from skim.standardlists.charm import loadStdD0_Kpi, loadStdD0_Kpipipi
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'
b2.set_log_level(b2.LogLevel.INFO)


skimCode = expert.encodeSkimName('TCPV')

# create a path
path = b2.Path()

fileList = expert.get_test_file("MC12_mixedBGx1")

ma.inputMdstList('default', fileList, path=path)

ma.fillParticleList('K+:1%',  cut="dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance and kaonID > 0.01", path=path)
ma.fillParticleList('e+:withCuts',  cut="dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance", path=path)
ma.fillParticleList(decayString='gamma:e+', cut="E < 1.0", path=path)
ma.correctBrems(outputList='e+:bremCorr', inputList='e+:withCuts', gammaList='gamma:e+', multiplePhotons=False,
                usePhotonOnlyOnce=True, writeOut=True, path=path)
ma.fillParticleList('mu+:withCuts',  cut="dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance", path=path)
loadStdSkimPi0(path=path)
loadStdSkimPhoton(path=path)
stdPi0s('eff40_Jan2020', path=path)
stdPi('loose', path=path)
stdK('all', path=path)
stdK('loose', path=path)
stdE('loose', path=path)
stdMu('loose', path=path)
stdPi('all', path=path)
stdPhotons('loose', path=path)
stdKshorts(path=path)
loadStdDiLeptons(True, path=path)
loadStdLightMesons(path=path)
ma.cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=path)
loadStdJpsiToee(path=path)
loadStdJpsiTomumu(path=path)
loadStdD0_Kpi(path=path)
loadStdD0_Kpipipi(path=path)

# TCPV Skim
from skim.tcpv import TCPVList
tcpvList = TCPVList(path=path)
expert.skimOutputUdst(skimCode, tcpvList, path=path)
ma.summaryOfLists(tcpvList, path=path)

expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)

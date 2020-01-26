#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Run TCPV skim
######################################################

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


skimpath = b2.Path()
fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=skimpath)


ma.fillParticleList('e+:all',  cut="dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance", path=skimpath)
ma.fillParticleList('mu+:all',  cut="dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance", path=skimpath)
ma.fillParticleList('K+:1%',  cut="dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance and kaonID > 0.01", path=skimpath)

stdPi('loose', path=skimpath)
stdK('loose', path=skimpath)
stdK('all', path=skimpath)
stdE('loose', path=skimpath)
stdMu('loose', path=skimpath)
stdPi('all', path=skimpath)
stdPi0s('loose', path=skimpath)
stdPhotons('loose', path=skimpath)
stdKshorts(path=skimpath)
stdE('all', path=skimpath)
stdMu('all', path=skimpath)


ma.cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=skimpath)


loadStdLightMesons(path=skimpath)
stdPhotons('all', path=skimpath)
loadStdSkimPi0(path=skimpath)
loadStdSkimPhoton(path=skimpath)
loadStdDiLeptons(True, path=skimpath)
loadStdJpsiToee(path=skimpath)
loadStdJpsiTomumu(path=skimpath)
loadStdD0_Kpi(path=skimpath)
loadStdD0_Kpipipi(path=skimpath)


# TCPV Skim
from skim.tcpv import TCPVList
expert.add_skim('TCPV', TCPVList(path=skimpath), path=skimpath)

# print out the summary
print(b2.statistics)

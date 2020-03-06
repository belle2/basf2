#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# All BtoCharmSkims in one _standalone
# Nibedita Dash
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import loadStdLightMesons, loadStdPi0ForBToHadrons
import skimExpertFunctions as expert


btocharmpath = b2.Path()

fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=btocharmpath)


stdPi('all', path=btocharmpath)
stdK('all', path=btocharmpath)
loadStdPi0ForBToHadrons(path=btocharmpath)

# B- to D(->Kpipipi)h- Skim
from skim.btocharm import BtoD0h_Kpipipi_Kpipi0List
from skim.standardlists.charm import loadStdD0_Kpipipi, loadStdD0_Kpipi0
from skim.standardlists.charm import loadStdDstar0_D0pi0_Kpipipi, loadStdDstar0_D0pi0_Kpipi0

loadStdD0_Kpipipi(path=btocharmpath)
loadStdD0_Kpipi0(path=btocharmpath)
loadStdDstar0_D0pi0_Kpipipi(path=btocharmpath)
loadStdDstar0_D0pi0_Kpipi0(path=btocharmpath)
BtoD0h_Kpipipi_list = BtoD0h_Kpipipi_Kpipi0List(path=btocharmpath)
expert.add_skim('BtoD0h_Kpipipi_Kpipi0', BtoD0h_Kpipipi_list, path=btocharmpath)

# B+ to D(->h+h-)h+ Skim
from skim.btocharm import BsigToD0hToKpiList
from skim.standardlists.charm import loadStdD0_Kpi

loadStdD0_Kpi(path=btocharmpath)
BtoD0h_Kpi_list = BsigToD0hToKpiList(path=btocharmpath)
expert.add_skim('BtoD0h_Kpi', BtoD0h_Kpi_list, path=btocharmpath)

# B0 to D-(K+ pi- pi-)pi+ Skim
from skim.btocharm import loadB0toDpi_Kpipi
from skim.standardlists.charm import loadStdDplus_Kpipi

loadStdDplus_Kpipi(path=btocharmpath)
B0toDpiList = loadB0toDpi_Kpipi(path=btocharmpath)
expert.add_skim('B0toDpi_Kpipi', B0toDpiList, path=btocharmpath)

# B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> K+ pi-
from skim.btocharm import loadB0toDstarPi_Kpi
from skim.standardlists.charm import loadStdD0_Kpi, loadStdDstarPlus_D0pi_Kpi

loadStdDstarPlus_D0pi_Kpi(path=btocharmpath)
B0toDstarPiList_Kpi = loadB0toDstarPi_Kpi(path=btocharmpath)
expert.add_skim('B0toDstarPi_D0pi_Kpi', B0toDstarPiList_Kpi, path=btocharmpath)

# B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> K- pi+ pi- pi+ Skim
from skim.btocharm import loadB0toDstarPi_Kpipipi_Kpipi0List
from skim.standardlists.charm import loadStdD0_Kpipi0, loadStdDstarPlus_D0pi_Kpipi0
from skim.standardlists.charm import loadStdD0_Kpipipi, loadStdDstarPlus_D0pi_Kpipipi
loadStdD0_Kpipi0(path=btocharmpath)
loadStdDstarPlus_D0pi_Kpipi0(path=btocharmpath)
loadStdD0_Kpipipi(path=btocharmpath)
loadStdDstarPlus_D0pi_Kpipipi(path=btocharmpath)
B0toDstarPiList_Kpipipi = loadB0toDstarPi_Kpipipi_Kpipi0List(path=btocharmpath)
expert.add_skim('B0toDstarPi_D0pi_Kpipipi_Kpipi0', B0toDstarPiList_Kpipipi, path=btocharmpath)


expert.setSkimLogging(path=btocharmpath)
b2.process(path=btocharmpath)

# print out the summary
print(b2.statistics)

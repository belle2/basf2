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
from skim.standardlists.lightmesons import loadStdLightMesons, loadStdPi0ForBToHadrons, loadStdAllRhoPlus
import skimExpertFunctions as expert


btocharmpath = b2.Path()

fileList = expert.get_test_file("MC13_mixedBGx1")
ma.inputMdstList('default', fileList, path=btocharmpath)

stdKshorts(path=btocharmpath)
stdPi('all', path=btocharmpath)
stdK('all', path=btocharmpath)
loadStdPi0ForBToHadrons(path=btocharmpath)
loadStdAllRhoPlus(path=btocharmpath)


# B- to D(->Kpipipi and -> Kpipi)h- Skim
from skim.btocharm import BtoD0h_Kpipipi_Kpipi0List
from skim.standardlists.charm import loadStdD0_Kpipipi, loadStdD0_Kpipi0
from skim.standardlists.charm import loadStdDstar0_D0pi0_Kpipipi, loadStdDstar0_D0pi0_Kpipi0
loadStdD0_Kpipipi(path=btocharmpath)
loadStdD0_Kpipi0(path=btocharmpath)
loadStdDstar0_D0pi0_Kpipipi(path=btocharmpath)
loadStdDstar0_D0pi0_Kpipi0(path=btocharmpath)
BtoD0h_Kpipipi_list = BtoD0h_Kpipipi_Kpipi0List(path=btocharmpath)
expert.add_skim('BtoD0h_Kpipipi_Kpipi0', BtoD0h_Kpipipi_list, path=btocharmpath)

# B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> K- pi+ pi- pi+ Skim
from skim.btocharm import loadB0toDstarPi_Kpipipi_Kpipi0List
from skim.standardlists.charm import loadStdDstarPlus_D0pi_Kpipi0
from skim.standardlists.charm import loadStdDstarPlus_D0pi_Kpipipi
loadStdDstarPlus_D0pi_Kpipi0(path=btocharmpath)
loadStdDstarPlus_D0pi_Kpipipi(path=btocharmpath)
B0toDstarPiList_Kpipipi = loadB0toDstarPi_Kpipipi_Kpipi0List(path=btocharmpath)
expert.add_skim('B0toDstarPi_D0pi_Kpipipi_Kpipi0', B0toDstarPiList_Kpipipi, path=btocharmpath)

# B+ -> anti-D0 rho+, anti-D*0( -> D0 pi0) rho+
from skim.btocharm import B0toDstarRho_Kpipipi_Kpipi0List
B0toDstarRho_Kpipipi_Kpipi0_list = B0toDstarRho_Kpipipi_Kpipi0List(path=btocharmpath)
expert.add_skim('B0toDstarRho_D0pi_Kpipipi_Kpipi0', B0toDstarRho_Kpipipi_Kpipi0_list, path=btocharmpath)

# B+ -> anti-D0 rho+, anti-D*0( -> D0 pi0) rho+
from skim.btocharm import BtoD0rho_Kpipipi_Kpipi0List
BtoD0rho_Kpipipi_Kpipi0_list = BtoD0rho_Kpipipi_Kpipi0List(path=btocharmpath)
expert.add_skim('BtoD0rho_Kpipipi_Kpipi0', BtoD0rho_Kpipipi_Kpipi0_list, path=btocharmpath)

# B+ to D(->h+h-)h+ Skim
from skim.btocharm import BsigToD0hToKpiList
from skim.standardlists.charm import loadStdD0_Kpi
loadStdD0_Kpi(path=btocharmpath)
BtoD0h_Kpi_list = BsigToD0hToKpiList(path=btocharmpath)
expert.add_skim('BtoD0h_Kpi', BtoD0h_Kpi_list, path=btocharmpath)


# B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> K+ pi-
from skim.btocharm import loadB0toDstarPi_Kpi
from skim.standardlists.charm import loadStdDstarPlus_D0pi_Kpi
loadStdDstarPlus_D0pi_Kpi(path=btocharmpath)
B0toDstarPiList_Kpi = loadB0toDstarPi_Kpi(path=btocharmpath)
expert.add_skim('B0toDstarPi_D0pi_Kpi', B0toDstarPiList_Kpi, path=btocharmpath)

# B+ -> anti-D0 (-> Kpi) rho+, anti-D*0( -> D0(-> Kpi) pi0) rho+
from skim.btocharm import BtoD0rho_KpiList
from skim.standardlists.charm import loadStdDstar0_D0pi0_Kpi
loadStdDstar0_D0pi0_Kpi(path=btocharmpath)
BtoD0rho_Kpi_list = BtoD0rho_KpiList(path=btocharmpath)
expert.add_skim('BtoD0rho_Kpi', BtoD0rho_Kpi_list, path=btocharmpath)


# B0 -> D*-( -> D0(-> Kpi) pi-) rho+
from skim.btocharm import B0toDstarRho_KpiList
B0toDstarRho_D0pi_Kpi_list = B0toDstarRho_KpiList(path=btocharmpath)
expert.add_skim('B0toDstarRho_D0pi_Kpi', B0toDstarRho_D0pi_Kpi_list, path=btocharmpath)


expert.setSkimLogging(path=btocharmpath)
b2.process(path=btocharmpath)

# print out the summary
print(b2.statistics)

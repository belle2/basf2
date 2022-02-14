#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from stdPi0s import stdPi0s
import modularAnalysis as ma


def bToChHltSkim(path):
    """
    Function to reconstruct B meson candidates for HLT skims
    @param path         modules are added to this path
    """

    BpList = []
    BzList = []

    # Light mesons
    ma.fillParticleList("pi+:GoodTrack", "abs(d0) < 2 and abs(z0) < 5", path=path)
    ma.fillParticleList("K+:GoodTrack", "abs(d0) < 2 and abs(z0) < 5", path=path)
    stdPi0s('all', path, loadPhotonBeamBackgroundMVA=False)
    ma.cutAndCopyList(outputListName='pi0:bth_skim', inputListName='pi0:all',
                      cut='[[daughter(0, clusterReg) == 1 and daughter(0, E) > 0.02250] or ' +
                      '[daughter(0, clusterReg) == 2 and daughter(0, E) > 0.020] or ' +
                      '[daughter(0, clusterReg) == 3 and daughter(0, E) > 0.020]] and ' +
                      '[[daughter(1, clusterReg) == 1 and daughter(1, E) > 0.02250] or ' +
                      '[daughter(1, clusterReg) == 2 and daughter(1, E) > 0.020] or ' +
                      '[daughter(1, clusterReg) == 3 and daughter(1, E) > 0.020]] and ' +
                      'M > 0.105 and M < 0.150', path=path)

    # D0 lists
    ma.reconstructDecay(decayString='D0:Kpi -> K-:GoodTrack pi+:GoodTrack', cut='1.7 < M < 2.0', path=path)
    ma.reconstructDecay(decayString='D0:Kpipi0 -> K-:GoodTrack pi+:GoodTrack pi0:bth_skim', cut='1.7 < M < 2.0', path=path)
    ma.reconstructDecay(decayString='D0:Kpipipi -> K-:GoodTrack pi+:GoodTrack pi+:GoodTrack pi-:GoodTrack',
                        cut='1.7 < M < 2.0', path=path)
    # D+ list
    ma.reconstructDecay(decayString='D+:Kpipi -> K-:GoodTrack pi+:GoodTrack pi+:GoodTrack', cut='1.8 < M < 1.9', path=path)

    # D*+ lists
    ma.reconstructDecay(decayString='D*+:D0_Kpi -> D0:Kpi pi+:GoodTrack', cut='massDifference(0) < 0.16', path=path)
    ma.reconstructDecay(decayString='D*+:D0_Kpipi0 -> D0:Kpipi0 pi+:GoodTrack', cut='massDifference(0) < 0.16', path=path)
    ma.reconstructDecay(decayString='D*+:D0_Kpipipi -> D0:Kpipipi pi+:GoodTrack', cut='massDifference(0) < 0.16', path=path)

    # B+ lists
    ma.reconstructDecay("B+:BtoD0pi_Kpi -> anti-D0:Kpi pi+:GoodTrack", "Mbc > 5.2 and abs(deltaE) < 0.5", path=path)
    BpList.append("B+:BtoD0pi_Kpi")
    ma.reconstructDecay("B+:BtoD0pi_Kpipi0 -> anti-D0:Kpipi0 pi+:GoodTrack", "Mbc > 5.2 and abs(deltaE) < 0.3", path=path)
    BpList.append("B+:BtoD0pi_Kpipi0")
    ma.reconstructDecay("B+:BtoD0pi_Kpipipi -> anti-D0:Kpipipi pi+:GoodTrack", "Mbc > 5.2 and abs(deltaE) < 0.3", path=path)
    BpList.append("B+:BtoD0pi_Kpipipi")

    # B0 lists
    ma.reconstructDecay("B0:B0toDpi_Kpipi -> D-:Kpipi pi+:GoodTrack", "5.2 < Mbc and abs(deltaE) < 0.3", path=path)
    BzList.append("B0:B0toDpi_Kpipi")
    ma.reconstructDecay("B0:B0toDstarPi_D0pi_Kpi -> D*-:D0_Kpi pi+:GoodTrack", "5.2 < Mbc and abs(deltaE) < 0.3", path=path)
    BzList.append("B0:B0toDstarPi_D0pi_Kpi")
    ma.reconstructDecay("B0:B0toDstarPi_D0pi_Kpipipi -> D*-:D0_Kpipipi pi+:GoodTrack", "5.2 < Mbc and abs(deltaE) < 0.3", path=path)
    BzList.append("B0:B0toDstarPi_D0pi_Kpipipi")
    ma.reconstructDecay("B0:B0toDstarPi_D0pi_Kpipi0 -> D*-:D0_Kpipi0 pi+:GoodTrack", "5.2 < Mbc and abs(deltaE) < 0.3", path=path)
    BzList.append("B0:B0toDstarPi_D0pi_Kpipi0")

    ma.copyLists("B+:btoch", BpList, path=path)
    ma.copyLists("B0:btoch", BzList, path=path)

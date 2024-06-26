#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma


def bToCharmHLTSkim(path):
    """
    Function to reconstruct B meson candidates for HLT skims
    @param path         modules are added to this path
    """

    BplusList = []
    BzeroList = []

    # Light mesons
    ma.fillParticleList("pi+:GoodTrackForHLT", "abs(d0) < 2 and abs(z0) < 5", path=path)
    ma.fillParticleList("K+:GoodTrackForHLT", "abs(d0) < 2 and abs(z0) < 5", path=path)
    ma.fillParticleList('gamma:GoodPi0ForHLT', '[clusterReg == 1 and E > 0.02250] or \
                                                [clusterReg == 2 and E > 0.020] or \
                                                [clusterReg == 3 and E > 0.020]',
                        writeOut=True, path=path)
    ma.reconstructDecay('pi0:GoodPi0ForHLT -> gamma:GoodPi0ForHLT gamma:GoodPi0ForHLT', '0.105 < M < 0.150', 1, True, path)
    ma.matchMCTruth('pi0:GoodPi0ForHLT', path)

    # D0 lists
    ma.reconstructDecay(decayString='D0:KpiForHLT -> K-:GoodTrackForHLT pi+:GoodTrackForHLT', cut='1.7 < M < 2.0', path=path)
    ma.reconstructDecay(
        decayString='D0:Kpipi0ForHLT -> K-:GoodTrackForHLT pi+:GoodTrackForHLT pi0:GoodPi0ForHLT',
        cut='1.7 < M < 2.0',
        path=path)
    ma.reconstructDecay(
        decayString='D0:KpipipiForHLT -> K-:GoodTrackForHLT pi+:GoodTrackForHLT pi+:GoodTrackForHLT pi-:GoodTrackForHLT',
        cut='1.7 < M < 2.0',
        path=path)
    # D+ list
    ma.reconstructDecay(
        decayString='D+:KpipiForHLT -> K-:GoodTrackForHLT pi+:GoodTrackForHLT pi+:GoodTrackForHLT',
        cut='1.8 < M < 1.9',
        path=path)

    # D*+ lists
    ma.reconstructDecay(
        decayString='D*+:D0_KpiForHLT -> D0:KpiForHLT pi+:GoodTrackForHLT',
        cut='massDifference(0) < 0.16',
        path=path)
    ma.reconstructDecay(
        decayString='D*+:D0_Kpipi0ForHLT -> D0:Kpipi0ForHLT pi+:GoodTrackForHLT',
        cut='massDifference(0) < 0.16',
        path=path)
    ma.reconstructDecay(
        decayString='D*+:D0_KpipipiForHLT -> D0:KpipipiForHLT pi+:GoodTrackForHLT',
        cut='massDifference(0) < 0.16',
        path=path)

    # B+ lists
    ma.reconstructDecay("B+:BtoD0pi_KpiForHLT -> anti-D0:KpiForHLT pi+:GoodTrackForHLT",
                        "Mbc > 5.15 and abs(deltaE) < 0.5", path=path)
    BplusList.append("B+:BtoD0pi_KpiForHLT")
    ma.reconstructDecay("B+:BtoD0pi_Kpipi0ForHLT -> anti-D0:Kpipi0ForHLT pi+:GoodTrackForHLT",
                        "Mbc > 5.15 and abs(deltaE) < 0.3", path=path)
    BplusList.append("B+:BtoD0pi_Kpipi0ForHLT")
    ma.reconstructDecay("B+:BtoD0pi_KpipipiForHLT -> anti-D0:KpipipiForHLT pi+:GoodTrackForHLT",
                        "Mbc > 5.15 and abs(deltaE) < 0.3", path=path)
    BplusList.append("B+:BtoD0pi_KpipipiForHLT")

    # B0 lists
    ma.reconstructDecay("B0:B0toDpi_KpipiForHLT -> D-:KpipiForHLT pi+:GoodTrackForHLT",
                        "5.15 < Mbc and abs(deltaE) < 0.3", path=path)
    BzeroList.append("B0:B0toDpi_KpipiForHLT")
    ma.reconstructDecay("B0:B0toDstarPi_D0pi_KpiForHLT -> D*-:D0_KpiForHLT pi+:GoodTrackForHLT",
                        "5.15 < Mbc and abs(deltaE) < 0.3", path=path)
    BzeroList.append("B0:B0toDstarPi_D0pi_KpiForHLT")
    ma.reconstructDecay("B0:B0toDstarPi_D0pi_KpipipiForHLT -> D*-:D0_KpipipiForHLT pi+:GoodTrackForHLT",
                        "5.15 < Mbc and abs(deltaE) < 0.3", path=path)
    BzeroList.append("B0:B0toDstarPi_D0pi_KpipipiForHLT")
    ma.reconstructDecay("B0:B0toDstarPi_D0pi_Kpipi0ForHLT -> D*-:D0_Kpipi0ForHLT pi+:GoodTrackForHLT",
                        "5.15 < Mbc and abs(deltaE) < 0.3", path=path)
    BzeroList.append("B0:B0toDstarPi_D0pi_Kpipi0ForHLT")

    ma.copyLists("B+:BtoCharmForHLT", BplusList, path=path)
    ma.copyLists("B0:BtoCharmForHLT", BzeroList, path=path)

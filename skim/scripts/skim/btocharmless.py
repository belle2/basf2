#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Skim list building functions for :math:`B\\to X_u +h` analyses
"""

__authors__ = [
    "Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"
]

import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header
from stdV0s import stdKshorts
from skim.standardlists.charmless import (
    loadStdVeryLooseTracks,
    loadStdPi0ForBToCharmless,
    loadStdVeryLooseKstar0,
    loadStdVeryLooseRho0,
    loadStdVeryLooseKstarPlus,
    loadStdVeryLooseKstarPlusPi0,
    loadStdVeryLooseRhoPlus)


__liaison__ = "Benedikt Wach <benedikt.wach@desy.de>"
__validation_sample___ = "mdst14.root"


@fancy_skim_header
class BtoPi0Pi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\B^0 -> \\pi^0 \\pi^0`

    Cuts applied:

    * ``5.24 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``

    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Francis Pham", "Benedikt Wach"]
    __description__ = "Skim list definitions for neutral B to two neutral pions."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charmless"

    def load_standard_lists(self, path):
        loadStdPi0ForBToCharmless(path=path)

    def build_lists(self, path):
        Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'
        BsigList = []
        ma.reconstructDecay('B0:Pi0Pi0 -> pi0:charmlessFit pi0:charmlessFit', Bcuts, path=path)
        BsigList.append('B0:Pi0Pi0')
        self.SkimLists = BsigList


@fancy_skim_header
class BtoHadTracks(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\B^0 -> \\pi^+ \\pi^-`
    * :math:`\\B^0 -> \\K^+ \\pi^-`
    * :math:`\\B^0 -> \\K^+ K^-`
    * :math:`\\B^+ -> \\pi^+ \\pi^+ \\pi^-`
    * :math:`\\B^+ -> K_{\\rm S}^0 \\pi^+`
    * :math:`\\B^+ -> K_{\\rm S}^0 K^+`
    * :math:`\\B^+ -> K^+ \\pi^+ \\pi^-`
    * :math:`\\B^+ -> K^+ K^+ \\pi^-`
    * :math:`\\B^+ -> K^+ K^- \\pi^+`
    * :math:`\\B^+ -> K^+ K^+ K^-`
    * :math:`\\B^0 -> \\pi^+ \\pi^- \\pi^+ \\pi^-`
    * :math:`\\B^0 -> K^+ \\pi^- \\pi^+ \\pi^-`
    * :math:`\\B^0 -> K^+ K^- \\pi^+ \\pi^-`
    * :math:`\\B^0 -> K^+ \\pi^- \\K^+ \\pi^-`
    * :math:`\\B^0 -> K^+ K^- K^+ \\pi^-`
    * :math:`\\B^0 -> K^+ K^- K^+ K^-`
    * :math:`\\B^0 -> K_{\\rm S}^0 \\pi^+ \\pi^-`
    * :math:`\\B^0 -> K_{\\rm S}^0 K^+ \\pi^-`
    * :math:`\\B^0 -> K_{\\rm S}^0 K^+ K^-`
    * :math:`\\B^0 -> K_{\\rm S}^0 K_{\\rm S}^0`
    * :math:`\\B^0 -> K^{*0} K^- K^+`
    * :math:`\\B^0 -> K^{*0} \\pi^- \\pi^+`
    * :math:`\\B^0 -> K^{*0} K^+ \\pi^-`
    * :math:`\\B^0 -> \\rho^0 \\rho^0`
    * :math:`\\B^+ -> K^{*+} K^- K^+`
    * :math:`\\B^+ -> K^{*+} \\pi^- \\pi^+`
    * :math:`\\B^+ -> K^{*+} K^+ \\pi^-`

    Cuts applied:

    * ``nTracks >= 2 (event level)``
    * ``5.20 < Mbc < 5.29``
    * ``abs(deltaE) < 0.3``

    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"]
    __description__ = "Skim list definitions for all B to charmless modes with tracks only in the final state."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charmless"

    def load_standard_lists(self, path):
        loadStdVeryLooseTracks('K', path=path)
        loadStdVeryLooseTracks('pi', path=path)
        stdKshorts(path=path)
        loadStdVeryLooseKstar0(path=path)
        loadStdVeryLooseRho0(path=path)
        loadStdVeryLooseKstarPlus(path=path)

    def build_lists(self, path):
        Bcuts = "5.2 < Mbc < 5.29 and abs(deltaE) < 0.3"

        channels = {'B0:Charmless_b2pipi': 'pi+:SkimVeryLoose pi-:SkimVeryLoose',  # 1
                    'B0:Charmless_b2Kpi': 'K+:SkimVeryLoose pi-:SkimVeryLoose',  # 2
                    'B0:Charmless_b2KK': 'K+:SkimVeryLoose K-:SkimVeryLoose',  # 3
                    'B+:Charmless_b2pipipi': 'pi+:SkimVeryLoose pi+:SkimVeryLoose pi-:SkimVeryLoose',  # 4
                    'B+:Charmless_b2Kspi': 'K_S0:merged pi+:SkimVeryLoose',  # 5
                    'B+:Charmless_b2KsK': 'K_S0:merged K+:SkimVeryLoose',  # 6
                    'B+:Charmless_b2Kpipi': 'K+:SkimVeryLoose pi+:SkimVeryLoose pi-:SkimVeryLoose ',  # 7
                    'B+:Charmless_b2KKpi_0': 'K+:SkimVeryLoose K+:SkimVeryLoose  pi-:SkimVeryLoose',  # 8
                    'B+:Charmless_b2KKpi_1': 'K+:SkimVeryLoose K-:SkimVeryLoose  pi+:SkimVeryLoose',  # 9
                    'B+:Charmless_b2KKK': 'K+:SkimVeryLoose K+:SkimVeryLoose K-:SkimVeryLoose',  # 10
                    'B0:Charmless_b2pipipipi': 'pi+:SkimVeryLoose pi-:SkimVeryLoose pi+:SkimVeryLoose pi-:SkimVeryLoose',  # 11
                    'B0:Charmless_b2Kpipipi': 'K+:SkimVeryLoose pi-:SkimVeryLoose pi+:SkimVeryLoose pi-:SkimVeryLoose',  # 12
                    'B0:Charmless_b2KKpipi': 'K+:SkimVeryLoose K-:SkimVeryLoose pi+:SkimVeryLoose pi-:SkimVeryLoose',  # 13
                    'B0:Charmless_b2KpiKpi': 'K+:SkimVeryLoose pi-:SkimVeryLoose K+:SkimVeryLoose pi-:SkimVeryLoose',  # 14
                    'B0:Charmless_b2KKKpi': 'K+:SkimVeryLoose K-:SkimVeryLoose K+:SkimVeryLoose pi-:SkimVeryLoose',  # 15
                    'B0:Charmless_b2KKKK': 'K+:SkimVeryLoose K-:SkimVeryLoose K+:SkimVeryLoose K-:SkimVeryLoose',  # 16
                    'B0:Charmless_b2Kspipi': 'K_S0:merged pi+:SkimVeryLoose pi-:SkimVeryLoose',  # 17
                    'B0:Charmless_b2KsKpi': 'K_S0:merged K+:SkimVeryLoose pi-:SkimVeryLoose',  # 18
                    'B0:Charmless_b2KsKK': 'K_S0:merged K+:SkimVeryLoose K-:SkimVeryLoose',  # 19
                    'B0:Charmless_b2KsKs': 'K_S0:merged K_S0:merged',  # 20
                    'B0:Charmless_b2K*KK': 'K*0:veryLoose K+:SkimVeryLoose K-:SkimVeryLoose',  # 21
                    'B0:Charmless_b2K*pipi': 'K*0:veryLoose pi+:SkimVeryLoose pi-:SkimVeryLoose',  # 22
                    'B0:Charmless_b2K*Kpi': 'K*0:veryLoose K+:SkimVeryLoose pi-:SkimVeryLoose',  # 23
                    'B0:Charmless_b2rhorho': 'rho0:veryLoose rho0:veryLoose',  # 24
                    'B+:Charmless_b2K*pKK': 'K*+:veryLoose K+:SkimVeryLoose K-:SkimVeryLoose',  # 25
                    'B+:Charmless_b2K*ppipi': 'K*+:veryLoose pi+:SkimVeryLoose pi-:SkimVeryLoose',  # 26
                    'B+:Charmless_b2K*pKpi': 'K*+:veryLoose K+:SkimVeryLoose pi-:SkimVeryLoose',  # 27
                    }

        BsigList = []
        for chID, channel in enumerate(channels.keys()):
            ma.reconstructDecay(decayString=channel + ' -> ' + channels[channel],
                                cut=Bcuts, dmID=chID, path=path)
            BsigList.append(channel)
        path = self.skim_event_cuts("nTracks >= 2", path=path)
        self.SkimLists = BsigList


@fancy_skim_header
class BtoHad1Pi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\B^+ -> \\pi^+ \\pi^0`
    * :math:`\\B^+ -> K^+ \\pi^0`
    * :math:`\\B^0 -> \\pi^+ \\pi^- \\pi^0`
    * :math:`\\B^0 -> K^+ \\pi^- \\pi^0`
    * :math:`\\B^0 -> K^- \\pi^+ \\pi^0`
    * :math:`\\B^0 -> K^+ K^- \\pi^0`
    * :math:`\\B^0 -> K_{\\rm S}^0 \\pi^0`
    * :math:`\\B^+ -> K_{\\rm S}^{0} pi^+ \\pi^0`
    * :math:`\\B^+ -> K_{\\rm S}^{0} K^+ \\pi^0`
    * :math:`\\B^+ -> rho+ rho0`

    Cuts applied:

    * ``5.20 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``

    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"]
    __description__ = "Skim list for B to charmless modes including 1 pi0 in the final state."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charmless"

    def load_standard_lists(self, path):
        loadStdVeryLooseTracks('K', path=path)
        loadStdVeryLooseTracks('pi', path=path)
        loadStdPi0ForBToCharmless(path=path)
        stdKshorts(path=path)
        loadStdVeryLooseRho0(path=path)
        loadStdVeryLooseRhoPlus(path=path)

    def build_lists(self, path):
        Bcuts = '5.20 < Mbc < 5.29 and abs(deltaE) < 0.5'
        BsigList = []
        channels = {
                    'B+:Charmless_b2pipi0': 'pi+:SkimVeryLoose pi0:charmlessFit',  # 1
                    'B+:Charmless_b2Kpi0': 'K+:SkimVeryLoose pi0:charmlessFit',  # 2
                    'B0:Charmless_b2pipipi0': 'pi+:SkimVeryLoose pi-:SkimVeryLoose pi0:charmlessFit',  # 1
                    'B0:Charmless_b2Kpipi0_0': 'K+:SkimVeryLoose pi-:SkimVeryLoose pi0:charmlessFit',  # 2
                    'B0:Charmless_b2KKpi0': 'K+:SkimVeryLoose K-:SkimVeryLoose pi0:charmlessFit',  # 4
                    'B0:Charmless_b2Kspi0': 'K_S0:merged pi0:charmlessFit',  # 5
                    'B+:Charmless_b2Kspipi0': 'K_S0:merged pi+:SkimVeryLoose pi0:charmlessFit',  # 6
                    'B+:Charmless_b2KsKpi0': 'K_S0:merged K+:SkimVeryLoose pi0:charmlessFit',  # 7
                    'B+:Charmless_b2rhorho0': 'rho+:veryLoose rho0:veryLoose',  # 8
                    }

        for chID, channel in enumerate(channels.keys()):
            ma.reconstructDecay(decayString=channel + ' -> ' + channels[channel],
                                cut=Bcuts, dmID=chID, path=path)
            BsigList.append(channel)
        self.SkimLists = BsigList


@fancy_skim_header
class BtoHad3Tracks1Pi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\B^+ -> K^{*+} K^+ K^-`
    * :math:`\\B^+ -> K^{*+} K^+ \\pi^-`
    * :math:`\\B^+ -> K^{*+} \\pi^+ \\pi^-`

    Cuts applied:

    * ``5.20 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``

    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"]
    __description__ = "Skim list for all charged B to charmless modes with 3 tracks and 1 Pi0."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charmless"

    def load_standard_lists(self, path):
        loadStdVeryLooseTracks('K', path=path)
        loadStdVeryLooseTracks('pi', path=path)
        loadStdPi0ForBToCharmless(path=path)
        loadStdVeryLooseKstarPlusPi0(path=path)

    def build_lists(self, path):
        Bcuts = '5.20 < Mbc < 5.29 and abs(deltaE) < 0.5'
        BsigList = []

        channels = {
                    'B+:Charmless_b2K*KK_2': 'K*+:veryLoosePi0 K-:SkimVeryLoose K+:SkimVeryLoose',  # 1
                    'B+:Charmless_b2K*pipi_2': 'K*+:veryLoosePi0 pi-:SkimVeryLoose pi+:SkimVeryLoose',  # 2
                    'B+:Charmless_b2K*Kpi_2': 'K*+:veryLoosePi0 K+:SkimVeryLoose pi-:SkimVeryLoose',  # 3
                    }
        for chID, channel in enumerate(channels.keys()):
            ma.reconstructDecay(decayString=channel + ' -> ' + channels[channel],
                                cut=Bcuts, dmID=chID, path=path)
            ma.rankByHighest(particleList=channel, variable="cos(mdstIndex)", numBest=3,
                             outputVariable="cosMdstIndex_rank", path=path)
            BsigList.append(channel)
        path = self.skim_event_cuts("nTracks >= 2", path=path)
        self.SkimLists = BsigList

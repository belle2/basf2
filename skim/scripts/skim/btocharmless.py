#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Skim list building functions for :math:`B\\to X_u +h` analyses
"""

__authors__ = [
    "Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"
]

import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import (
    loadStdAllRhoPlus,
    loadStdPi0ForBToHadrons)
from skim.standardlists.charmless import (
    loadStdVeryLooseTracks,
    loadStdPi0ForBToCharmless,
    loadStdVeryLooseKstar0,
    loadStdVeryLooseRho0,
    loadStdVeryLooseKstarPlus,
    loadStdVeryLooseKstarPlusPi0,
    loadStdVeryLooseRhoPlus)
from skimExpertFunctions import BaseSkim, fancy_skim_header


__liaison__ = "Benedikt Wach <benedikt.wach@desy.de>"


@fancy_skim_header
class BtoPi0Pi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\B^0 -> \\pi^0 \\pi^0`
    Cuts applied:

    * ``5.24 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``

    Note:
    19120100
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
class BtoHad2Tracks(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\B^0 -> \\pi^+ \\pi^-`
    * :math:`\\B^0 -> \\K^+ \\pi^-`
    * :math:`\\B^0 -> \\K^+ K^-`

    Cuts applied:

    * ``nTracks >= 2 (event level)``
    * ``5.20 < Mbc < 5.29``
    * ``abs(deltaE) < 0.3``

    Note:
    19130100
    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"]
    __description__ = "Skim list definitions for all neutral B to charmless modes  with 2 tracks."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charmless"

    def load_standard_lists(self, path):
        loadStdVeryLooseTracks('K', path=path)
        loadStdVeryLooseTracks('pi', path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"

        channels = {'B0:Charmless_b2pipi': 'pi+:veryLoose pi-:veryLoose',  # 1
                    'B0:Charmless_b2Kpi': 'K+:veryLoose pi-:veryLoose',  # 2
                    'B0:Charmless_b2KK': 'K+:veryLoose K-:veryLoose',  # 3
                    }

        BsigList = []
        for chID, channel in enumerate(channels.keys()):
            ma.reconstructDecay(decayString=channel + ' -> ' + channels[channel],
                                cut=Bcuts, dmID=chID, path=path)
            BsigList.append(channel)
        path = self.skim_event_cuts("nTracks >= 2", path=path)
        self.SkimLists = BsigList

    def validation_histograms(self, path):
        ma.cutAndCopyLists('B0:validation', self.SkimLists, '', path=path)
        # the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
        ma.variablesToHistogram(
            filename='BtoHad2Tracks_Validation.root',
            decayString='B0:validation',
            variables=[
                ('Mbc', 100, 5.2, 5.3),
                ('deltaE', 100, -0.3, 0.3)],
            variables_2d=[
                ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.3, 0.3)],
            path=path)


@fancy_skim_header
class BtoHad3Tracks(BaseSkim):
    """

    Reconstructed decay modes:

    * :math:`\\B^+ -> \\pi^+ \\pi^+ \\pi^-`
    * :math:`\\B^+ -> K_{\\rm S}^0 \\pi^+`
    * :math:`\\B^+ -> K_{\\rm S}^0 K^+`
    * :math:`\\B^+ -> K^+ \\pi^+ \\pi^-`
    * :math:`\\B^+ -> K^+ K^+ \\pi^-`
    * :math:`\\B^+ -> K^+ K^- \\pi^+`
    * :math:`\\B^+ -> K^+ K^+ K^-`

    Cuts applied:

    * ``nTracks >= 3 (event level)``
    * ``5.20 < Mbc < 5.29``
    * ``abs(deltaE) < 0.3``

    Note:
    19130200
    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"]
    __description__ = "Skim list definitions for all neutral B to charmless modes with 3 tracks."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charmless"

    def load_standard_lists(self, path):
        loadStdVeryLooseTracks('K', path=path)
        loadStdVeryLooseTracks('pi', path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        Bcuts = '5.20 < Mbc < 5.29 and abs(deltaE) < 0.3'
        channels = {'B+:Charmless_b2pipipi': 'pi+:veryLoose pi+:veryLoose pi-:veryLoose',  # 1
                    'B+:Charmless_b2Kspi': 'K_S0:merged pi+:veryLoose',  # 2
                    'B+:Charmless_b2KsK': 'K_S0:merged K+:veryLoose',  # 3
                    'B+:Charmless_b2Kpipi': 'K+:veryLoose pi+:veryLoose pi-:veryLoose ',  # 4
                    'B+:Charmless_b2KKpi_0': 'K+:veryLoose K+:veryLoose  pi-:veryLoose',  # 5
                    'B+:Charmless_b2KKpi_1': 'K+:veryLoose K-:veryLoose  pi+:veryLoose',  # 6
                    'B+:Charmless_b2KKK': 'K+:veryLoose K+:veryLoose K-:veryLoose',  # 7
                    }

        BsigList = []
        for chID, channel in enumerate(channels.keys()):
            ma.reconstructDecay(decayString=channel + ' -> ' + channels[channel],
                                cut=Bcuts, dmID=chID, path=path)
            BsigList.append(channel)
        path = self.skim_event_cuts("nTracks >= 3", path=path)
        self.SkimLists = BsigList


@fancy_skim_header
class BtoHad4Tracks(BaseSkim):
    """

    Reconstructed decay modes:

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

    Cuts applied:

    * ``5.20 < Mbc < 5.29``
    * ``abs(deltaE) < 0.3``

    Note:
    19130300
    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"]
    __description__ = "Skim list definitions for all neutral B to charmless modes with 4 tracks."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charmless"

    def load_standard_lists(self, path):
        loadStdVeryLooseTracks('K', path=path)
        loadStdVeryLooseTracks('pi', path=path)
        stdKshorts(path=path)
        loadStdVeryLooseKstar0(path=path)
        loadStdVeryLooseRho0(path=path)

    def build_lists(self, path):
        Bcuts = '5.20 < Mbc < 5.29 and abs(deltaE) < 0.3'
        channels = {'B0:Charmless_b2pipipipi': 'pi+:veryLoose pi-:veryLoose pi+:veryLoose pi-:veryLoose',  # 1
                    'B0:Charmless_b2Kpipipi': 'K+:veryLoose pi-:veryLoose pi+:veryLoose pi-:veryLoose',  # 2
                    'B0:Charmless_b2KKpipi': 'K+:veryLoose K-:veryLoose pi+:veryLoose pi-:veryLoose',  # 3
                    'B0:Charmless_b2KpiKpi': 'K+:veryLoose pi-:veryLoose K+:veryLoose pi-:veryLoose',  # 4
                    'B0:Charmless_b2KKKpi': 'K+:veryLoose K-:veryLoose K+:veryLoose pi-:veryLoose',  # 5
                    'B0:Charmless_b2KKKK': 'K+:veryLoose K-:veryLoose K+:veryLoose K-:veryLoose',  # 6
                    'B0:Charmless_b2Kspipi': 'K_S0:merged pi+:veryLoose pi-:veryLoose',  # 7
                    'B0:Charmless_b2KsKpi': 'K_S0:merged K+:veryLoose pi-:veryLoose',  # 8
                    'B0:Charmless_b2KsKK': 'K_S0:merged K+:veryLoose K-:veryLoose',  # 9
                    'B0:Charmless_b2KsKs': 'K_S0:merged K_S0:merged',  # 10
                    'B0:Charmless_b2K*KK': 'K*0:veryLoose K+:veryLoose K-:veryLoose',  # 11
                    'B0:Charmless_b2K*pipi': 'K*0:veryLoose pi+:veryLoose pi-:veryLoose',  # 12
                    'B0:Charmless_b2K*Kpi': 'K*0:veryLoose K+:veryLoose pi-:veryLoose',  # 13
                    'B0:Charmless_b2rhorho': 'rho0:veryLoose rho0:veryLoose'  # 14
                    }
        BsigList = []
        for chID, channel in enumerate(channels.keys()):
            ma.reconstructDecay(decayString=channel + ' -> ' + channels[channel],
                                cut=Bcuts, dmID=chID, path=path)
            BsigList.append(channel)
        path = self.skim_event_cuts("nTracks >= 4", path=path)
        self.SkimLists = BsigList


@fancy_skim_header
class BtoHad5Tracks(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\B^+ -> K^{*+} K^- K^+`
    * :math:`\\B^+ -> K^{*+} \\pi^- \\pi^+`
    * :math:`\\B^+ -> K^{*+} K^+ \\pi^-`

    Cuts applied:

    * ``5.20 < Mbc < 5.29``
    * ``abs(deltaE) < 0.3``

    Note:
    19130400
    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"]
    __description__ = "Skim list for B+ to charmless modes with 5 tracks and including one intermediate state K*+."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charmless"

    def load_standard_lists(self, path):
        loadStdVeryLooseTracks('K', path=path)
        loadStdVeryLooseTracks('pi', path=path)
        stdKshorts(path=path)
        loadStdVeryLooseKstarPlus(path=path)

    def build_lists(self, path):
        Bcuts = '5.20 < Mbc < 5.29 and abs(deltaE) < 0.3'
        BsigList = []
        channels = {
                    'B+:Charmless_b2K*KK': 'K*+:veryLoose K+:veryLoose K-:veryLoose',  # 1
                    'B+:Charmless_b2K*pipi': 'K*+:veryLoose pi+:veryLoose pi-:veryLoose',  # 2
                    'B+:Charmless_b2K*Kpi': 'K*+:veryLoose K+:veryLoose pi-:veryLoose',  # 3
                    }

        for chID, channel in enumerate(channels.keys()):
            ma.reconstructDecay(decayString=channel + ' -> ' + channels[channel],
                                cut=Bcuts, dmID=chID, path=path)
        BsigList.append(channel)
        path = self.skim_event_cuts("nTracks >= 5", path=path)
        self.SkimLists = BsigList


@fancy_skim_header
class BtoHad1Track1Pi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\B^+ -> \\pi^+ \\pi^0`
    * :math:`\\B^+ -> K^+ \\pi^0`

    Cuts applied:

    * ``5.20 < Mbc < 5.29``
    * ``abs(deltaE) < 0.3``

    Note:
    19130500
    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"]
    __description__ = "Skim list for all B+ to charmless modes with 1 track and 1 pi0."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    def load_standard_lists(self, path):
        loadStdVeryLooseTracks('K', path=path)
        loadStdVeryLooseTracks('pi', path=path)
        loadStdPi0ForBToCharmless(path=path)

    def build_lists(self, path):
        Bcuts = '5.20 < Mbc < 5.29 and abs(deltaE) < 0.3'
        BsigList = []
        channels = {
                    'B+:Charmless_b2pipi0': 'pi+:veryLoose pi0:charmlessFit',  # 1
                    'B+:Charmless_b2Kpi0': 'K+:veryLoose pi0:charmlessFit',  # 2
                    }

        for chID, channel in enumerate(channels.keys()):
            ma.reconstructDecay(decayString=channel + ' -> ' + channels[channel],
                                cut=Bcuts, dmID=chID, path=path)
            BsigList.append(channel)

        self.SkimLists = BsigList


@fancy_skim_header
class BtoHad2Tracks1Pi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\B^0 -> \\pi^+ \\pi^- \\pi^0`
    * :math:`\\B^0 -> K^+ \\pi^- \\pi^0`
    * :math:`\\B^0 -> K^- \\pi^+ \\pi^0`
    * :math:`\\B^0 -> K^+ K^- \\pi^0`
    * :math:`\\B^0 -> K_{\\rm S}^0 \\pi^0`

    Cuts applied:

    * ``5.20 < Mbc < 5.29``
    * ``abs(deltaE) < 0.3``

    Note:
    19130600
    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"]
    __description__ = "Skim list for all neutral B to charmless modes with 2 tracks and 1 pi0."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    def load_standard_lists(self, path):
        loadStdPi0ForBToCharmless(path=path)
        stdKshorts(path=path)
        loadStdVeryLooseTracks('K', path=path)
        loadStdVeryLooseTracks('pi', path=path)

    def build_lists(self, path):
        Bcuts = '5.20 < Mbc < 5.29 and abs(deltaE) < 0.3'
        BsigList = []
        channels = {
                    'B0:Charmless_b2pipipi0': 'pi+:veryLoose pi-:veryLoose pi0:charmlessFit',  # 1
                    'B0:Charmless_b2Kpipi0_0': 'K+:veryLoose pi-:veryLoose pi0:charmlessFit',  # 2
                    'B0:Charmless_b2Kpipi0_1': 'K-:veryLoose pi+:veryLoose pi0:charmlessFit',  # 3
                    'B0:Charmless_b2KKpi0': 'K+:veryLoose K-:veryLoose pi0:charmlessFit',  # 4
                    'B0:Charmless_b2Kspi0': 'K_S0:merged pi0:charmlessFit',  # 5
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

    * :math:`\\B^+ -> K_{\\rm S}^{0} pi^+ \\pi^0`
    * :math:`\\B^+ -> K_{\\rm S}^{0} K^+ \\pi^0`
    * :math:`\\B^+ -> rho+ rho0`

    Cuts applied:

    * ``5.20 < Mbc < 5.29``
    * ``abs(deltaE) < 0.3``

    Note:
    19130700
    """
    __authors__ = ["Fernando Abudinen", "Riccardo Manfredi", "Sebastiano Raiz", "Benedikt Wach"]
    __description__ = "Skim list for all charged B to charmless modes with 3 tracks and 1 Pi0."
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
                    'B+:Charmless_b2Kspipi0': 'K_S0:merged pi+:veryLoose pi0:charmlessFit',  # 1
                    'B+:Charmless_b2KsKpi0': 'K_S0:merged K+:veryLoose pi0:charmlessFit',  # 2
                    'B+:Charmless_b2rhorho0': 'rho+:veryLoose rho0:veryLoose'  # 3
                    }

        for chID, channel in enumerate(channels.keys()):
            ma.reconstructDecay(decayString=channel + ' -> ' + channels[channel],
                                cut=Bcuts, dmID=chID, path=path)
            BsigList.append(channel)

        self.SkimLists = BsigList

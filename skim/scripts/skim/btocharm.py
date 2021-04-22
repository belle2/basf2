#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
List of functions to skim events containing :math:`B\\to X_c + h` decays,
where :math:`X_c` stays for :math:`D^0`, :math:`D^{\\pm}`, :math:`D^{*0}` and :math:`D^{*\\pm}`,
and :math:`h` stays for :math:`\\pi^{\\pm}`, :math:`K^{\\pm}`, :math:`\\rho^{\\pm}` and :math:`a_1^{\\pm}`.
"""

__authors__ = [
    "Fernando Abudinen",
    "Hulya Atmacan",
    "Chiara La Licata",
    "Minakshi Nayak",
    "Niharika Rout"
]

import modularAnalysis as ma
from skim.standardlists.charm import (loadD0_hh_loose, loadD0_Kshh_loose,
                                      loadD0_Kspi0_loose, loadD0_Kspipipi0,
                                      loadKForBtoHadrons, loadPiForBtoHadrons,
                                      loadStdD0_Kpi, loadStdD0_Kpipi0,
                                      loadStdD0_Kpipipi, loadStdDplus_Kpipi,
                                      loadStdDplus_Kspi,
                                      loadStdDstar0_D0pi0_Kpi,
                                      loadStdDstar0_D0pi0_Kpipi0,
                                      loadStdDstar0_D0pi0_Kpipipi,
                                      loadStdDstarPlus_D0pi_Kpi,
                                      loadStdDstarPlus_D0pi_Kpipi0,
                                      loadStdDstarPlus_D0pi_Kpipipi,
                                      loadStdDstarPlus_Dpi0_Kpipi)
from skim.standardlists.lightmesons import (loadStdAllRhoPlus,
                                            loadStdPi0ForBToHadrons)
from skimExpertFunctions import BaseSkim, fancy_skim_header
from stdCharged import stdK, stdPi
from stdPi0s import loadStdSkimPi0, stdPi0s
from stdV0s import stdKshorts

__liaison__ = "Yi Zhang <yi.zhang2@desy.de>"
_VALIDATION_SAMPLE = "mdst14.root"


@fancy_skim_header
class BtoD0h_Kspi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^0) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^0) K^+`,

    Cuts applied:

    * ``1.5 < M_D0 < 2.2``
    * ``5.2 < Mbc``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadD0_Kspi0_loose`, where :math:`D^0`
        channels are defined.
    """
    __authors__ = ["Minakshi Nayak"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)
        stdKshorts(path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadD0_Kspi0_loose(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"

        BsigChannels = ["anti-D0:Kspi0 pi+:all",
                        "anti-D0:Kspi0 K+:all"]
        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:BtoD0h_Kspi0" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B+:BtoD0h_Kspi0" + str(chID))

        self.SkimLists = BsigList

    def validation_histograms(self, path):
        loadStdSkimPi0(path=path)
        stdPi0s(listtype='eff50_May2020Fit', path=path)

        ma.reconstructDecay('D0 -> K_S0:merged pi0:eff50_May2020Fit', '1.84 < M < 1.89', path=path)
        ma.reconstructDecay('B-:ch3 ->D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

        # the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
        ma.variablesToHistogram(
            filename=f'{self}_Validation.root',
            decayString='B-:ch3',
            variables=[
                ('Mbc', 100, 5.2, 5.3),
                ('deltaE', 100, -1, 1),
                ('daughter(0, InvM)', 100, 1.8, 1.9)],  # D0 invariant mass
            variables_2d=[
                ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)], path=path)


@fancy_skim_header
class BtoD0h_Kspipipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^- \\pi^0) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^- \\pi^0) K^+`,

    Cuts applied:

    * ``1.8 < M_D0 < 1.9``
    * ``5.25 < Mbc``
    * ``abs(deltaE) < 0.2``

    Note:
        This skim uses `skim.standardlists.charm.loadD0_Kspipipi0`, where :math:`D^0`
        channels are defined.
    """

    __authors__ = ["Niharika Rout"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)
        stdPi0s("eff40_May2020Fit", path=path)
        stdKshorts(path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadPiForBtoHadrons(path=path)
        loadD0_Kspipipi0(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.25 and abs(deltaE) < 0.2"

        BsigChannels = ["anti-D0:Kspipipi0 pi+:all",
                        "anti-D0:Kspipipi0 K+:all"
                        ]
        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:BtoD0h_Kspipipi0" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B+:BtoD0h_Kspipipi0" + str(chID))

        self.SkimLists = BsigList

    def validation_histograms(self, path):
        stdPi('all', path=path)
        stdK('all', path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        stdPi0s(listtype='eff40_May2020Fit', path=path)

        ma.reconstructDecay('D0 -> K_S0:merged pi-:all pi+:all pi0:eff40_May2020Fit', '1.84 < M < 1.89', path=path)
        ma.reconstructDecay('B-:ch3 ->D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

        # the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
        ma.variablesToHistogram(
            filename=f'{self}_Validation.root',
            decayString='B-:ch3',
            variables=[
                ('Mbc', 100, 5.2, 5.3),
                ('deltaE', 100, -1, 1),
                ('daughter(0, InvM)', 100, 1.8, 1.9)],  # D0 invariant mass
            variables_2d=[
                ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)], path=path)


@fancy_skim_header
class B0toDpi_Kpipi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{-} (D^{0} \\to K^+ \\pi^- \\pi^-) \\pi^+`

    Cuts applied:

    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadStdDplus_Kpipi`, where :math:`D^-`
        channel is defined.
    """

    __authors__ = ["Chiara La Licata"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdDplus_Kpipi(path=path)

    def build_lists(self, path):
        Bcuts = "5.2 < Mbc and abs(deltaE) < 0.3"
        ma.reconstructDecay("B0:Dpi_Kpipi -> D-:Kpipi pi+:GoodTrack", Bcuts, 0, path=path)

        self.SkimLists = ["B0:Dpi_Kpipi"]


@fancy_skim_header
class B0toDpi_Kspi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{-} (\\to K_{\\rm S}^0 \\pi^-) \\pi^+`,

    Cuts applied:

    * ``1.8 < M_D < 1.9``
    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `stdV0s.stdKshorts` and
        `skim.standardlists.charm.loadStdDplus_Kpipi`, where :math:`D^0` channels are defined.
    """

    __authors__ = ["Fernando Abudinen", "Chiara La Licata"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair

    def load_standard_lists(self, path):
        stdKshorts(path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadPiForBtoHadrons(path=path)
        loadStdDplus_Kspi(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"
        ma.reconstructDecay("B0:B0toDpi_Kspi -> D-:Kspi pi+:GoodTrack", Bcuts, 1, path=path)

        self.SkimLists = ["B0:B0toDpi_Kspi"]


@fancy_skim_header
class B0toDstarPi_D0pi_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{*-} (D^{0} \\to K^+ \\pi^-) \\pi^+`

    Cuts applied:

    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadStdDstarPlus_D0pi_Kpi`, where the
        :math:`D^{*-}` channel is defined.
    """

    __authors__ = ["Chiara La Licata"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair

    def load_standard_lists(self, path):
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpi(path=path)
        loadStdDstarPlus_D0pi_Kpi(path=path)

    def build_lists(self, path):
        Bcuts = "5.2 < Mbc and abs(deltaE) < 0.3"
        ma.reconstructDecay("B0:Dstarpi_Kpi -> D*-:D0_Kpi pi+:GoodTrack", Bcuts, 0, path=path)

        self.SkimLists = ["B0:Dstarpi_Kpi"]


@fancy_skim_header
class B0toDstarPi_D0pi_Kpipipi_Kpipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to \\overline{D}^{*-} (\\to \\overline{D}^{0}
      (\\to K^+ \\pi^- \\pi^- \\pi^+, K^+\\pi^-\\pi^0) \\pi^-) \\pi^+`

    Cuts applied:

    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``
    """

    __authors__ = ["Chiara La Licata"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair

    def load_standard_lists(self, path):
        loadStdPi0ForBToHadrons(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpipi0(path=path)
        loadStdD0_Kpipipi(path=path)
        loadStdDstarPlus_D0pi_Kpipi0(path=path)
        loadStdDstarPlus_D0pi_Kpipipi(path=path)

    def build_lists(self, path):
        Bcuts = "5.2 < Mbc and abs(deltaE) < 0.3"
        BsigChannels = [
            "D*-:D0_Kpipipi pi+:GoodTrack",
            "D*-:D0_Kpipi0 pi+:GoodTrack"
        ]

        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B0:Dstarpi_Kpipipi_Kpipi0" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B0:Dstarpi_Kpipipi_Kpipi0" + str(chID))

        self.SkimLists = BsigList


@fancy_skim_header
class B0toDrho_Kpipi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{-} (\\to K^+ \\pi^- \\pi^-) \\rho^+`,

    Cuts applied:

    * ``1.8 < M_D < 1.9``
    * ``0.47 < M_rho < 1.07``
    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadStdDplus_Kpipi`, where :math:`D^0` channels
        are defined, and `skim.standardlists.lightmesons.loadStdAllRhoPlus`.
    """

    __authors__ = ["Fernando Abudinen"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdDplus_Kpipi(path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadStdAllRhoPlus(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"
        ma.reconstructDecay("B0:B0toDrho_Kpipi -> D-:Kpipi rho+:all", Bcuts, 1, path=path)

        self.SkimLists = ["B0:B0toDrho_Kpipi"]


@fancy_skim_header
class B0toDrho_Kspi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{-} (\\to K_{\\rm S}^0 \\pi^-) \\rho^+`,

    Cuts applied:

    * ``1.8 < M_D < 1.9``
    * ``0.47 < M_rho < 1.07``
    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `stdV0s.stdKshorts` and
        `skim.standardlists.charm.loadStdDplus_Kspi`, where :math:`D^0` channels are defined, and
        `skim.standardlists.lightmesons.loadStdAllRhoPlus`.
    """

    __authors__ = ["Fernando Abudinen"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        stdKshorts(path=path)
        loadPiForBtoHadrons(path=path)
        loadStdDplus_Kspi(path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadStdAllRhoPlus(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"

        ma.reconstructDecay("B0:B0toDrho_Kspi -> D-:Kspi rho+:all", Bcuts, 1, path=path)

        self.SkimLists = ["B0:B0toDrho_Kspi"]


@fancy_skim_header
class B0toDstarRho_D0pi_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{*-} (\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\pi^-) \\rho^+`,

    Cuts applied:

    * ``1.7 < M_D < 2.0``
    * ``0.47 < M_rho < 1.07``
    * ``DM_Dstar_D < 0.16``
    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpi` and
        `skim.standardlists.charm.loadStdDstarPlus_D0pi_Kpi` where :math:`D^0` channels are
        defined, and `skim.standardlists.lightmesons.loadStdAllRhoPlus`.
    """

    __authors__ = ["Fernando Abudinen"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadStdAllRhoPlus(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpi(path=path)
        loadStdDstarPlus_D0pi_Kpi(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"
        ma.reconstructDecay("B0:BtoDstarRho_D0pi_Kpi -> D*-:D0_Kpi rho+:all", Bcuts, 1, path=path)

        self.SkimLists = ["B0:BtoDstarRho_D0pi_Kpi"]


@fancy_skim_header
class B0toDstarRho_D0pi_Kpipipi_Kpipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{*-} (\\to \\overline{D}^{0}
      (\\to K^+2\\pi^-\\pi^+, K^+\\pi^-\\pi^0)\\pi^-) \\rho^+`,

    Cuts applied:

    * ``1.7 < M_D < 2.0``
    * ``0.47 < M_rho < 1.07``
    * ``DM_Dstar_D < 0.16``
    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpipi0`,
        `skim.standardlists.charm.loadStdD0_Kpipipi`,
        `skim.standardlists.charm.loadStdDstarPlus_D0pi_Kpipi0` and
        `skim.standardlists.charm.loadStdDstarPlus_D0pi_Kpipipi` where :math:`D^0`
        channels are defined, and `skim.standardlists.lightmesons.loadStdAllRhoPlus`.
    """

    __authors__ = ["Fernando Abudinen"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadStdAllRhoPlus(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpipi0(path=path)
        loadStdD0_Kpipipi(path=path)
        loadStdDstarPlus_D0pi_Kpipi0(path=path)
        loadStdDstarPlus_D0pi_Kpipipi(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"

        BsigChannels = [
            "D*-:D0_Kpipipi rho+:all",
            "D*-:D0_Kpipi0 rho+:all",
        ]
        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:B0toDstarRho" + str(chID) + " -> " + channel, Bcuts, chID, path=path, allowChargeViolation=True)
            BsigList.append("B+:B0toDstarRho" + str(chID))

        self.SkimLists = BsigList


@fancy_skim_header
class BtoD0h_hh(BaseSkim):
    """
    Skim list definitions for all charged B to charm 2 body decays.

    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^- \\pi^+) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to \\pi^+ \\pi^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ K^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) K^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^- \\pi^+) K^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to \\pi^+ \\pi^-) K^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ K^-) K^+`,

    Cuts applied:

    * ``1.5 < M_D0 < 2.2``
    * ``5.2 < Mbc``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadD0_hh_loose`, where :math:`D^0`
        channels are defined.
    """

    __authors__ = ["Hulya Atmacan"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadD0_hh_loose(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"

        BsigChannels = [
            "anti-D0:hh pi+:GoodTrack",
            "anti-D0:hh K+:GoodTrack"
        ]
        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:BtoD0h_hh" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B+:BtoD0h_hh" + str(chID))

        self.SkimLists = BsigList

    def validation_histograms(self, path):
        stdPi('all', path=path)
        stdK('all', path=path)

        ma.reconstructDecay('D0 -> K-:all pi+:all', '1.84 < M < 1.89', path=path)
        ma.reconstructDecay('B-:ch3 ->D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

        # the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
        ma.variablesToHistogram(
            filename=f'{self}_Validation.root',
            decayString='B-:ch3',
            variables=[
                ('Mbc', 100, 5.2, 5.3),
                ('deltaE', 100, -1, 1),
                ('daughter(0, InvM)', 100, 1.8, 1.9)],  # D0 invariant mass
            variables_2d=[
                ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)], path=path)


@fancy_skim_header
class BtoD0h_Kpi(BaseSkim):
    """
    Skim list definitions for all charged B to charm 3 body decays.

    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) K^+`,

    Cuts applied:

    * ``1.7 < M_D0 < 2.0``
    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.5``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpi`, where :math:`D^0`
        channels are defined.
    """

    __authors__ = ["Niharika Rout"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpi(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.5"

        BsigChannels = ["anti-D0:Kpi pi+:GoodTrack",
                        "anti-D0:Kpi K+:GoodTrack"
                        ]
        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:BtoD0h_Kpi" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B+:BtoD0h_Kpi" + str(chID))

        self.SkimLists = BsigList


@fancy_skim_header
class BtoD0h_Kpipipi_Kpipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ pi^- pi^- pi^+, \\to K^+ pi^- pi^0) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ pi^- pi^- pi^+, \\to K^+ pi^- pi^0) K^+`,
    * :math:`B^{+}\\to \\overline{D}^{*0} (\\to \\overline{D}^{0} (\\to K^+2\\pi^-\\pi^+, K^+\\pi^-\\pi^0)
      \\pi^0) \\pi^+`
    * :math:`B^{+}\\to \\overline{D}^{*0} (\\to \\overline{D}^{0} (\\to K^+2\\pi^-\\pi^+, K^+\\pi^-\\pi^0)
      \\pi^0) \\K^+`

    Cuts applied:

    * ``1.7 < M_D0 < 2.0``
    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpipipi` and
        `skim.standardlists.charm.loadStdD0_Kpipi0`, where :math:`D^0` channels are
        defined.
    """

    __authors__ = ["Chiara La Licata"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadStdPi0ForBToHadrons(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpipi0(path=path)
        loadStdD0_Kpipipi(path=path)
        loadStdDstar0_D0pi0_Kpipipi(path=path)
        loadStdDstar0_D0pi0_Kpipi0(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"

        BsigChannels = ["anti-D0:Kpipipi pi+:GoodTrack",
                        "anti-D0:Kpipipi K+:GoodTrack",
                        "anti-D0:Kpipi0 pi+:GoodTrack",
                        "anti-D0:Kpipi0 K+:GoodTrack",
                        "anti-D*0:D0_Kpipipi pi+:GoodTrack",
                        "anti-D*0:D0_Kpipipi K+:GoodTrack",
                        "anti-D*0:D0_Kpipi0 pi+:GoodTrack",
                        "anti-D*0:D0_Kpipi0 K+:GoodTrack"
                        ]
        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:BtoD0h_Khh_Khpi0" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B+:BtoD0h_Khh_Khpi0" + str(chID))

        ma.copyLists(outputListName="B+:BtoD0h_merged", inputListNames=BsigList, path=path)

        # Select only three random candidates
        ma.rankByHighest(particleList="B+:BtoD0h_merged", variable="cos(mdstIndex)", numBest=3,
                         outputVariable="cosMdstIndex_rank", path=path)

        self.SkimLists = ["B+:BtoD0h_merged"]


@fancy_skim_header
class BtoD0h_Kshh(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^+ \\pi^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^- \\pi^+) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^+ K^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^+ \\pi^-) K^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^- \\pi^+) K^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^-) K^+`,
    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^+ K^-) K^+`,

    Cuts applied:

    * ``1.5 < M_D0 < 2.2``
    * ``5.2 < Mbc``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadD0_Kshh_loose`, where :math:`D^0`
        channels are defined.
    """

    __authors__ = ["Minakshi Nayak"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdKshorts(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadD0_Kshh_loose(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"

        BsigChannels = ["anti-D0:Kshh pi+:GoodTrack",
                        "anti-D0:Kshh K+:GoodTrack"
                        ]
        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:BtoD0h_Kshh" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B+:BtoD0h_Kshh" + str(chID))

        self.SkimLists = BsigList

    def validation_histograms(self, path):
        stdPi('all', path=path)
        stdK('all', path=path)

        ma.reconstructDecay('D0 -> K_S0:merged pi+:all pi-:all', '1.84 < M < 1.89', path=path)
        ma.reconstructDecay('B-:ch3 ->D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

        # the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.

        ma.variablesToHistogram(
            filename=f'{self}_Validation.root',
            decayString='B-:ch3',
            variables=[
                ('Mbc', 100, 5.2, 5.3),
                ('deltaE', 100, -1, 1),
                ('daughter(0, InvM)', 100, 1.8, 1.9)],  # D0 invariant mass
            variables_2d=[
                ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)], path=path)


@fancy_skim_header
class BtoD0rho_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\rho^+`,
    * :math:`B^{+}\\to \\overline{D}^{*0} (\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\pi^0) \\rho^+`,

    Cuts applied:

    * ``1.7 < M_D0 < 2.0``
    * ``0.47 < M_rho < 1.07``
    * ``DM_Dstar_D < 0.16``
    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpi` and
        `skim.standardlists.charm.loadStdDstar0_D0pi0_Kpi`, where :math:`D^0` channels
        are defined, and `skim.standardlists.lightmesons.loadStdAllRhoPlus`.
    """

    __authors__ = ["Fernando Abudinen"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadStdAllRhoPlus(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpi(path=path)
        loadStdDstar0_D0pi0_Kpi(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3"

        BsigChannels = ["anti-D0:Kpi rho+:all",
                        "anti-D*0:D0_Kpi rho+:all"]
        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:BtoD0rho_Kpi" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B+:BtoD0rho_Kpi" + str(chID))

        self.SkimLists = BsigList


@fancy_skim_header
class BtoD0rho_Kpipipi_Kpipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+2\\pi^-\\pi^+, K^+\\pi^-\\pi^0) \\rho^+`
    * :math:`B^{+}\\to \\overline{D}^{*0} (\\to \\overline{D}^{0} (\\to K^+2 \\pi^-
      \\pi^+, K^+\\pi^-\\pi^0) \\pi^0) \\rho^+`

    Cuts applied:

    * ``1.7 < M_D0 < 2.0``
    * ``DM_Dstar_D < 0.16``
    * ``0.6 < M_rho < 0.9``
    * ``cosHel_rho < 0.90``
    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpipi0`,
        `skim.standardlists.charm.loadStdD0_Kpipipi`,
        `skim.standardlists.charm.loadStdDstar0_D0pi0_Kpipi0`, and
        `skim.standardlists.charm.loadStdDstar0_D0pi0_Kpipipi`, where :math:`D^0`
        channels are defined, and `skim.standardlists.lightmesons.loadStdAllRhoPlus`.

    Warning:
       This skim saves only three randomly-chosen :math:`B^{+}` candidates in a
       ``B+:BtoD0rho_merged`` list, since the candidate multiplicity of this skim is
       very high.
    """
    __authors__ = ["Fernando Abudinen"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadStdAllRhoPlus(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpipi0(path=path)
        loadStdD0_Kpipipi(path=path)
        loadStdDstar0_D0pi0_Kpipi0(path=path)
        loadStdDstar0_D0pi0_Kpipipi(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.2 and abs(deltaE) < 0.3 and cosHelicityAngle(1,0) < 0.9 and 0.6 <= daughter(1,M) <= 0.9"

        BsigChannels = [
            "anti-D0:Kpipipi rho+:all",
            "anti-D0:Kpipi0 rho+:all",
            "anti-D*0:D0_Kpipipi rho+:all",
            "anti-D*0:D0_Kpipi0 rho+:all"
        ]
        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:BtoD0rho_merged" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B+:BtoD0rho_merged" + str(chID))

        ma.copyLists(outputListName="B+:BtoD0rho_merged", inputListNames=BsigList, path=path)

        # Select only three random candidates to save them as these channels have high multiplicity.
        ma.rankByHighest(particleList="B+:BtoD0rho_merged", variable="cos(mdstIndex)", numBest=3,
                         outputVariable="cosMdstIndex_rank", path=path)

        self.SkimLists = ["B+:BtoD0rho_merged"]


@fancy_skim_header
class B0toDD_Kpipi_Kspi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{+}(\\to K^- \\pi^+ \\pi^+) D^{-}(\\to K^+ \\pi^- \\pi^-)`
    * :math:`B^{0}\\to D^{+}(\\to K^- \\pi^+ \\pi^+) D^{-}(\\to K_{\\rm S}^0 \\pi^-)`
    * :math:`B^{0}\\to D^{+}(\\to K_{\\rm S}^0 \\pi^-) D^{-}(\\to K_{\\rm S}^0 \\pi^-)`

    Cuts applied:

    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``
    * ``1.8 < M_D < 1.9``

    Note:
        This skim uses `skim.standardlists.charm.loadStdDplus_Kpipi` and
        `skim.standardlists.charm.loadStdDplus_Kspi`, where :math:`D^-`
        channel is defined.
    """

    produce_on_tau_samples = False  # retention is very close to zero on taupair

    __authors__ = ["Chiara La Licata"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdKshorts(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdDplus_Kpipi(path=path)
        loadStdDplus_Kspi(path=path)

    def build_lists(self, path):
        Bcuts = "5.2 < Mbc and abs(deltaE) < 0.3"
        BsigChannels = [
            "D+:Kpipi D-:Kpipi",
            "D+:Kpipi D-:Kspi",
            "D+:Kspi D-:Kspi"
        ]

        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B0:B0toDD" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B0:B0toDD" + str(chID))

        self.SkimLists = BsigList


@fancy_skim_header
class B0toDstarD(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to \\overline{D}^{*-} (\\to \\overline{D}^{0}
      (\\to K^+ \\pi^-, \\to K^+ \\pi^- \\pi^- \\pi^+, K^+ \\pi^- \\pi^0) \\pi^-) \\D^+(\\to K^- \\pi^+ \\pi^+)`
    * :math:`B^{0}\\to \\overline{D}^{*-} (\\to D^{-} \\pi^0) \\D^+(\\to K^- \\pi^+ \\pi^+)`

    Cuts applied:

    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``
    * ``DM_Dstar_D < 0.16``
    * ``1.8 < M_D < 1.9``
    """

    __authors__ = ["Chiara La Licata"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair

    def load_standard_lists(self, path):
        loadStdPi0ForBToHadrons(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpi(path=path)
        loadStdD0_Kpipi0(path=path)
        loadStdD0_Kpipipi(path=path)
        loadStdDplus_Kpipi(path=path)
        loadStdDstarPlus_D0pi_Kpi(path=path)
        loadStdDstarPlus_D0pi_Kpipipi(path=path)
        loadStdDstarPlus_D0pi_Kpipi0(path=path)
        loadStdDstarPlus_Dpi0_Kpipi(path=path)

    def build_lists(self, path):
        Bcuts = "5.2 < Mbc and abs(deltaE) < 0.3"
        BsigChannels = [
            "D*+:D0_Kpi D-:Kpipi",
            "D*+:D0_Kpipipi D-:Kpipi",
            "D*+:D0_Kpipi0 D-:Kpipi",
            "D*+:Dpi0_Kpipi D-:Kpipi"
        ]

        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B0:B0toDstarD" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B0:B0toDstarD" + str(chID))

        self.SkimLists = BsigList

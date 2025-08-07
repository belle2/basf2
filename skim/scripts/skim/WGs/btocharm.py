#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
List of functions to skim events containing :math:`B\\to X_c + h` decays,
where :math:`X_c` stands for :math:`D^0`, :math:`D^{\\pm}`, :math:`D^{*0}` and :math:`D^{*\\pm}`,
and :math:`h` stands for :math:`\\pi^{\\pm}`, :math:`K^{\\pm}`, :math:`\\rho^{\\pm}` and :math:`a_1^{\\pm}`.
"""

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
                                      loadStdDstarPlus_Dpi0_Kpipi,
                                      loadCharmlessD0_Kpipi0,
                                      loadPiSkimHighEff,
                                      loadKSkimHighEff,
                                      loadSlowPi)
from skim.standardlists.lightmesons import (loadStdAllRhoPlus,
                                            loadStdPi0ForBToHadrons,
                                            loadStdSkimHighEffPhi,
                                            loadStdSkimHighEffKstar0,
                                            loadStdPhotonCutEta)
from skim.standardlists.charmless import (loadStdPi0ForBToCharmless)
from skim.standardlists.charmless import (loadStdVeryLooseTracks)
from skim import BaseSkim, fancy_skim_header
from stdCharged import stdK, stdPi
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from stdPhotons import stdPhotons

__liaison__ = "Yi Zhang <yi.zhang2@desy.de>"
_VALIDATION_SAMPLE = "mdst16.root"


@fancy_skim_header
class BtoD0h_Kspi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 \\pi^0) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 \\pi^0) K^+`,

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

        return BsigList

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        stdPi0s(listtype='eff50_May2020Fit', path=path)

        ma.reconstructDecay('D0 -> K_S0:merged pi0:eff50_May2020Fit', '1.84 < M < 1.89', path=path)
        ma.reconstructDecay('B-:ch3 -> D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

        # the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
        create_validation_histograms(
            rootfile=f'{self}_Validation.root',
            particlelist='B-:ch3',
            variables_1d=[
                ('Mbc', 100, 5.2, 5.3, 'Mbc', self.__contact__, 'Beam constrained mass', '', 'Mbc', 'Candidates'),
                ('deltaE', 100, -1, 1, 'deltaE', self.__contact__, 'Energy difference of B', '', 'deltaE', 'Candidates'),
                ('daughter(0, InvM)', 100, 1.8, 1.9, 'D0_InvM', self.__contact__, 'D0 invariant mass', '', 'InvM', 'Candidates')],
            variables_2d=[
                ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7, 'Mbc vs deltaE', self.__contact__,
                 'Beam constrained mass vs energy difference of reconstructed B', 'Mbc', 'DeltaE')],
            path=path)


@fancy_skim_header
class BtoD0h_Kspipipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^- \\pi^0) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^- \\pi^0) K^+`,

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

        return BsigList

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.reconstructDecay('D0 -> K_S0:merged pi-:all pi+:all pi0:eff40_May2020Fit', '1.84 < M < 1.89', path=path)
        ma.reconstructDecay('B-:ch3 -> D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

        # the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
        create_validation_histograms(
            rootfile=f'{self}_Validation.root',
            particlelist='B-:ch3',
            variables_1d=[
                ('Mbc', 100, 5.2, 5.3, 'Mbc', self.__contact__, 'Beam constrained mass', '', 'Mbc', 'Candidates'),
                ('deltaE', 100, -1, 1, 'deltaE', self.__contact__, 'Energy difference of B', '', 'deltaE', 'Candidates'),
                ('daughter(0, InvM)', 100, 1.8, 1.9, 'D0_InvM', self.__contact__, 'D0 invariant mass', '', 'InvM', 'Candidates')],
            variables_2d=[
                ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7, 'Mbc vs deltaE', self.__contact__,
                 'Beam constrained mass vs energy difference of reconstructed B', 'Mbc', 'DeltaE')],
            path=path)


@fancy_skim_header
class BtoDstarpipipi0_D0pi_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to D^{*-} (\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\pi^-) \\pi^+ \\pi^+ \\pi^0`

    Cuts applied:

    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.3``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpi` and
        `skim.standardlists.charm.loadStdDstarPlus_D0pi_Kpi`, where
        :math:`D^{*-}` channel is defined.
    """

    __authors__ = ["Fedja Ceplak Mencin"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadStdPi0ForBToHadrons(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpi(path=path)
        loadStdDstarPlus_D0pi_Kpi(path=path)

    def build_lists(self, path):
        Bcuts = "5.2 < Mbc and abs(deltaE) < 0.3"

        ma.reconstructDecay("B+:Dstarpipipi0_D0pi_Kpi -> D*-:D0_Kpi pi+:GoodTrack pi+:GoodTrack pi0:bth_skim", Bcuts, path=path)

        return ["B+:Dstarpipipi0_D0pi_Kpi"]


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

        return ["B0:Dpi_Kpipi"]


@fancy_skim_header
class B0toDpi_KpipiLooseDelE(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{-} (D^{0} \\to K^+ \\pi^- \\pi^-) \\pi^+`

    Cuts applied:

    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.45``

    Note:
        This skim is the same as B0toDpi_Kpipi, with the only difference of a larger deltaE cut (abs(deltaE) < 0.45).
        As for B0toDpi_Kpipi, this skim uses skim.standardlists.charm.loadStdDplus_Kpipi, where :math:`D^-` channel is defined.
    """

    __authors__ = ["Sourav Dey"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdDplus_Kpipi(path=path)

    def build_lists(self, path):
        Bcuts = "5.2 < Mbc and abs(deltaE) < 0.45"
        ma.reconstructDecay("B0:Dpi_KpipiLooseDelE -> D-:Kpipi pi+:GoodTrack", Bcuts, 0, path=path)

        return ["B0:Dpi_KpipiLooseDelE"]


@fancy_skim_header
class BptoD0etapi_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\bar{D}^{0} (\\to K^+ \\pi^-) \\eta (\\to \\gamma \\gamma) \\pi^+`

    Cuts applied:

    * ``Mbc > 5.25``
    * ``abs(deltaE) < 0.32``
    * ``1.7 < M_D < 2.0``
    * ``0.35 < M_eta < 0.7``
    * ``E_photons > 0.05``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpi`.
    """

    __authors__ = ["Vismaya V S"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPhotons("all", path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpi(path=path)
        loadStdPhotonCutEta(path=path)

    def build_lists(self, path):
        Bcuts = "5.25 < Mbc and abs(deltaE) < 0.32 and 0.35 < daughter(1,M) < 0.7"

        ma.reconstructDecay("B+:BptoD0etapi_Kpi -> anti-D0:Kpi eta:gm pi+:GoodTrack", Bcuts, path=path)

        return ["B+:BptoD0etapi_Kpi"]


@fancy_skim_header
class BptoD0pipi0_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\bar{D}^{0} (\\to K^+ \\pi^-) \\pi^+ \\pi^0`

    Cuts applied:

    * ``Mbc > 5.25``
    * ``abs(deltaE) < 0.32``
    * ``1.7 < M_D < 2.0``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpi`.
    """

    __authors__ = ["Vismaya V S"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadStdPi0ForBToHadrons(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpi(path=path)

    def build_lists(self, path):
        Bcuts = "5.25 < Mbc and abs(deltaE) < 0.32"

        ma.reconstructDecay("B+:BptoD0pipi0_Kpi -> anti-D0:Kpi pi+:GoodTrack pi0:bth_skim", Bcuts, path=path)

        return ["B+:BptoD0pipi0_Kpi"]


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

        return ["B0:B0toDpi_Kspi"]


@fancy_skim_header
class B0toDstaretapi_D0pi_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to \\overline{D}^{*-} (\\to \\bar{D}^{0} (\\to K^+ \\pi^-) \\pi^-) \\eta (\\to \\gamma \\gamma) \\pi^+`

    Cuts applied:

    * ``Mbc > 5.25``
    * ``abs(deltaE) < 0.32``
    * ``1.7 < M_D0 < 2.0``
    * ``(M_D* - M_D0) < 0.16``
    * ``0.35 < M_eta < 0.7``
    * ``Energy_photons > 0.05``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpi` and `skim.standardlists.charm.loadStdDstarPlus_D0pi_Kpi`, where the
        :math:`\\bar{D}^{0}` channel is defined.
    """

    __authors__ = ["Vismaya V S"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPhotons("all", path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpi(path=path)
        loadStdDstarPlus_D0pi_Kpi(path=path)
        loadStdPhotonCutEta(path=path)

    def build_lists(self, path):
        Bcuts = "5.25 < Mbc and abs(deltaE) < 0.32 and 0.35 < daughter(1,M) < 0.7"

        ma.reconstructDecay("B0:B0toDstaretapi_D0pi_Kpi -> D*-:D0_Kpi eta:gm pi+:GoodTrack", Bcuts, path=path)

        return ["B0:B0toDstaretapi_D0pi_Kpi"]


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

        return ["B0:Dstarpi_Kpi"]


@fancy_skim_header
class B0toDstarPi_D0pi_Kpipipi_Kpipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to \\overline{D}{}^{*-} (\\to \\overline{D}{}^{0}
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

        return BsigList


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

        return ["B0:B0toDrho_Kpipi"]


@fancy_skim_header
class B0toDstarpipi0_D0pi_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to \\overline{D}^{*-} (\\to \\bar{D}^{0} (\\to K^+ \\pi^-) \\pi^-) \\pi^+ \\pi^0`

    Cuts applied:

    * ``Mbc > 5.25``
    * ``abs(deltaE) < 0.32``
    * ``1.7 < M_D < 2.0``
    * ``(M_D* - M_D0) < 0.16``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpi` and `skim.standardlists.charm.loadStdDstarPlus_D0pi_Kpi`, where the
        :math:`\\bar{D}^{0}` channel is defined.
    """

    __authors__ = ["Vismaya V S"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadStdPi0ForBToHadrons(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0_Kpi(path=path)
        loadStdDstarPlus_D0pi_Kpi(path=path)

    def build_lists(self, path):
        Bcuts = "5.25 < Mbc and abs(deltaE) < 0.32"

        ma.reconstructDecay("B0:B0toDstarpipi0_D0pi_Kpi -> D*-:D0_Kpi pi+:GoodTrack pi0:bth_skim", Bcuts, path=path)

        return ["B0:B0toDstarpipi0_D0pi_Kpi"]


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

        return ["B0:B0toDrho_Kspi"]


@fancy_skim_header
class B0toDstarRho_D0pi_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{*-} (\\to \\overline{D}{}^{0} (\\to K^+ \\pi^-) \\pi^-) \\rho^+`,

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

        return ["B0:BtoDstarRho_D0pi_Kpi"]


@fancy_skim_header
class B0toDstarRho_D0pi_Kpipipi_Kpipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D^{*-} (\\to \\overline{D}{}^{0}
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

        return BsigList


@fancy_skim_header
class BtoD0h_hh(BaseSkim):
    """
    Skim list definitions for all charged B to charm 2 body decays.

    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^+ \\pi^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^- \\pi^+) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to \\pi^+ \\pi^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^+ K^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^+ \\pi^-) K^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^- \\pi^+) K^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to \\pi^+ \\pi^-) K^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^+ K^-) K^+`,

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

        return BsigList

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.reconstructDecay('D0 -> K-:all pi+:all', '1.84 < M < 1.89', path=path)
        ma.reconstructDecay('B-:ch3 -> D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

        # the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
        create_validation_histograms(
            rootfile=f'{self}_Validation.root',
            particlelist='B-:ch3',
            variables_1d=[
                ('Mbc', 100, 5.2, 5.3, 'Mbc', self.__contact__, 'Beam constrained mass', '', 'Mbc', 'Candidates'),
                ('deltaE', 100, -1, 1, 'deltaE', self.__contact__, 'Energy difference of B', '', 'deltaE', 'Candidates'),
                ('daughter(0, InvM)', 100, 1.8, 1.9, 'D0_InvM', self.__contact__, 'D0 invariant mass', '', 'InvM', 'Candidates')],
            variables_2d=[
                ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7, 'Mbc vs deltaE', self.__contact__,
                 'Beam constrained mass vs energy difference of reconstructed B', 'Mbc', 'DeltaE')],
            path=path)


@fancy_skim_header
class BtoD0h_Kpi(BaseSkim):
    """
    Skim list definitions for all charged B to charm 3 body decays.

    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^+ \\pi^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^+ \\pi^-) K^+`,

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

        return BsigList


@fancy_skim_header
class BtoD0h_Kpipipi_Kpipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^+ \\pi^- \\pi^- \\pi^+, \\to K^+ \\pi^- \\pi^0) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^+ \\pi^- \\pi^- \\pi^+, \\to K^+ \\pi^- \\pi^0) K^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{*0} (\\to \\overline{D}{}^{0} (\\to K^+2\\pi^-\\pi^+, K^+\\pi^-\\pi^0)
      \\pi^0) \\pi^+`
    * :math:`B^{+}\\to \\overline{D}{}^{*0} (\\to \\overline{D}{}^{0} (\\to K^+2\\pi^-\\pi^+, K^+\\pi^-\\pi^0)
      \\pi^0) K^+`

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

        ma.copyLists(outputListName="B+:BtoD0h_Kpipipi_Kpipi0", inputListNames=BsigList, path=path)

        # Select only three random candidates
        ma.rankByHighest(particleList="B+:BtoD0h_Kpipipi_Kpipi0", variable="cos(mdstIndex)", numBest=3,
                         outputVariable="cosMdstIndex_rank", path=path)

        return ["B+:BtoD0h_Kpipipi_Kpipi0"]


@fancy_skim_header
class BtoD0h_Kshh(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 K^+ \\pi^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 K^- \\pi^+) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 K^+ K^-) \\pi^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 K^+ \\pi^-) K^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 K^- \\pi^+) K^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^-) K^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K_{\\rm S}^0 K^+ K^-) K^+`,

    Cuts applied:

    * ``1.5 < M_D0 < 2.2``
    * ``5.2 < Mbc``
    * ``deltaE < 0.3 and deltaE > -0.45``

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
        Bcuts = "Mbc > 5.2 and deltaE < 0.3 and deltaE > -0.45"

        BsigChannels = ["anti-D0:Kshh pi+:GoodTrack",
                        "anti-D0:Kshh K+:GoodTrack"
                        ]
        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:BtoD0h_Kshh" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B+:BtoD0h_Kshh" + str(chID))

        return BsigList

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.reconstructDecay('D0 -> K_S0:merged pi+:all pi-:all', '1.84 < M < 1.89', path=path)
        ma.reconstructDecay('B-:ch3 ->D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

        # the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
        create_validation_histograms(
            rootfile=f'{self}_Validation.root',
            particlelist='B-:ch3',
            variables_1d=[
                ('Mbc', 100, 5.2, 5.3, 'Mbc', self.__contact__, 'Beam constrained mass', '', 'Mbc', 'Candidates'),
                ('deltaE', 100, -1, 1, 'deltaE', self.__contact__, 'Energy difference of B', '', 'deltaE', 'Candidates'),
                ('daughter(0, InvM)', 100, 1.8, 1.9, 'D0_InvM', self.__contact__, 'D0 invariant mass', '', 'InvM', 'Candidates')],
            variables_2d=[
                ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7, 'Mbc vs deltaE', self.__contact__,
                 'Beam constrained mass vs energy difference of reconstructed B', 'Mbc', 'DeltaE')],
            path=path)


@fancy_skim_header
class BtoD0rho_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^+ \\pi^-) \\rho^+`,
    * :math:`B^{+}\\to \\overline{D}{}^{*0} (\\to \\overline{D}{}^{0} (\\to K^+ \\pi^-) \\pi^0) \\rho^+`,

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

        return BsigList


@fancy_skim_header
class BtoD0rho_Kpipipi_Kpipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to \\overline{D}{}^{0} (\\to K^+2\\pi^-\\pi^+, K^+\\pi^-\\pi^0) \\rho^+`
    * :math:`B^{+}\\to \\overline{D}{}^{*0} (\\to \\overline{D}{}^{0} (\\to K^+2 \\pi^-
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

        ma.copyLists(outputListName="B+:BtoD0rho_Kpipipi_Kpipi0", inputListNames=BsigList, path=path)

        # Select only three random candidates to save them as these channels have high multiplicity.
        ma.rankByHighest(particleList="B+:BtoD0rho_Kpipipi_Kpipi0", variable="cos(mdstIndex)", numBest=3,
                         outputVariable="cosMdstIndex_rank", path=path)

        return ["B+:BtoD0rho_Kpipipi_Kpipi0"]


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
            ma.reconstructDecay("B0:B0toDD_Kpipi_Kspi" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B0:B0toDD_Kpipi_Kspi" + str(chID))

        return BsigList


@fancy_skim_header
class B0toDstarD(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to \\overline{D}{}^{*-} (\\to \\overline{D}{}^{0}
      (\\to K^+ \\pi^-, \\to K^+ \\pi^- \\pi^- \\pi^+, K^+ \\pi^- \\pi^0) \\pi^-) D^+ (\\to K^- \\pi^+ \\pi^+)`
    * :math:`B^{0}\\to \\overline{D}{}^{*-} (\\to D^{-} \\pi^0) D^+ (\\to K^- \\pi^+ \\pi^+)`

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

        return BsigList


@fancy_skim_header
class B0toDDs0star(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D_{s0}^{*+} (\\to D_{s}^{+} (\\to \\phi (\\to K^+ K^-) \\pi^+ \\pi^0) \\pi^0) D^{-} (\\to K^+ \\pi^- \\pi^-)`
    * :math:`B^{0}\\to D_{s0}^{*+} (\\to D_{s}^{+} (\\to \\overline{K}{}^{*0} (\\to K^- \\pi^+ ) K^+) \\pi^0) D^{-}
      (\\to K^+ \\pi^- \\pi^-)`
    * :math:`B^{0}\\to D_{s0}^{*+} (\\to D_{s}^{+} (\\to \\phi (\\to K^+ K^-) \\pi^+) \\pi^0) D^{-} (\\to K^+ \\pi^- \\pi^-)`

    Cuts applied:

    * ``5.2 < Mbc < 5.3``
    * ``abs(deltaE) < 0.2``
    * ``0.31 < D_s0ST_massDifference_0 < 0.347``
    """

    __authors__ = ["Kuanying Wu"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdDplus_Kpipi(path=path)
        loadStdPi0ForBToHadrons(path=path)

    def build_lists(self, path):
        ma.reconstructDecay("phi:KK_B0toDDs0star -> K+:GoodTrack K-:GoodTrack",
                            cut="[1.01 < M < 1.03]", path=path)
        ma.reconstructDecay("anti-K*0:B0toDDs0star -> K-:GoodTrack pi+:GoodTrack",
                            cut="[0.793 < M < 1.015]", path=path)
        ma.reconstructDecay("D_s+:phipipi0_B0toDDs0star -> phi:KK_B0toDDs0star pi+:GoodTrack pi0:bth_skim",
                            cut="[1.942 < M < 1.978]", path=path)
        ma.reconstructDecay("D_s+:antiKK_B0toDDs0star -> anti-K*0:B0toDDs0star K+:GoodTrack",
                            cut="[1.944 < M < 1.992]", path=path)
        ma.reconstructDecay("D_s+:phipi_B0toDDs0star -> phi:KK_B0toDDs0star pi+:GoodTrack",
                            cut="[1.935 < M < 1.999]", path=path)
        Dslist = ['D_s+:phipipi0_B0toDDs0star', 'D_s+:antiKK_B0toDDs0star', 'D_s+:phipi_B0toDDs0star']
        ma.copyLists(outputListName='D_s+:all_DDs0star', inputListNames=Dslist, path=path)

        ma.reconstructDecay("D_s0*+:all_DDs0star -> D_s+:all_DDs0star pi0:bth_skim",
                            cut="[2.249 < M < 2.298] and [0.31 < massDifference(0) < 0.347] and \
                            [1.017 < p < 2.552]", path=path)

        ma.reconstructDecay("B0:B0toDDs0star -> D_s0*+:all_DDs0star D-:Kpipi",
                            cut="[5.2 < Mbc < 5.3] and \
                            [-0.2 < deltaE < 0.2]", path=path)

        return ["B0:B0toDDs0star"]


@fancy_skim_header
class B0toD0Kpipi0_pi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to \\overline{D}{}^{0} (\\to K^+ \\pi^- \\pi^0) \\pi^0`

    Cuts applied:

    * ``Mbc > 5.2``
    * ``abs(deltaE) < 0.5``

    Note:
        This skim uses `skim.standardlists.charm.loadStdD0_Kpipi0`, where the
        :math:`\\overline{D}{}^{0}` channel is defined.
    """

    __authors__ = ["Francis Pham"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair

    def load_standard_lists(self, path):
        loadStdPi0ForBToCharmless(path=path)
        loadStdVeryLooseTracks('K', path=path)
        loadStdVeryLooseTracks('pi', path=path)
        loadCharmlessD0_Kpipi0(path=path)

    def build_lists(self, path):
        Bcuts = "5.2 < Mbc and abs(deltaE) < 0.5"

        ma.reconstructDecay("B0:D0Kpipi0_pi0 -> anti-D0:Kpipi0_loose pi0:charmlessFit", Bcuts, path=path)

        return ["B0:D0Kpipi0_pi0"]


@fancy_skim_header
class B0toDs1D(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0}\\to D_{s1}^{+} (\\to D_s^{*+}(\\to D_s^+(\\to \\phi (\\to K^+ K^-)\\pi^+) \\gamma) \\pi^0)
      D^- (\\to K^+ \\pi^-\\pi^-)`,
    * :math:`B^{0}\\to D_{s1}^{+} (\\to D_s^{*+}(\\to D_s^+(\\to \\phi (\\to K^+ K^-) \\pi^+ \\pi^0) \\gamma) \\pi^0)
      D^- (\\to K^+ \\pi^-\\pi^-)`,
    * :math:`B^{0}\\to D_{s1}^{+} (\\to D_s^{*+}(\\to D_s^+(\\to \\overline{K}{}^{*0} (\\to K^- \\pi^+)K^+) \\gamma) \\pi^0)
      D^- (\\to K^+ \\pi^-\\pi^-)`,
    * :math:`B^{0}\\to D_{s1}^{+} (\\to D_s^{*+}(\\to D_s^+(\\to K_S^0 (\\to \\pi^- \\pi^+)K^+) \\gamma) \\pi^0)
      D^- (\\to K^+ \\pi^-\\pi^-)`,

    Cuts applied:

    * ``5.2 < Mbc < 5.3``
    * ``-0.5 < deltaE < 0.5``
    * ``0.247 < M(D_s1+) - M(D_s*+) < 0.378``
    * ``0.072 < M(D_s*+) - M(D_s+) < 0.179``
    * ``2.288 < M(D_s1+) < 2.507``
    """

    __authors__ = ["Tsai Hua Lee, Chih Han Tseng"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        loadPiSkimHighEff(path=path)
        loadKSkimHighEff(path=path)
        loadStdPi0ForBToHadrons(path=path)
        stdPhotons('loose', path=path)
        stdKshorts(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path)
        loadStdDplus_Kpipi(path=path)
        loadStdSkimHighEffPhi(path=path)
        loadStdSkimHighEffKstar0(path=path)

    def build_lists(self, path):
        ma.reconstructDecay(decayString="D_s+:phipiSkimHighEff_B0toDs1D -> phi:SkimHighEff pi+:SkimHighEff",
                            cut="[1.942 < M < 1.993]", path=path)
        ma.reconstructDecay(decayString="D_s+:phipipi0SkimHighEff_B0toDs1D -> phi:SkimHighEff pi+:SkimHighEff pi0:bth_skim",
                            cut="[1.874 < M < 1.997]", path=path)
        ma.reconstructDecay(decayString="D_s+:Ksk_B0toDs1D -> K_S0:merged K+:SkimHighEff",
                            cut="[1.914 < M < 2.015]", path=path)
        ma.reconstructDecay(decayString="D_s+:anti-Kstar0K_B0toDs1D -> anti-K*0:SkimHighEff K+:SkimHighEff",
                            cut="[1.934 < M < 2.002]", path=path)
        DsList = [
            'D_s+:phipiSkimHighEff_B0toDs1D',
            'D_s+:phipipi0SkimHighEff_B0toDs1D',
            'D_s+:Ksk_B0toDs1D',
            'D_s+:anti-Kstar0K_B0toDs1D']
        ma.copyLists(outputListName="D_s+:all_B0toDs1D", inputListNames=DsList, path=path)

        ma.reconstructDecay(decayString="D_s*+:Ds1D_B0toDs1D -> D_s+:all_B0toDs1D gamma:loose",
                            cut="[2.062 < M < 2.131] and [0.072 < massDifference(0) < 0.179]", path=path)
        ma.reconstructDecay(decayString="D_s1+:Ds1D_B0toDs1D -> D_s*+:Ds1D_B0toDs1D pi0:bth_skim",
                            cut="[2.288 < M < 2.507] and [0.247 < massDifference(0) < 0.378]", path=path)
        ma.reconstructDecay(decayString="B0:B0toDs1D -> D_s1+:Ds1D_B0toDs1D D-:Kpipi",
                            cut="[5.2 < Mbc < 5.3] and [-0.5 < deltaE < 0.5]", path=path)

        return ["B0:B0toDs1D"]


@fancy_skim_header
class B0toDomegapi_Kpipi_pipipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0} \\to D^{-} (\\to K^+ \\pi^- \\pi^-) \\omega (\\to \\pi^- \\pi^+ \\pi^0) \\pi^+`,

    Cuts applied:

    * ``1.84 < M_D < 1.90``
    * ``0.70 < M_omega < 0.90``
    * ``Mbc > 5.25``
    * ``abs(deltaE) < 0.25``
    """

    __authors__ = ["Dmitry Matvienko"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPi0s("eff40_May2020Fit", path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.25 and abs(deltaE) < 0.25"
        omegacuts = "0.70 < M < 0.90"
        Dcuts = "1.84 < M < 1.90"

        ma.reconstructDecay(
            "omega:B0toDomegapi_Kpipi_pipipi0 -> pi-:GoodTrack pi+:GoodTrack pi0:eff40_May2020Fit",
            omegacuts,
            path=path)
        ma.reconstructDecay("D-:B0toDomegapi_Kpipi_pipipi0 -> K+:GoodTrack pi-:GoodTrack pi-:GoodTrack", Dcuts, path=path)
        ma.reconstructDecay(
            "B0:B0toDomegapi_Kpipi_pipipi0 -> D-:B0toDomegapi_Kpipi_pipipi0 omega:B0toDomegapi_Kpipi_pipipi0 pi+:GoodTrack",
            Bcuts,
            path=path)

        return ["B0:B0toDomegapi_Kpipi_pipipi0"]


@fancy_skim_header
class B0toDomegapi_Kspi_pipipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{0} \\to D^{-} (\\to K_{\\rm S}^0 \\pi^-) \\omega (\\to \\pi^- \\pi^+ \\pi^0) \\pi^+`,

    Cuts applied:

    * ``1.84 < M_D < 1.90``
    * ``0.70 < M_omega < 0.90``
    * ``Mbc > 5.25``
    * ``abs(deltaE) < 0.25``
    """

    __authors__ = ["Dmitry Matvienko"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdKshorts(path=path)
        stdPi0s("eff40_May2020Fit", path=path)
        loadPiForBtoHadrons(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.25 and abs(deltaE) < 0.25"
        omegacuts = "0.70 < M < 0.90"
        Dcuts = "1.84 < M < 1.90"

        ma.reconstructDecay(
            "omega:B0toDomegapi_Kspi_pipipi0 -> pi-:GoodTrack pi+:GoodTrack pi0:eff40_May2020Fit",
            omegacuts,
            path=path)
        ma.reconstructDecay("D-:B0toDomegapi_Kspi_pipipi0 -> K_S0:merged pi-:GoodTrack", Dcuts, path=path)
        ma.reconstructDecay(
            "B0:B0toDomegapi_Kspi_pipipi0 -> D-:B0toDomegapi_Kspi_pipipi0 omega:B0toDomegapi_Kspi_pipipi0 pi+:GoodTrack",
            Bcuts,
            path=path)

        return ["B0:B0toDomegapi_Kspi_pipipi0"]


@fancy_skim_header
class BtoD0pi_Kpiomega_pipipi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+} \\to \\bar{D}^{0} (\\to K^+ \\pi^- \\omega (\\to \\pi^- \\pi^+ \\pi^0)) \\pi^+`,

    Cuts applied:

    * ``1.7 < M_D < 2.0``
    * ``0.73 < M_omega < 0.9``
    * ``Mbc > 5.25``
    * ``abs(deltaE) < 0.25``
    """

    __authors__ = ["Dmitry Matvienko"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPi0s("eff40_May2020Fit", path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.25 and abs(deltaE) < 0.25"
        omegacuts = "0.73 < M < 0.9"
        Dcuts = "1.7 < M < 2.0"

        ma.reconstructDecay(
            "omega:BtoD0pi_Kpiomega_pipipi0 -> pi-:GoodTrack pi+:GoodTrack pi0:eff40_May2020Fit",
            omegacuts,
            path=path)
        ma.reconstructDecay(
            "anti-D0:BtoD0pi_Kpiomega_pipipi0 -> K+:GoodTrack pi-:GoodTrack omega:BtoD0pi_Kpiomega_pipipi0",
            Dcuts,
            path=path)
        ma.reconstructDecay("B+:BtoD0pi_Kpiomega_pipipi0 -> anti-D0:BtoD0pi_Kpiomega_pipipi0 pi+:GoodTrack", Bcuts, path=path)

        return ["B+:BtoD0pi_Kpiomega_pipipi0"]


@fancy_skim_header
class BtoDstpipi_D0pi_Kpi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{-}\\to D^{*+} (\\to D^0 (\\to K^+ \\pi^-) \\pi^+) \\pi^- \\pi^-`,

    Cuts applied:

    * ``5.22 < Mbc``
    * ``-0.3 < deltaE < 0.2``

    """
    __authors__ = ["Agrim Aggarwal"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        loadKForBtoHadrons(path=path),
        loadPiForBtoHadrons(path=path),
        loadStdD0_Kpi(path=path),
        loadStdDstarPlus_D0pi_Kpi(path=path),
        loadSlowPi(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.22 and -0.3 < deltaE < 0.2"

        ma.reconstructDecay("B-:Dsthpipi_D0pi_Kpi -> D*+:D0_Kpi pi-:GoodTrack pi-:GoodTrack", Bcuts, path=path)

        return ["B-:Dsthpipi_D0pi_Kpi"]


@fancy_skim_header
class BtoDpipi_Kpipi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{-}\\to D^{+} (\\to K^- \\pi^+ \\pi^-) \\pi^- \\pi^-`,


    Cuts applied:

    * ``5.22 < Mbc``
    * ``-0.3 < deltaE < 0.2``
    """
    __authors__ = ["Melisa-Melek Akdag"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdDplus_Kpipi(path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.22 and -0.3 < deltaE < 0.2"
        ma.reconstructDecay("B-:BtoDpipi_Kpipi -> D+:Kpipi pi-:GoodTrack pi-:GoodTrack",
                            Bcuts,
                            path=path)
        return ["B-:BtoDpipi_Kpipi"]


@fancy_skim_header
class BtoDsDsst_Kpi_KK(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^{+}\\to D_{s}^{-} (\\to \\phi (\\to K^+ K^-) \\pi^+) K^+ \\pi^+`,
    * :math:`B^{+}\\to D_{s}^{-} (\\to K^{*0} (\\to K^- \\pi^+) K^-) K^+ \\pi^+`,
    * :math:`B^{+}\\to D_{s}^{-} (\\to K_{\\rm S}^0 (\\to \\pi^+ \\pi^-) K^-) K^+ \\pi^+`,
    * :math:`B^{+}\\to D_{s}^{-} (\\to \\phi (\\to K^+ K^-) \\pi^+) K^+ K^+`,
    * :math:`B^{+}\\to D_{s}^{-} (\\to K^{*0} (\\to K^- \\pi^+) K^-) K^+ K^+`,
    * :math:`B^{+}\\to D_{s}^{-} (\\to K_{\\rm S}^0 (\\to \\pi^+ \\pi^-) K^-) K^+ K^+`,
    * :math:`B^{+}\\to D_{s}^{*-} (\\to D_{s}^{-} \\gamma ) K^+ \\pi^+`,
    * :math:`B^{+}\\to D_{s}^{*-} (\\to D_{s}^{-} \\gamma ) K^+ K^+`,


    Cuts applied:

    * ``5.24 < Mbc``
    * ``-0.2 < deltaE < 0.2``
    * ``0.82 < M_K*0 < 0.98``
    * ``1.93 < M_Ds < 1.99``
    * ``2.06 < M_D*s < 2.15``
    """

    __authors__ = ["Agrim Aggarwal"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        stdKshorts(path=path)
        stdPhotons("loose", path=path)

    def build_lists(self, path):

        Bcuts = "Mbc > 5.24 and -0.2 < deltaE < 0.2"

        ma.reconstructDecay("anti-K*0:Kpi_BtoDsDsst_Kpi_KK -> K-:GoodTrack pi+:GoodTrack",
                            cut="[0.82 < M < 0.98]", path=path)

        ma.reconstructDecay("D_s+:KKpi_BtoDsDsst_Kpi_KK -> K+:GoodTrack K-:GoodTrack pi+:GoodTrack ",
                            cut="[1.93 < M < 1.99]", path=path)

        ma.reconstructDecay("D_s+:KstK_BtoDsDsst_Kpi_KK -> anti-K*0:Kpi_BtoDsDsst_Kpi_KK K+:GoodTrack",
                            cut="[1.93 < M < 1.99]", path=path)

        ma.reconstructDecay("D_s+:KszK_BtoDsDsst_Kpi_KK -> K_S0:merged K+:GoodTrack ",
                            cut="[1.93 < M < 1.99]", path=path)

        Dslist = ['D_s+:KKpi_BtoDsDsst_Kpi_KK', 'D_s+:KstK_BtoDsDsst_Kpi_KK', 'D_s+:KszK_BtoDsDsst_Kpi_KK']

        ma.copyLists(outputListName='D_s+:all_BtoDsDsst_Kpi_KK', inputListNames=Dslist, path=path)

        ma.reconstructDecay('D_s*+:Dsg_BtoDsDsst_Kpi_KK -> D_s+:all_BtoDsDsst_Kpi_KK gamma:loose',
                            cut="[2.06 < M < 2.15]", path=path)

        BsigChannels = [
            "D_s-:all_BtoDsDsst_Kpi_KK K+:GoodTrack pi+:GoodTrack",
            "D_s-:all_BtoDsDsst_Kpi_KK K+:GoodTrack K+:GoodTrack",
            "D_s*-:Dsg_BtoDsDsst_Kpi_KK K+:GoodTrack pi+:GoodTrack",
            "D_s*-:Dsg_BtoDsDsst_Kpi_KK K+:GoodTrack K+:GoodTrack",
            ]

        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("B+:BtoDsDsst_Kpi_KK" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("B+:BtoDsDsst_Kpi_KK" + str(chID))

        return BsigList


@fancy_skim_header
class antiB0toDpipi0_Kpipi(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\bar{B^{0}}\\to D^{+} (\\to K^- \\pi^+ \\pi^-) \\pi^- \\pi^0`,


    Cuts applied:

    * ``5.22 < Mbc``
    * ``-0.3 < deltaE < 0.2``
    """
    __authors__ = ["Agrim Aggarwal"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdDplus_Kpipi(path=path)
        stdPi0s("eff50_May2020Fit", path=path)

    def build_lists(self, path):
        Bcuts = "Mbc > 5.22 and -0.3 < deltaE < 0.2"
        ma.reconstructDecay("anti-B0:antiB0toDpipi0_Kpipi -> D+:Kpipi pi-:GoodTrack pi0:eff50_May2020Fit",
                            Bcuts,
                            path=path)
        return ["anti-B0:antiB0toDpipi0_Kpipi"]


@fancy_skim_header
class antiB0toDsDsst_Kpi0(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`\\bar{B^{0}}\\to D_{s}^{+} (\\to \\phi (\\to K^+ K^-) \\pi^+) K^- \\pi^0`,
    * :math:`\\bar{B^{0}}\\to D_{s}^{+} (\\to K^{*0} (\\to K^- \\pi^+) K^-) K^- \\pi^0`,
    * :math:`\\bar{B^{0}}\\to D_{s}^{+} (\\to K_{\\rm S}^0 (\\to \\pi^+ \\pi^-) K^-) K^- \\pi^0`,
    * :math:`\\bar{B^{0}}\\to D_{s}^{*+} (\\to D_{s}^{-} \\gamma ) K^- \\pi^0`,


    Cuts applied:

    * ``5.24 < Mbc``
    * ``-0.2 < deltaE < 0.2``
    * ``0.82 < M_K*0 < 0.98``
    * ``1.93 < M_Ds < 1.99``
    * ``2.06 < M_D*s < 2.15``
    """

    __authors__ = ["Agrim Aggarwal"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charm"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        stdKshorts(path=path)
        stdPhotons("loose", path=path)
        stdPi0s("eff40_May2020Fit", path=path)

    def build_lists(self, path):

        Bcuts = "Mbc > 5.24 and -0.2 < deltaE < 0.2"

        ma.reconstructDecay("anti-K*0:Kpi_antiB0toDsDsst_Kpi0 -> K-:GoodTrack pi+:GoodTrack",
                            cut="[0.82 < M < 0.98]", path=path)

        ma.reconstructDecay("D_s+:KKpi_antiB0toDsDsst_Kpi0 -> K+:GoodTrack K-:GoodTrack pi+:GoodTrack ",
                            cut="[1.93 < M < 1.99]", path=path)

        ma.reconstructDecay("D_s+:KstK_antiB0toDsDsst_Kpi0 -> anti-K*0:Kpi_antiB0toDsDsst_Kpi0 K+:GoodTrack",
                            cut="[1.93 < M < 1.99]", path=path)

        ma.reconstructDecay("D_s+:KszK_antiB0toDsDsst_Kpi0 -> K_S0:merged K+:GoodTrack ",
                            cut="[1.93 < M < 1.99]", path=path)

        Dslist = ['D_s+:KKpi_antiB0toDsDsst_Kpi0', 'D_s+:KstK_antiB0toDsDsst_Kpi0', 'D_s+:KszK_antiB0toDsDsst_Kpi0']

        ma.copyLists(outputListName='D_s+:all_antiB0toDsDsst_Kpi0', inputListNames=Dslist, path=path)

        ma.reconstructDecay('D_s*+:Dsg_antiB0toDsDsst_Kpi0 -> D_s+:all_antiB0toDsDsst_Kpi0 gamma:loose',
                            cut="[2.06 < M < 2.15]", path=path)

        BsigChannels = [
            "D_s+:all_antiB0toDsDsst_Kpi0 K-:GoodTrack pi0:eff40_May2020Fit",
            "D_s*+:Dsg_antiB0toDsDsst_Kpi0 K-:GoodTrack pi0:eff40_May2020Fit",
            ]

        BsigList = []
        for chID, channel in enumerate(BsigChannels):
            ma.reconstructDecay("anti-B0:antiB0toDsDsst_Kpi0" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BsigList.append("anti-B0:antiB0toDsDsst_Kpi0" + str(chID))

        return BsigList

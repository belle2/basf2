#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Skim list building functions for :math:`B\\to X_u +h` analyses
"""

__authors__ = [
    "Kim Smith"
]

import modularAnalysis as ma
from skim.standardlists.lightmesons import loadStdLightMesons
from skimExpertFunctions import BaseSkim, fancy_skim_header
from stdCharged import stdCharged
from stdPhotons import stdPhotons
from stdPi0s import loadStdSkimPi0, stdPi0s
from stdV0s import stdKshorts

__liaison__ = "Benedikt Wach <benedikt.wach@desy.de>"


@fancy_skim_header
class CharmlessHad2Body(BaseSkim):
    """
    **Particle Lists**:

    * Standard loose lists for all light mesons.
    * Modified ``K_S0:merged`` with tighter invariant mass, ``0.48 < M < 0.51``.
    * Modified ``pi0:skim`` with tighter invariant mass and minimum energy, ``0.12 < M < 0.14 and E > 0.25``.

    **Additional Cuts**:

    * ``R2 < 0.5``
    * ``nTracks >= 2 (event level)``
    * ``5.24 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``
    """

    __authors__ = ["Kim Smith"]
    __description__ = "Skim list for all neutral and charged :math:`B` to charmless 2 body modes."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charmless"

    def load_standard_lists(self, path):
        stdCharged("K", "loose", path=path)
        stdCharged("pi", "loose", path=path)
        stdPhotons("loose", path=path)
        stdPi0s("eff40_Jan2020", path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        loadStdLightMesons(path=path)

    def CharmlessHad2BodyB0List(self, path):
        """
        Skim list definitions for all neutral B to charmless 2 body modes.

        **Decay Modes**:

        * ``B0 -> pi+ pi-``
        * ``B0 -> pi0 pi0``
        * ``B0 -> K+ pi-``
        * ``B0 -> K+ K-``
        * ``B0 -> eta pi0``
        * ``B0 -> eta eta``
        * ``B0 -> K_S0 pi0``
        * ``B0 -> rho0 pi0``
        * ``B0 -> rho- pi+``
        * ``B0 -> K+ rho-``
        * ``B0 -> eta rho0``
        * ``B0 -> omega eta``
        * ``B0 -> phi pi0``
        * ``B0 -> phi eta``
        * ``B0 -> omega pi0``
        * ``B0 -> eta K*0``
        * ``B0 -> K*+ pi-``
        * ``B0 -> K*0 pi0``
        * ``B0 -> K*+ K-``
        * ``B0 -> rho0 rho0``
        * ``B0 -> rho+ rho-``
        * ``B0 -> omega rho0``
        * ``B0 -> omega omega``
        * ``B0 -> phi rho0``
        * ``B0 -> phi omega``
        * ``B0 -> phi phi``
        * ``B0 -> omega K*0``
        * ``B0 -> K*0 rho0``
        * ``B0 -> K*+ rho-``
        * ``B0 -> K*0 phi``
        * ``B0 -> K*0 anti-K*0``
        * ``B0 -> K*0 K*0``
        * ``B0 -> K*+ K*-``
        * ``B0 -> eta f_0``
        * ``B0 -> omega f_0``
        * ``B0 -> phi f_0``
        * ``B0 -> rho0 f_0``
        * ``B0 -> K*0 f_0``
        * ``B0 -> f_0 f_0``

        Note:
            Does not include decay modes involving eta prime at the current time.
        """

        ma.cutAndCopyList("pi0:hbd", "pi0:skim", "0.12 < M < 0.14 and E > 0.25", path=path)
        ma.cutAndCopyList("K_S0:hbd", "K_S0:merged", "0.48 < M < 0.51", path=path)

        Bcuts = "5.24 < Mbc < 5.29 and abs(deltaE) < 0.5"

        B0_PPChannels = ["pi+:loose pi-:loose",  # 1
                         "pi0:hbd pi0:hbd",  # 2
                         "K+:loose pi-:loose",  # 3
                         "K+:loose K-:loose",  # 4
                         "eta:loose pi0:hbd",  # 5
                         "eta:loose eta:loose",  # 6
                         "K_S0:hbd pi0:hbd"]  # 7

        B0_PVChannels = ["rho0:loose pi0:hbd",  # 1
                         "rho-:loose pi+:loose",  # 2
                         "K+:loose rho-:loose",  # 3
                         "eta:loose rho0:loose",  # 4
                         "omega:loose eta:loose",  # 5
                         "phi:loose pi0:hbd",  # 6
                         "phi:loose eta:loose",  # 7
                         "omega:loose pi0:hbd",  # 8
                         "eta:loose K*0:loose",  # 9
                         "K*+:loose pi-:loose",  # 10
                         "K*0:loose pi0:hbd",  # 11
                         "K*+:loose K-:loose"]  # 12

        B0_VVChannels = ["rho0:loose rho0:loose",  # 1
                         "rho+:loose rho-:loose",  # 2
                         "omega:loose rho0:loose",  # 3
                         "omega:loose omega:loose",  # 4
                         "phi:loose rho0:loose",  # 5
                         "phi:loose omega:loose",  # 6
                         "phi:loose phi:loose",  # 7
                         "omega:loose K*0:loose",  # 8
                         "K*0:loose rho0:loose",  # 9
                         "K*+:loose rho-:loose",  # 10
                         "K*0:loose phi:loose",  # 11
                         "K*0:loose anti-K*0:loose",  # 12
                         "K*0:loose K*0:loose",  # 13
                         "K*+:loose K*-:loose"]  # 14

        B0_PSChannels = ["eta:loose f_0:loose"]  # 1

        B0_VSChannels = ["omega:loose f_0:loose",  # 1
                         "phi:loose f_0:loose",  # 2
                         "rho0:loose f_0:loose",  # 3
                         "K*0:loose f_0:loose"]  # 4

        B0_SSChannels = ["f_0:loose f_0:loose"]  # 1

        B0PPList = []
        for chID, channel in enumerate(B0_PPChannels):
            ma.reconstructDecay("B0:HAD_b2PP" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            B0PPList.append("B0:HAD_b2PP" + str(chID))

        B0PVList = []
        for chID, channel in enumerate(B0_PVChannels):
            ma.reconstructDecay("B0:HAD_b2PV" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            B0PVList.append("B0:HAD_b2PV" + str(chID))

        B0VVList = []
        for chID, channel in enumerate(B0_VVChannels):
            ma.reconstructDecay("B0:HAD_b2VV" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            B0VVList.append("B0:HAD_b2VV" + str(chID))

        B0PSList = []
        for chID, channel in enumerate(B0_PSChannels):
            ma.reconstructDecay("B0:HAD_b2PS" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            B0PSList.append("B0:HAD_b2PS" + str(chID))

        B0VSList = []
        for chID, channel in enumerate(B0_VSChannels):
            ma.reconstructDecay("B0:HAD_b2VS" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            B0VSList.append("B0:HAD_b2VS" + str(chID))

        B0SSList = []
        for chID, channel in enumerate(B0_SSChannels):
            ma.reconstructDecay("B0:HAD_b2SS" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            B0SSList.append("B0:HAD_b2SS" + str(chID))

        ma.copyLists("B0:2BodyB0", B0PPList + B0PVList + B0VVList + B0PSList + B0VSList + B0SSList, path=path)

        ma.buildRestOfEvent("B0:2BodyB0", path=path)
        cleanMask = ("cleanMask", "useCMSFrame(p)<=3.2", "p >= 0.05 and useCMSFrame(p)<=3.2")
        ma.appendROEMasks("B0:2BodyB0", [cleanMask], path=path)
        ma.buildContinuumSuppression("B0:2BodyB0", "cleanMask", path=path)
        ma.cutAndCopyList("B0:2BodySkim", "B0:2BodyB0", "R2 < 0.5", path=path)

        List = ["B0:2BodySkim"]
        return List

    def CharmlessHad2BodyBmList(self, path):
        """
        Skim list definitions for all charged B to charmless 2 body modes.

        **Decay Modes**:

        * ``B- -> pi- pi0``
        * ``B- -> K- pi0``
        * ``B- -> eta pi-``
        * ``B- -> eta K-``
        * ``B- -> rho0 pi-``
        * ``B- -> rho- pi0``
        * ``B- -> K- rho0``
        * ``B- -> omega pi-``
        * ``B- -> eta rho-``
        * ``B- -> phi pi-``
        * ``B- -> eta K*-``
        * ``B- -> omega K-``
        * ``B- -> K*0 pi-``
        * ``B- -> K*- pi0``
        * ``B- -> K- phi``
        * ``B- -> rho- rhro0``
        * ``B- -> omega rho-``
        * ``B- -> phi rho-``
        * ``B- -> omega K*-``
        * ``B- -> K*- rho0``
        * ``B- -> K*0 rho-``
        * ``B- -> K*- K*0``
        * ``B- -> K*- phi``
        * ``B- -> pi- f_0``
        * ``B- -> f_0 K-``
        * ``B- -> rho- f_0``
        * ``B- -> K*- f_0``

        Note:
            Does not include decay modes involving eta prime at the current time.
        """

        ma.cutAndCopyList("pi0:hbd", "pi0:skim", "0.12 < M < 0.14 and E > 0.25", path=path)
        ma.cutAndCopyList("K_S0:hbd", "K_S0:merged", "0.48 < M < 0.51", path=path)

        Bcuts = "5.24 < Mbc < 5.29 and abs(deltaE) < 0.5"

        Bm_PPChannels = ["pi-:loose pi0:hbd",  # 1
                         "K-:loose pi0:hbd",  # 2
                         "eta:loose pi-:loose",  # 3
                         "eta:loose K-:loose"]  # 4

        Bm_PVChannels = ["rho0:loose pi-:loose",  # 1
                         "rho-:loose pi0:hbd",  # 2
                         "K-:loose rho0:loose",  # 3
                         "omega:loose pi-:loose",  # 4
                         "eta:loose rho-:loose",  # 5
                         "phi:loose pi-:loose",  # 6
                         "eta:loose K*-:loose",  # 7
                         "omega:loose K-:loose",  # 8
                         "K*0:loose pi-:loose",  # 9
                         "K*-:loose pi0:hbd",  # 10
                         "K-:loose phi:loose"]  # 11

        Bm_VVChannels = ["rho-:loose rho0:loose",  # 1
                         "omega:loose rho-:loose",  # 2
                         "phi:loose rho-:loose",  # 3
                         "omega:loose K*-:loose",  # 4
                         "K*-:loose rho0:loose",  # 5
                         "K*0:loose rho-:loose",  # 6
                         "K*-:loose K*0:loose",  # 7
                         "K*-:loose phi:loose"]  # 8

        Bm_PSChannels = ["pi-:loose f_0:loose",  # 1
                         "f_0:loose K-:loose"]  # 2

        Bm_VSChannels = ["rho-:loose f_0:loose",  # 1
                         "K*-:loose f_0:loose"]  # 2

        BmPPList = []
        for chID, channel in enumerate(Bm_PPChannels):
            ma.reconstructDecay("B-:HAD_b2PP" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BmPPList.append("B-:HAD_b2PP" + str(chID))

        BmPVList = []
        for chID, channel in enumerate(Bm_PVChannels):
            ma.reconstructDecay("B-:HAD_b2PV" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BmPVList.append("B-:HAD_b2PV" + str(chID))

        BmVVList = []
        for chID, channel in enumerate(Bm_VVChannels):
            ma.reconstructDecay("B-:HAD_b2VV" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BmVVList.append("B-:HAD_b2VV" + str(chID))

        BmPSList = []
        for chID, channel in enumerate(Bm_PSChannels):
            ma.reconstructDecay("B-:HAD_b2PS" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BmPSList.append("B-:HAD_b2PS" + str(chID))

        BmVSList = []
        for chID, channel in enumerate(Bm_VSChannels):
            ma.reconstructDecay("B-:HAD_b2VS" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            BmVSList.append("B-:HAD_b2VS" + str(chID))

        ma.copyLists("B-:2BodyBm", BmPPList + BmPVList + BmVVList + BmPSList + BmVSList, path=path)

        ma.buildRestOfEvent("B-:2BodyBm", path=path)
        cleanMask = ("cleanMask", "useCMSFrame(p)<=3.2", "p >= 0.05 and useCMSFrame(p)<=3.2")
        ma.appendROEMasks("B-:2BodyBm", [cleanMask], path=path)
        ma.buildContinuumSuppression("B-:2BodyBm", "cleanMask", path=path)
        ma.cutAndCopyList("B-:2BodySkim", "B-:2BodyBm", "R2 < 0.5", path=path)

        List = ["B-:2BodySkim"]
        return List

    def build_lists(self, path):
        """Build lists defined in `CharmlessHad2BodyB0List` and `CharmlessHad2BodyBmList`."""
        path = self.skim_event_cuts("nTracks >= 2", path=path)
        self.SkimLists = self.CharmlessHad2BodyB0List(path) + self.CharmlessHad2BodyBmList(path)


@fancy_skim_header
class CharmlessHad3Body(BaseSkim):
    """
    **Particle Lists**:

    * Standard loose lists for all light mesons.
    * Modified ``K_S0:merged`` with tighter invariant mass, ``0.48 < M < 0.51``.
    * Modified ``pi0:skim`` with tighter invariant mass and minimum energy, ``0.12 < M < 0.14 and E > 0.25``.

    **Additional Cuts**:

    * ``R2 < 0.5``
    * ``nTracks >= 2 (event level)``
    * ``5.24 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``
    * ``abs(cosTBTO) < 0.9``
    * ``abs(cosTBz) < 0.85``
    """

    __authors__ = ["Kim Smith"]
    __description__ = "Skim list for all neutral and charged :math:`B` to charmless 3 body modes."
    __contact__ = __liaison__
    __category__ = "physics, hadronic B to charmless"

    def load_standard_lists(self, path):

        stdCharged("K", "loose", path=path)
        stdCharged("pi", "loose", path=path)
        stdPhotons("loose", path=path)
        stdPi0s("eff40_Jan2020", path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        loadStdLightMesons(path=path)

    def CharmlessHad3BodyB0List(self, path):
        """
        Skim list definitions for all neutral B to charmless 3 body modes.

        **Decay Modes**:

        * ``B0 -> pi+ pi- pi0``
        * ``B0 -> K+ pi- pi0``
        * ``B0 -> K+ K- pi0``
        * ``B0 -> K_S0 K_S0 pi0``
        * ``B0 -> K_S0 K_S0 eta``
        * ``B0 -> K_S0 K_S0 K_S0``
        * ``B0 -> rho0 pi+ pi-``
        * ``B0 -> rho0 K+ pi-``
        * ``B0 -> phi pi+ pi-``
        * ``B0 -> K*0 pi+ pi-``
        * ``B0 -> K*0 K+ K-``
        * ``B0 -> K*0 K- pi+``
        * ``B0 -> K*0 K+ pi-``
        * ``B0 -> f_0 pi+ pi-``
        * ``B0 -> f_0 K+ pi-``

        Note:
            Does not include decay modes involving eta prime at the current time.
        """
        ma.cutAndCopyList('pi0:hbd', 'pi0:skim', '0.12 < M < 0.14 and E > 0.25', path=path)
        ma.cutAndCopyList('K_S0:hbd', 'K_S0:merged', '0.48 < M < 0.51', path=path)

        Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

        B0_PPPChannels = ['pi+:loose pi-:loose pi0:hbd',  # 1
                          'K+:loose pi-:loose pi0:hbd',  # 2
                          'K+:loose K-:loose pi0:hbd',  # 3
                          'K_S0:hbd K_S0:hbd pi0:hbd',  # 4
                          'K_S0:hbd K_S0:hbd eta:loose',  # 5
                          'K_S0:hbd K_S0:hbd K_S0:hbd']  # 6

        B0_PPVChannels = ['rho0:loose pi+:loose pi-:loose',  # 1
                          'rho0:loose K+:loose pi-:loose',  # 2
                          'phi:loose pi+:loose pi-:loose',  # 3
                          'K*0:loose pi+:loose pi-:loose',  # 4
                          'K*0:loose K+:loose K-:loose',  # 5
                          'K*0:loose K-:loose pi+:loose',  # 6
                          'K*0:loose K+:loose pi-:loose']  # 7

        B0_PPSChannels = ['f_0:loose pi+:loose pi-:loose',  # 1
                          'f_0:loose K+:loose pi-:loose']  # 2

        B0PPPList = []
        for chID, channel in enumerate(B0_PPPChannels):
            ma.reconstructDecay('B0:HAD_b2PPP' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
            B0PPPList.append('B0:HAD_b2PPP' + str(chID))

        B0PPVList = []
        for chID, channel in enumerate(B0_PPVChannels):
            ma.reconstructDecay('B0:HAD_b2PPV' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
            B0PPVList.append('B0:HAD_b2PPV' + str(chID))

        B0PPSList = []
        for chID, channel in enumerate(B0_PPSChannels):
            ma.reconstructDecay('B0:HAD_b2PPS' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
            B0PPSList.append('B0:HAD_b2PPS' + str(chID))

        ma.copyLists('B0:3BodyB0', B0PPPList + B0PPVList + B0PPSList, path=path)

        ma.buildRestOfEvent('B0:3BodyB0', path=path)
        cleanMask = ('cleanMask', 'useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
        ma.appendROEMasks('B0:3BodyB0', [cleanMask], path=path)
        ma.buildContinuumSuppression('B0:3BodyB0', 'cleanMask', path=path)
        ma.cutAndCopyList('B0:3BodySkim', 'B0:3BodyB0', 'R2 < 0.5 and abs(cosTBTO) < 0.9 and abs(cosTBz) < 0.85', path=path)

        List = ['B0:3BodySkim']
        return List

    def CharmlessHad3BodyBmList(self, path):
        """
        Skim list definitions for all charged B to charmless 3 body modes.

        **Decay Modes**:

        * ``B- -> K- pi+ pi-``
        * ``B- -> K- pi0 pi0``
        * ``B- -> K+ pi- pi-``
        * ``B- -> K- K+ pi-``
        * ``B- -> K- K+ K-``
        * ``B- -> pi- pi- pi+``
        * ``B- -> pi- pi0 pi0``
        * ``B- -> K- K_S0 K_S0``
        * ``B- -> pi- K_S0 K_S0``
        * ``B- -> K_S0 pi- pi0``
        * ``B- -> K*- pi- pi+``
        * ``B- -> K*- pi- K+``
        * ``B- -> K*- K- K+``
        * ``B- -> phi K_S0 pi-``
        * ``B- -> K- phi phi``
        * ``B- -> omega phi K-``

        Note:
            Does not include decay modes involving eta prime at the current time.
        """
        ma.cutAndCopyList('pi0:hbd', 'pi0:skim', '0.12 < M < 0.15 and E > 0.25', path=path)
        ma.cutAndCopyList('K_S0:hbd', 'K_S0:merged', '0.48 < M < 0.51', path=path)

        Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

        Bm_PPPChannels = ['K-:loose pi+:loose pi-:loose',  # 1
                          'K-:loose pi0:hbd pi0:hbd',  # 2
                          'K+:loose pi-:loose pi-:loose',  # 3
                          'K-:loose K+:loose pi-:loose',  # 4
                          'K-:loose K+:loose K-:loose',  # 5
                          'pi-:loose pi-:loose pi+:loose',  # 6
                          'pi-:loose pi0:hbd pi0:hbd',  # 7
                          'K-:loose K_S0:hbd K_S0:hbd',  # 8
                          'pi-:loose K_S0:hbd K_S0:hbd',  # 9
                          'K_S0:hbd pi-:loose pi0:hbd']  # 10

        Bm_PPVChannels = ['K*-:loose pi-:loose pi+:loose',  # 1
                          'K*-:loose pi-:loose K+:loose',  # 2
                          'K*-:loose K-:loose K+:loose',  # 3
                          'phi:loose K_S0:hbd pi-:loose']  # 4

        Bm_PVVChannels = ['K-:loose phi:loose phi:loose',  # 1
                          'omega:loose phi:loose K-:loose']  # 2

        BmPPPList = []
        for chID, channel in enumerate(Bm_PPPChannels):
            ma.reconstructDecay('B-:HAD_b2PPP' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
            BmPPPList.append('B-:HAD_b2PPP' + str(chID))

        BmPPVList = []
        for chID, channel in enumerate(Bm_PPVChannels):
            ma.reconstructDecay('B-:HAD_b2PPV' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
            BmPPVList.append('B-:HAD_b2PPV' + str(chID))

        BmPVVList = []
        for chID, channel in enumerate(Bm_PVVChannels):
            ma.reconstructDecay('B-:HAD_b2PVV' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
            BmPVVList.append('B-:HAD_b2PVV' + str(chID))

        ma.copyLists('B-:3BodyBm', BmPPPList + BmPPVList + BmPVVList, path=path)

        ma.buildRestOfEvent('B-:3BodyBm', path=path)
        cleanMask = ('cleanMask', 'useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
        ma.appendROEMasks('B-:3BodyBm', [cleanMask], path=path)
        ma.buildContinuumSuppression('B-:3BodyBm', 'cleanMask', path=path)
        ma.cutAndCopyList('B-:3BodySkim', 'B-:3BodyBm', 'R2 < 0.5 and abs(cosTBTO) < 0.9 and abs(cosTBz) < 0.85', path=path)

        List = ['B-:3BodySkim']
        return List

    def build_lists(self, path):
        """Build lists defined in `CharmlessHad3BodyB0List` and `CharmlessHad3BodyBmList`."""
        path = self.skim_event_cuts("nTracks >= 2", path=path)
        self.SkimLists = self.CharmlessHad3BodyB0List(path) + self.CharmlessHad3BodyBmList(path)

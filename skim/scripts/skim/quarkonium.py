#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for quarkonium analyses: bottomonium, charmonium, resonance """

__authors__ = [
    "Stefano Spataro"
    "Sen Jia"
    "..."
]

import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header


__liaison__ = "Sen Jia <jiasen@buaa.edu.cn>"


@fancy_skim_header
class BottomoniumEtabExclusive(BaseSkim):
    """
    Reconstructed decay modes:

    * ``eta_b -> gamma gamma``

    Selection criteria:

    * ``2 std photon with E > 3.5 GeV``
    * ``7 < M(eta_b) < 10 GeV/c^2``
    * ``foxWolframR2 < 0.995``
    """

    __authors__ = ["Stefano Spataro", "Sen Jia"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    RequiredStandardLists = {
        "stdPhotons": {
            "stdPhotons": ["loose"],
        },
    }

    def build_lists(self, path):
        # create and fill hard photon
        ma.fillParticleList(decayString="pi+:BottomoniumEtab_eventshape", cut="pt > 0.1", path=path)
        ma.fillParticleList(decayString="gamma:BottomoniumEtab_eventshape", cut="E > 0.1", path=path)

        ma.buildEventShape(inputListNames=["pi+:BottomoniumEtab_eventshape", "gamma:BottomoniumEtab_eventshape"],
                           allMoments=False,
                           foxWolfram=True,
                           harmonicMoments=False,
                           cleoCones=False,
                           thrust=False,
                           collisionAxis=False,
                           jets=False,
                           sphericity=False,
                           checkForDuplicates=False,
                           path=path)

        ma.cutAndCopyList("gamma:hard", "gamma:loose", "E>3.5", path=path)
        ma.applyCuts("gamma:hard", "foxWolframR2 < 0.995", path=path)

        # the requirement of 7 < M(eta_b) < 10 GeV/c2
        Etabcuts = "M > 7 and M < 10"

        # eta_b candidates are reconstructed
        Etab_Channels = ["gamma:hard gamma:hard"]

        # define the eta_b decay list
        EtabList = []

        # reconstruct the decay eta_b -> gamma gamma
        for chID, channel in enumerate(Etab_Channels):
            ma.reconstructDecay("eta_b:all" + str(chID) + " -> " + channel, Etabcuts, chID, path=path)
            EtabList.append("eta_b:all" + str(chID))

        self.SkimLists = EtabList


@fancy_skim_header
class BottomoniumUpsilon(BaseSkim):
    """
    Reconstructed decay modes:

    * Y(1S,2S) -> l^+ l^{-} (l = e or mu)

    Selection criteria:

    * 2 tracks with momentum ranging between ``3.5 < p < 15``
    * At least 1 track ``p < 1.5`` or 1 std photon with ``E > 150 MeV``
    * ``M(Y(1S,2S)) > 8 GeV/c^2``
    * ``foxWolframR2 < 0.995``
    """
    __authors__ = ["Stefano Spataro", "Sen Jia"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    RequiredStandardLists = {
        "stdPhotons": {
            "stdPhotons": ["loose"],
        },
    }

    def build_lists(self, path):
        Ycuts = ""
        # create and fill e/mu/pi/photon ParticleLists
        ma.fillParticleList("mu+:BottomoniumUpsilon", "p<15 and p>3.5", path=path)
        ma.fillParticleList("e+:BottomoniumUpsilon", "p<15 and p>3.5", path=path)
        ma.fillParticleList("pi+:BottomoniumUpsilon", "p<1.5 and pt>0.05", path=path)
        ma.cutAndCopyList("gamma:soft", "gamma:loose", "E>0.15", path=path)

        # Y(1S,2S) are reconstructed with e^+ e^- or mu^+ mu^-
        ma.reconstructDecay("Upsilon:ee -> e+:BottomoniumUpsilon e-:BottomoniumUpsilon", "M > 8", path=path)
        ma.reconstructDecay("Upsilon:mumu -> mu+:BottomoniumUpsilon mu-:BottomoniumUpsilon", "M > 8", path=path)
        ma.copyLists("Upsilon:all", ["Upsilon:ee", "Upsilon:mumu"], path=path)

        # require foxWolframR2 < 0.995
        ma.fillParticleList(decayString="pi+:BottomoniumUpsilon_eventshape", cut="pt > 0.1", path=path)
        ma.fillParticleList(decayString="gamma:BottomoniumUpsilon_eventshape", cut="E > 0.1", path=path)

        ma.buildEventShape(inputListNames=["pi+:BottomoniumUpsilon_eventshape", "gamma:BottomoniumUpsilon_eventshape"],
                           allMoments=False,
                           foxWolfram=True,
                           harmonicMoments=False,
                           cleoCones=False,
                           thrust=False,
                           collisionAxis=False,
                           jets=False,
                           sphericity=False,
                           checkForDuplicates=False,
                           path=path)

        ma.applyCuts("Upsilon:all", "foxWolframR2 < 0.995", path=path)

        # Y(1S,2S) with pi+ or photon are reconstructed
        Upsilon_Channels = ["Upsilon:all pi+:BottomoniumUpsilon",
                            "Upsilon:all gamma:soft"]

        # define the Y(1S,2S) decay channel list
        UpsilonList = []

        # reconstruct the decay channel
        for chID, channel in enumerate(Upsilon_Channels):
            ma.reconstructDecay("junction:all" + str(chID) + " -> " + channel, Ycuts, chID, path=path, allowChargeViolation=True)
            UpsilonList.append("junction:all" + str(chID))

        # reture the list
        self.SkimLists = UpsilonList

    #       *two* sets of validation scripts defined.


@fancy_skim_header
class ISRpipicc(BaseSkim):
    """
    Reconstructed decay modes:

    * ``e+e- -> pi+ pi- J/psi -> e+e-``
    * ``e+e- -> pi+ pi- J/psi -> mu+mu-``
    * ``e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> e+e-``
    * ``e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> mu+mu-``

    Selection criteria:

    * Standard ``e/mu/pi ParticleLists``
    * Mass window cut for ``J/psi`` and ``psi(2S)`` candidates
    * Apply ``-4 < the recoil mass square of hadrons < 4 GeV^{2}/c^{4}`` to extract ISR signal events
    """
    __authors__ = []
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["loose"],
            "stdMu": ["loose"],
            "stdPi": ["loose"],
        },
    }

    def build_lists(self, path):
        # intermediate state J/psi and psi(2S) are reconstructed
        # add mass window cut for J/psi and psi(2S) candidates
        ma.reconstructDecay("J/psi:ee_ISR -> e+:loose e-:loose", "M>3.0 and M<3.2", path=path)
        ma.reconstructDecay("J/psi:mumu_ISR -> mu+:loose mu-:loose", "M>3.0 and M<3.2", path=path)
        ma.reconstructDecay("psi(2S):ee -> pi+:loose pi-:loose e+:loose e-:loose", "M>3.64 and M<3.74", path=path)
        ma.reconstructDecay("psi(2S):mumu -> pi+:loose pi-:loose mu+:loose mu-:loose", "M>3.64 and M<3.74", path=path)

        # the requirement of recoil mass square of hadrons
        MMScuts = "-4 < m2Recoil < 4"

        # four ISR modes are reconstructed
        # e+e- -> pi+ pi- J/psi -> e+e- via ISR
        # e+e- -> pi+ pi- J/psi -> mu+mu- via ISR
        # e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> e+e- via ISR
        # e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> mu+mu- via ISR
        vpho_Channels = [
            "pi+:loose pi-:loose J/psi:ee_ISR",
            "pi+:loose pi-:loose J/psi:mumu_ISR",
            "pi+:loose pi-:loose psi(2S):ee",
            "pi+:loose pi-:loose psi(2S):mumu"
        ]

        # define the ISR process list
        vphoList = []

        # reconstruct the different ISR channels and append to the virtual photon
        for chID, channel in enumerate(vpho_Channels):
            ma.reconstructDecay("vpho:myCombination" + str(chID) + " -> " + channel, MMScuts, chID, path=path)
            vphoList.append("vpho:myCombination" + str(chID))

        self.SkimLists = vphoList

    def validation_histograms(self, path):
        # [ee -> ISR pi+pi- [J/psi -> mu+mu-]] decay
        ma.reconstructDecay("J/psi:mumu_validation -> mu+:95eff mu-:95eff", "2.9 < M < 3.3", path=path)
        ma.reconstructDecay("vpho:myCombinations_validation -> J/psi:mumu_validation pi+:95eff pi-:95eff", "", path=path)

        ma.variablesToHistogram(
            filename="ISRpipimumu_Validation.root",
            decayString="vpho:myCombinations_validation",
            variables=[
                ("daughterInvariantMass(0)", 80, 2.9, 3.3),
                ("useCMSFrame(cosTheta)", 50, -1, 1),
                ("m2Recoil", 50, -1, 1)
            ], path=path)

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


def EtabList(path):
    """
    **Skim code**: 15420200

    **Skim Author**: Stefano Spataro, Sen Jia

    **Decay Modes**:

    * ``eta_b -> gamma gamma``

    **Selection Criteria**:

    1. ``2 std photon with E > 3.5 GeV``
    2. ``7 < M(eta_b) < 10 GeV/c^2``
    3. ``foxWolframR2 < 0.995``

    """
    __author__ = "Stefano Spataro & Sen Jia"

    # create and fill hard photon
    ma.fillParticleList(decayString='pi+:eventShapeForSkims', cut='pt > 0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims', cut='E > 0.1', path=path)

    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
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

    ma.cutAndCopyList('gamma:hard', 'gamma:loose', 'E>3.5', path=path)
    ma.applyCuts('gamma:hard', 'foxWolframR2 < 0.995', path=path)

    # the requirement of 7 < M(eta_b) < 10 GeV/c2
    Etabcuts = 'M > 7 and M < 10'

    # eta_b candidates are reconstructed
    Etab_Channels = ['gamma:hard gamma:hard']

    # define the eta_b decay list
    EtabList = []

    # reconstruct the decay eta_b -> gamma gamma
    for chID, channel in enumerate(Etab_Channels):
        ma.reconstructDecay('eta_b:all' + str(chID) + ' -> ' + channel, Etabcuts, chID, path=path)
        EtabList.append('eta_b:all' + str(chID))

    # return the eta_b decaylist
    return EtabList


def UpsilonList(path):
    """
    **Skim Code**: 15440100

    **Skim Author**: Stefano Spataro, Sen Jia

    **Skim Selection**:

    * Y(1S,2S) -> l^+ l^{-} (l = e or mu)

    **Selection Criteria**

    1. 2 tracks with momentum ranging between ``3.5 < p < 15``
    2. At least 1 track ``p < 1.5`` or 1 std photon with ``E > 150 MeV``
    3. ``M(Y(1S,2S)) > 8 GeV/c^2``
    4. ``foxWolframR2 < 0.995``

    """
    __author__ = "Stefano Spataro & Sen Jia"

    Ycuts = ''
    # create and fill e/mu/pi/photon ParticleLists
    ma.fillParticleList('mu+:loose', 'p<15 and p>3.5', path=path)
    ma.fillParticleList('e+:loose', 'p<15 and p>3.5', path=path)
    ma.fillParticleList('pi+:loose', 'p<1.5 and pt>0.05', path=path)
    ma.cutAndCopyList('gamma:soft', 'gamma:loose', 'E>0.15', path=path)

    # Y(1S,2S) are reconstructed with e^+ e^- or mu^+ mu^-
    ma.reconstructDecay('Upsilon:ee -> e+:loose e-:loose', 'M > 8', path=path)
    ma.reconstructDecay('Upsilon:mumu -> mu+:loose mu-:loose', 'M > 8', path=path)
    ma.copyLists('Upsilon:all', ['Upsilon:ee', 'Upsilon:mumu'], path=path)

    # require foxWolframR2 < 0.995
    ma.fillParticleList(decayString='pi+:eventShapeForSkims', cut='pt > 0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims', cut='E > 0.1', path=path)

    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
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

    ma.applyCuts('Upsilon:all', 'foxWolframR2 < 0.995', path=path)

    # Y(1S,2S) with pi+ or photon are reconstructed
    Upsilon_Channels = ['Upsilon:all pi+:loose',
                        'Upsilon:all gamma:soft']

    # define the Y(1S,2S) decay channel list
    UpsilonList = []

    # reconstruct the decay channel
    for chID, channel in enumerate(Upsilon_Channels):
        ma.reconstructDecay('junction:all' + str(chID) + ' -> ' + channel, Ycuts, chID, path=path)
        UpsilonList.append('junction:all' + str(chID))

    # reture the list
    return UpsilonList


def ISRpipiccList(path):
    """
    **Skim Code**: 16460100

    **Skim Author**: Sen Jia

    **Skim Selection**:

    * ``e+e- -> pi+ pi- J/psi -> e+e-``
    * ``e+e- -> pi+ pi- J/psi -> mu+mu-``
    * ``e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> e+e-``
    * ``e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> mu+mu-``

    **Selection Criteria**:

    (1) Standard `e/mu/pi ParticleLists`
    (2) Mass window cut for ``J/psi`` and ``psi(2S)`` candidates
    (3) Apply ``-4 < the recoil mass square of hadrons < 4 GeV^{2}/c^{4}`` to extract ISR signal events

    """
    __author__ = "Sen Jia"

    # intermediate state J/psi and psi(2S) are reconstructed
    # add mass window cut for J/psi and psi(2S) candidates
    ma.reconstructDecay('J/psi:ee -> e+:loose e-:loose', 'M>3.0 and M<3.2', path=path)
    ma.reconstructDecay('J/psi:mumu -> mu+:loose mu-:loose', 'M>3.0 and M<3.2', path=path)
    ma.reconstructDecay('psi(2S):ee -> pi+:loose pi-:loose e+:loose e-:loose', 'M>3.64 and M<3.74', path=path)
    ma.reconstructDecay('psi(2S):mumu -> pi+:loose pi-:loose mu+:loose mu-:loose', 'M>3.64 and M<3.74', path=path)

    # the requirement of recoil mass square of hadrons
    MMScuts = '-4 < m2Recoil < 4'

    # four ISR modes are reconstructed
    # e+e- -> pi+ pi- J/psi -> e+e- via ISR
    # e+e- -> pi+ pi- J/psi -> mu+mu- via ISR
    # e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> e+e- via ISR
    # e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> mu+mu- via ISR
    vpho_Channels = [
        'pi+:loose pi-:loose J/psi:ee',
        'pi+:loose pi-:loose J/psi:mumu',
        'pi+:loose pi-:loose psi(2S):ee',
        'pi+:loose pi-:loose psi(2S):mumu'
    ]

    # define the ISR process list
    vphoList = []

    # reconstruct the different ISR channels and append to the virtual photon
    for chID, channel in enumerate(vpho_Channels):
        ma.reconstructDecay('vpho:myCombination' + str(chID) + ' -> ' + channel, MMScuts, chID, path=path)
        vphoList.append('vpho:myCombination' + str(chID))

    # return the ISR process list
    return vphoList


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
    __WorkingGroup__ = "Bottomonium"
    __SkimDescription__ = ""

    RequiredStandardLists = {
        "stdPhotons": {
            "stdPhotons": ["loose"],
        },
    }

    def build_lists(self, path):
        # create and fill hard photon
        ma.fillParticleList(decayString="pi+:eventShapeForSkims", cut="pt > 0.1", path=path)
        ma.fillParticleList(decayString="gamma:eventShapeForSkims", cut="E > 0.1", path=path)

        ma.buildEventShape(inputListNames=["pi+:eventShapeForSkims", "gamma:eventShapeForSkims"],
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
    __WorkingGroup__ = "Bottomonium"
    __SkimDescription__ = ""

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["loose"],
            "stdMu": ["loose"],
            "stdPi": ["loose"],
        },
        "stdPhotons": {
            "stdPhotons": ["loose"],
        },
    }

    def build_lists(self, path):
        Ycuts = ""
        # create and fill e/mu/pi/photon ParticleLists
        ma.fillParticleList("mu+:loose", "p<15 and p>3.5", path=path)
        ma.fillParticleList("e+:loose", "p<15 and p>3.5", path=path)
        ma.fillParticleList("pi+:loose", "p<1.5 and pt>0.05", path=path)
        ma.cutAndCopyList("gamma:soft", "gamma:loose", "E>0.15", path=path)

        # Y(1S,2S) are reconstructed with e^+ e^- or mu^+ mu^-
        ma.reconstructDecay("Upsilon:ee -> e+:loose e-:loose", "M > 8", path=path)
        ma.reconstructDecay("Upsilon:mumu -> mu+:loose mu-:loose", "M > 8", path=path)
        ma.copyLists("Upsilon:all", ["Upsilon:ee", "Upsilon:mumu"], path=path)

        # require foxWolframR2 < 0.995
        ma.fillParticleList(decayString="pi+:eventShapeForSkims", cut="pt > 0.1", path=path)
        ma.fillParticleList(decayString="gamma:eventShapeForSkims", cut="E > 0.1", path=path)

        ma.buildEventShape(inputListNames=["pi+:eventShapeForSkims", "gamma:eventShapeForSkims"],
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
        Upsilon_Channels = ["Upsilon:all pi+:loose",
                            "Upsilon:all gamma:soft"]

        # define the Y(1S,2S) decay channel list
        UpsilonList = []

        # reconstruct the decay channel
        for chID, channel in enumerate(Upsilon_Channels):
            ma.reconstructDecay("junction:all" + str(chID) + " -> " + channel, Ycuts, chID, path=path)
            UpsilonList.append("junction:all" + str(chID))

        # reture the list
        self.SkimLists = UpsilonList

    # TODO: Not sure how to implement validation_histograms, as BottomoniumUpsilon has
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
    __WorkingGroup__ = "Charmonium"
    __SkimDescription__ = ""

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all", "loose"],
            "stdK": ["all", "loose"],
            "stdMu": ["95eff", "all", "loose"],
            "stdPi": ["95eff", "all", "loose"],
        },
    }

    def build_lists(self, path):
        # intermediate state J/psi and psi(2S) are reconstructed
        # add mass window cut for J/psi and psi(2S) candidates
        ma.reconstructDecay("J/psi:ee -> e+:loose e-:loose", "M>3.0 and M<3.2", path=path)
        ma.reconstructDecay("J/psi:mumu -> mu+:loose mu-:loose", "M>3.0 and M<3.2", path=path)
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
            "pi+:loose pi-:loose J/psi:ee",
            "pi+:loose pi-:loose J/psi:mumu",
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

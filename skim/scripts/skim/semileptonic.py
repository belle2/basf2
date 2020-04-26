#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""(Semi-)Leptonic Working Group Skim list building functions for semi-leptonic analyses.
"""

__authors__ = [
    "Racha Cheaib",
    "Hannah Wakeling",
    "Phil Grace"
]

from basf2 import Path
import modularAnalysis as ma
<< << << < HEAD
from skimExpertFunctions import BaseSkim, fancy_skim_header
from validation_tools.metadata import create_validation_histograms
from variables import variables as vm
== == == =
from skimExpertFunctions import ifEventPasses
>>>>>> > master


def SemileptonicList(path):
    """
    Note:
        * **Skim description**: skim to be used initially for
          semileptonic :math:`B: decays (:math:`B \\to D \\ell\\nu,`
          where :math:`\\ell=e,\\mu`)
        * **Skim LFN code**: 11160200
        * **Working Group**: (Semi-)Leptonic and Missing Energy
          Working Group (WG1)

    Build leptonic untagged skim lists, and supply the names of the
    lists. Uses the standard electron and muon particle lists, so
    these must be added to the path first. Additionally, charm meson
    decays are defined by the charm list functions, and these require
    various photon, pion and kaon lists to be added to the path.

    Example usage:

    >>> from stdCharged import stdPi, stdK, stdE, stdMu
    >>> from stdPi0s import stdPi0s, stdPhotons
    >>> from stdV0s import stdKshorts
    >>> from skim.standardlists.charm import loadStdD0, loadStdDplus, loadStdDstar0, loadStdDstarPlus
    >>> from skim.semileptonic import SemileptonicList
    >>> stdE('all', path=path)
    >>> stdMu('all', path=path)
    >>> stdPi('all', path=path)
    >>> stdPi('loose', path=path)
    >>> stdPi0s('skim', path=path)
    >>> stdPi0s('loose', path=path)
    >>> stdPhotons('loose', path=path)
    >>> stdK('all', path=path)
    >>> stdKshorts(path=path)
    >>> loadStdD0(path)
    >>> loadStdDplus(path)
    >>> loadStdDstar0(path)
    >>> loadStdDstarPlus(path)
    >>> SemileptonicList(path)
    ['B+:SL1', 'B+:SL2', 'B+:SL3', 'B+:SL4', 'B0:SL1', 'B0:SL2', 'B0:SL3', 'B0:SL4']

    Reconstructed decays
        * :math:`B^+ \\to \\overline{D}^{0} e^+`
        * :math:`B^+ \\to \\overline{D}^{0} \\mu^+`
        * :math:`B^+ \\to \\overline{D}^{*0} e^+`
        * :math:`B^+ \\to \\overline{D}^{*0} \\mu^+`
        * :math:`B^0 \\to  D^{-} e^+`
        * :math:`B^0 \\to  D^{-} \\mu^+`
        * :math:`B^0 \\to  D^{*-} e^+`
        * :math:`B^0 \\to  D^{*-} \\mu^+`

    Cuts applied
        * :math:`p_{\\ell} > 0.35\\,\\text{GeV}`
        * :math:`5.24 < M_{\\text{bc}} < 5.29`
        * :math:`|\\Delta E | < 0.5`
        * :math:`n_{\\text{tracks}} > 4`

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        ``SLLists`` (list(str)): A list containing the names of the skim particle lists.
    """

    __authors__ = [
        "Phillip Urquijo",
        "Racha Cheaib"
    ]

    ma.cutAndCopyList('e+:SLUntagged', 'e+:all', 'p>0.35', True, path=path)
    ma.cutAndCopyList('mu+:SLUntagged', 'mu+:all', 'p>0.35', True, path=path)
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    BplusChannels = ['anti-D0:all e+:SLUntagged',
                     'anti-D0:all mu+:SLUntagged',
                     'anti-D*0:all e+:SLUntagged',
                     'anti-D*0:all mu+:SLUntagged'
                     ]

    B0Channels = ['D-:all e+:SLUntagged',
                  'D-:all mu+:SLUntagged',
                  'D*-:all e+:SLUntagged',
                  'D*-:all mu+:SLUntagged'
                  ]

    bplusList = []
    for chID, channel in enumerate(BplusChannels):
        ma.reconstructDecay(f"B+:SLUntagged_{chID} -> {channel}", Bcuts, chID, path=path)
        ma.applyCuts(f"B+:SLUntagged_{chID}", "nTracks>4", path=path)
        bplusList.append(f"B+:SLUntagged_{chID}")

    b0List = []
    for chID, channel in enumerate(B0Channels):
        ma.reconstructDecay(f"B0:SLUntagged_{chID} -> {channel}", Bcuts, chID, path=path)
        ma.applyCuts(f"B0:SLUntagged_{chID}", "nTracks>4", path=path)
        b0List.append(f"B0:SLUntagged_{chID}")

    SLLists = b0List + bplusList
    return SLLists


def PRList(path):
    """
    Note:
        * (Semi-)Leptonic Working Group skim for partial reconstruction analysis.
        * To be used initially for measuring B to D* l v using partial reconstruction.
        * Skim code: 11110100

    **Decay Modes**:

    * :math:`B^0 \\to \\pi^- e^+`
    * :math:`B^0 \\to \\pi^- \\mu^+`

    **Cuts applied**:

    Event-level cuts:

    * :math:`\\text{foxWolframR2} > 0.5` constructed using tracks with
      :math:`p_T>0.1\\,\\text{GeV}` and clusters with :math:`E>0.1\\,\\text{GeV}`.
    * :math:`n_{\\text{tracks}} > 4`

    Cuts on electrons:

    * :math:`\\text{electronID} > 0.5`
    * :math:`p > 1.5\\,\\text{GeV}` in CMS frame

    Cuts on muons:

    * :math:`\\text{muonID} > 0.5`
    * :math:`p > 1.5\\,\\text{GeV}` in CMS frame

    Cuts on pions:

    * :math:`\\text{pionID}>0.5`
    * :math:`\\text{muonID}<0.2`
    * :math:`0.060\\,\\text{GeV}<p<0.220\\,\\text{GeV}` in CMS frame

    Cuts on partially reconstructed :math:`B` mesons:

    * :math:`\\cos\\theta_{\\ell,\\,\\pi}<0` in CMS frame.
    """

    __authors__ = [
        "Lucien Cremaldi",
        "Racha Cheaib",
        "Romulus Godang"
    ]

    ma.fillParticleList(decayString='pi+:eventShapeForSkims',
                        cut='pt> 0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims',
                        cut='E > 0.1', path=path)

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

    path2 = Path()
    ifEventPasses('foxWolframR2<0.5 and nTracks>4', conditional_path=path2, path=path)

    ma.cutAndCopyList('e+:PRSemileptonic_1', 'e+:all', 'useCMSFrame(p) > 1.50 and electronID > 0.5', path=path2)
    ma.cutAndCopyList('mu+:PRSemileptonic_1', 'mu+:all', 'useCMSFrame(p) > 1.50 and muonID > 0.5', path=path2)
    ma.cutAndCopyList('pi-:PRSemileptonic_1', 'pi-:all',
                      'pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.220', path=path2)

    ma.cutAndCopyList('e+:PRSemileptonic_2', 'e+:all', '0.600 < useCMSFrame(p) <= 1.50 and electronID > 0.5', path=path2)
    ma.cutAndCopyList('mu+:PRSemileptonic_2', 'mu+:all', '0.350 < useCMSFrame(p) <= 1.50 and muonID > 0.5', path=path2)
    ma.cutAndCopyList('pi-:PRSemileptonic_2', 'pi-:all', 'pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.160', path=path2)

    ma.reconstructDecay('B0:PRSemileptonic_1 ->  pi-:PRSemileptonic_1 e+:PRSemileptonic_1',
                        'useCMSFrame(cos(daughterAngle(0,1)))<0.00', 1, path=path2)
    ma.reconstructDecay('B0:PRSemileptonic_2 ->  pi-:PRSemileptonic_1 mu+:PRSemileptonic_1',
                        'useCMSFrame(cos(daughterAngle(0,1)))<0.00', 2, path=path2)
    ma.reconstructDecay('B0:PRSemileptonic_3 ->  pi-:PRSemileptonic_2 e+:PRSemileptonic_2',
                        'useCMSFrame(cos(daughterAngle(0,1)))<1.00', 3, path=path2)
    ma.reconstructDecay('B0:PRSemileptonic_4 ->  pi-:PRSemileptonic_2 mu+:PRSemileptonic_2',
                        'useCMSFrame(cos(daughterAngle(0,1)))<1.00', 4, path=path2)

    PRList = ['B0:PRSemileptonic_1', 'B0:PRSemileptonic_2']
    return PRList, path2


@fancy_skim_header
class PRsemileptonicUntagged(BaseSkim):
    """
    Reconstructed decay modes:
    * B0:L1 ->  pi-:PR1 e+:PR1
    * B0:L2 ->  pi-:PR1 mu+:PR1

    Cuts applied:
    * electronID>0.5
    * muonID>0.5
    * lepton Momentum>1.5
    * foxWolframR2<0.5
    * nTracks>4
    """

    __authors__ = ["Lucien Cremaldi", "Racha Cheaib", "Romulus Godang"]
    __description__ = "Skim for partial reconstruction analysis in leptonic group."
    __contact__ = ""
    __category__ = "physics, semileptonic"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all"],
            "stdMu": ["all"],
            "stdPi": ["all"],
        },
    }

    def build_lists(self, path):
        ma.fillParticleList(decayString="pi+:eventShapeForSkims",
                            cut="pt> 0.1", path=path)
        ma.fillParticleList(decayString="gamma:eventShapeForSkims",
                            cut="E > 0.1", path=path)

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

        path = self.skim_event_cuts("foxWolframR2<0.5 and nTracks>4", path=path)

        ma.cutAndCopyList("e+:PR1", "e+:all", "useCMSFrame(p) > 1.50 and electronID > 0.5", path=path)
        ma.cutAndCopyList("mu+:PR1", "mu+:all", "useCMSFrame(p) > 1.50 and muonID > 0.5", path=path)
        ma.cutAndCopyList("pi-:PR1", "pi-:all", "pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.220", path=path)

        ma.cutAndCopyList("e+:PR2", "e+:all", "0.600 < useCMSFrame(p) <= 1.50 and electronID > 0.5", path=path)
        ma.cutAndCopyList("mu+:PR2", "mu+:all", "0.350 < useCMSFrame(p) <= 1.50 and muonID > 0.5", path=path)
        ma.cutAndCopyList("pi-:PR2", "pi-:all", "pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.160", path=path)

        ma.reconstructDecay("B0:L1 ->  pi-:PR1 e+:PR1", "useCMSFrame(cos(daughterAngle(0,1)))<0.00", 1, path=path)
        ma.reconstructDecay("B0:L2 ->  pi-:PR1 mu+:PR1", "useCMSFrame(cos(daughterAngle(0,1)))<0.00", 2, path=path)
        ma.reconstructDecay("B0:L3 ->  pi-:PR2 e+:PR2", "useCMSFrame(cos(daughterAngle(0,1)))<1.00", 3, path=path)
        ma.reconstructDecay("B0:L4 ->  pi-:PR2 mu+:PR2", "useCMSFrame(cos(daughterAngle(0,1)))<1.00", 4, path=path)

        self.SkimLists = ["B0:L1", "B0:L2"]

    def validation_histograms(self, path):
        ma.cutAndCopyLists("B0:PRsemileptonic", ["B0:L1", "B0:L2"], "", path=path)

        ma.buildRestOfEvent("B0:PRsemileptonic", path=path)
        ma.appendROEMask("B0:PRsemileptonic", "basic",
                         "pt>0.05 and -2<dr<2 and -4.0<dz<4.0",
                         "E>0.05",
                         path=path)
        ma.buildContinuumSuppression("B0:PRsemileptonic", "basic", path=path)

        vm.addAlias("d0_p", "daughter(0, p)")
        vm.addAlias("d1_p", "daughter(1, p)")
        vm.addAlias("MissM2", "weMissM2(basic,0)")

        histogramFilename = f"{self}_Validation.root"
        email = "Phil Grace <philip.grace@adelaide.edu.au>"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist="B0:PRsemileptonic",
            variables_1d=[
                ("Mbc", 100, 4.0, 5.3, "Mbc", email, "", ""),
                ("d0_p", 100, 0, 5.2, "Signal-side pion momentum", email, "", ""),
                ("d1_p", 100, 0, 5.2, "Signal-side lepton momentum", email, "", ""),
                ("MissM2", 100, -5, 5, "Missing mass squared", email, "", "")
            ],
            variables_2d=[("deltaE", 100, -5, 5, "Mbc", 100, 4.0, 5.3, "Mbc vs deltaE", email, "", "")],
            path=path)


@fancy_skim_header
class SLUntagged(BaseSkim):
    """
    Cuts applied:
    * :math:`p_{\\ell} > 0.35\\,\\text{GeV}`
    * :math:`5.24 < M_{\\text{bc}} < 5.29`
    * :math:`|\\Delta E | < 0.5`
    * :math:`n_{\\text{tracks}} > 4`

    Reconstructed decays:
    * :math:`B^+ \\to \\overline{D}^{0} e^+`
    * :math:`B^+ \\to \\overline{D}^{0} \\mu^+`
    * :math:`B^+ \\to \\overline{D}^{*0} e^+`
    * :math:`B^+ \\to \\overline{D}^{*0} \\mu^+`
    * :math:`B^0 \\to  D^{-} e^+`
    * :math:`B^0 \\to  D^{-} \\mu^+`
    * :math:`B^0 \\to  D^{*-} e^+`
    * :math:`B^0 \\to  D^{*-} \\mu^+`
    """

    __authors__ = ["Phillip Urquijo", "Racha Cheaib"]
    __description__ = (
        "Skim for semileptonic decays, :math:`B: decays "
        "(:math:`B \\to D \\ell\\nu,` where :math:`\\ell=e,\\mu`)"
    )
    __contact__ = ""
    __category__ = "physics, semileptonic"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all"],
            "stdK": ["all"],
            "stdMu": ["all"],
            "stdPi": ["all", "loose"],
        },
        "stdPhotons": {
            "stdPhotons": ["loose"],
        },
        "stdPi0s": {
            "stdPi0s": ["loose"],
        },
        "stdV0s": {
            "stdKshorts": [],
        },
        "skim.standardlists.lightmesons": {
            "loadStdPi0ForBToHadrons": [],
        },
        "skim.standardlists.charm": {
            "loadStdD0": [],
            "loadStdDstar0": [],
            "loadStdDplus": [],
            "loadStdDstarPlus": [],
        },
    }

    def build_lists(self, path):
        ma.cutAndCopyList("e+:SLB", "e+:all", "p>0.35", True, path=path)
        ma.cutAndCopyList("mu+:SLB", "mu+:all", "p>0.35", True, path=path)
        Bcuts = "5.24 < Mbc < 5.29 and abs(deltaE) < 0.5"

        BplusChannels = ["anti-D0:all e+:SLB",
                         "anti-D0:all mu+:SLB",
                         "anti-D*0:all e+:SLB",
                         "anti-D*0:all mu+:SLB"
                         ]

        B0Channels = ["D-:all e+:SLB",
                      "D-:all mu+:SLB",
                      "D*-:all e+:SLB",
                      "D*-:all mu+:SLB"
                      ]

        bplusList = []
        for chID, channel in enumerate(BplusChannels):
            ma.reconstructDecay("B+:SL" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            ma.applyCuts("B+:SL" + str(chID), "nTracks>4", path=path)
            bplusList.append("B+:SL" + str(chID))

        b0List = []
        for chID, channel in enumerate(B0Channels):
            ma.reconstructDecay("B0:SL" + str(chID) + " -> " + channel, Bcuts, chID, path=path)
            ma.applyCuts("B+:SL" + str(chID), "nTracks>4", path=path)
            b0List.append("B0:SL" + str(chID))

        self.SkimLists = b0List + bplusList

    def validation_histograms(self, path):
        ma.cutAndCopyLists("B+:SLUntagged", ["B+:SL0", "B+:SL1", "B+:SL2", "B+:SL3"], "", path=path)

        ma.buildRestOfEvent("B+:SLUntagged", path=path)
        ma.appendROEMask("B+:SLUntagged", "basic",
                         "pt>0.05 and -2<dr<2 and -4.0<dz<4.0",
                         "E>0.05",
                         path=path)
        ma.buildContinuumSuppression("B+:SLUntagged", "basic", path=path)

        vm.addAlias("d1_p", "daughter(1,p)")
        vm.addAlias("MissM2", "weMissM2(basic,0)")

        histogramFilename = f"{self}_Validation.root"
        myEmail = "Phil Grace <philip.grace@adelaide.edu.au>"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist="B+:SLUntagged",
            variables_1d=[
                ("cosThetaBetweenParticleAndNominalB", 100, -6.0, 4.0, "cosThetaBY", myEmail, "", ""),
                ("Mbc", 100, 4.0, 5.3, "Mbc", myEmail, "", ""),
                ("d1_p", 100, 0, 5.2, "Signal-side lepton momentum", myEmail, "", ""),
                ("MissM2", 100, -5, 5, "Missing mass squared", myEmail, "", "")
            ],
            variables_2d=[("deltaE", 100, -5, 5, "Mbc", 100, 4.0, 5.3, "Mbc vs deltaE", myEmail, "", "")],
            path=path)

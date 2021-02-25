#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""(Semi-)Leptonic Working Group Skim list building functions for semi-leptonic analyses.
"""

__authors__ = [
    "Racha Cheaib",
    "Hannah Wakeling",
    "Phil Grace"
]

import modularAnalysis as ma
from skim.standardlists.charm import (loadKForBtoHadrons, loadPiForBtoHadrons,
                                      loadStdD0, loadStdDplus, loadStdDstar0,
                                      loadStdDstarPlus)
from skim.standardlists.lightmesons import loadStdPi0ForBToHadrons
from skimExpertFunctions import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s
from stdV0s import stdKshorts

__liaison__ = "Shanette De La Motte <shanette.delamotte@adelaide.edu.au>"


@fancy_skim_header
class PRsemileptonicUntagged(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^0 \\to \\pi^- e^+`
    * :math:`B^0 \\to \\pi^- \\mu^+`

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

    __authors__ = ["Lucien Cremaldi", "Racha Cheaib", "Romulus Godang"]
    __description__ = "Skim for partial reconstruction analysis in leptonic group."
    __contact__ = __liaison__
    __category__ = "physics, semileptonic"

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)

    def build_lists(self, path):
        ma.fillParticleList(decayString="pi+:PRSL_eventshape",
                            cut="pt> 0.1", path=path)
        ma.fillParticleList(decayString="gamma:PRSL_eventshape",
                            cut="E > 0.1", path=path)

        ma.buildEventShape(inputListNames=["pi+:PRSL_eventshape", "gamma:PRSL_eventshape"],
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

        ma.cutAndCopyList("e+:PRSemileptonic_1", "e+:all",
                          "useCMSFrame(p) > 1.50 and electronID > 0.5", path=path)
        ma.cutAndCopyList("mu+:PRSemileptonic_1", "mu+:all",
                          "useCMSFrame(p) > 1.50 and muonID > 0.5", path=path)
        ma.cutAndCopyList("pi-:PRSemileptonic_1", "pi-:all",
                          "pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.220", path=path)

        ma.cutAndCopyList("e+:PRSemileptonic_2", "e+:all",
                          "0.600 < useCMSFrame(p) <= 1.50 and electronID > 0.5", path=path)
        ma.cutAndCopyList("mu+:PRSemileptonic_2", "mu+:all",
                          "0.350 < useCMSFrame(p) <= 1.50 and muonID > 0.5", path=path)
        ma.cutAndCopyList("pi-:PRSemileptonic_2", "pi-:all",
                          "pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.160", path=path)

        ma.reconstructDecay("B0:PRSemileptonic_1 ->  pi-:PRSemileptonic_1 e+:PRSemileptonic_1",
                            "useCMSFrame(cos(daughterAngle(0,1)))<0.00", 1, path=path)
        ma.reconstructDecay("B0:PRSemileptonic_2 ->  pi-:PRSemileptonic_1 mu+:PRSemileptonic_1",
                            "useCMSFrame(cos(daughterAngle(0,1)))<0.00", 2, path=path)
        ma.reconstructDecay("B0:PRSemileptonic_3 ->  pi-:PRSemileptonic_2 e+:PRSemileptonic_2",
                            "useCMSFrame(cos(daughterAngle(0,1)))<1.00", 3, path=path)
        ma.reconstructDecay("B0:PRSemileptonic_4 ->  pi-:PRSemileptonic_2 mu+:PRSemileptonic_2",
                            "useCMSFrame(cos(daughterAngle(0,1)))<1.00", 4, path=path)

        self.SkimLists = ["B0:PRSemileptonic_1", "B0:PRSemileptonic_2"]


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
    __contact__ = __liaison__
    __category__ = "physics, semileptonic"

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdK("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdPi("loose", path=path)
        stdPhotons("loose", path=path)
        stdPi0s("eff40_Jan2020", path=path)
        stdKshorts(path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0(path=path)
        loadStdDstar0(path=path)
        loadStdDplus(path=path)
        loadStdDstarPlus(path=path)

    def build_lists(self, path):
        ma.cutAndCopyList("e+:SLUntagged", "e+:all", "p>0.35", True, path=path)
        ma.cutAndCopyList("mu+:SLUntagged", "mu+:all", "p>0.35", True, path=path)
        Bcuts = "5.24 < Mbc < 5.29 and abs(deltaE) < 0.5"

        BplusChannels = ["anti-D0:all e+:SLUntagged",
                         "anti-D0:all mu+:SLUntagged",
                         "anti-D*0:all e+:SLUntagged",
                         "anti-D*0:all mu+:SLUntagged"
                         ]

        B0Channels = ["D-:all e+:SLUntagged",
                      "D-:all mu+:SLUntagged",
                      "D*-:all e+:SLUntagged",
                      "D*-:all mu+:SLUntagged"
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

        self.SkimLists = b0List + bplusList

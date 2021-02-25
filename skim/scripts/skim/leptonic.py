#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""(Semi-)Leptonic Working Group Skims for leptonic analyses.
"""

__authors__ = [
    "Sophie Hollit",
    "Racha Cheaib",
    "Hannah Wakeling",
    "Phil Grace"
]

import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdMu

__liaison__ = "Shanette De La Motte <shanette.delamotte@adelaide.edu.au>"


@fancy_skim_header
class LeptonicUntagged(BaseSkim):
    """
    Reconstructed decays
        * :math:`B^- \\to e^-`
        * :math:`B^- \\to \\mu^-`

    Cuts applied
        * :math:`p_{\\ell}^{*} > 2\\,\\text{GeV}` in CMS Frame
        * :math:`\\text{electronID} > 0.5`
        * :math:`\\text{muonID} > 0.5`
        * :math:`n_{\\text{tracks}} \\geq 3`
    """

    __authors__ = ["Phillip Urquijo"]
    __contact__ = __liaison__
    __description__ = (
        "Skim for leptonic analyses, "
        ":math:`B_{\\text{sig}}^-\\to\\ell\\nu`, where :math:`\\ell=e,\\mu`"
    )
    __category__ = "physics, leptonic"

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)

    def build_lists(self, path):
        ma.cutAndCopyList(
            "e-:LeptonicUntagged",
            "e-:all",
            "useCMSFrame(p) > 2.0 and electronID > 0.5",
            True,
            path=path,
        )
        ma.cutAndCopyList(
            "mu-:LeptonicUntagged",
            "mu-:all",
            "useCMSFrame(p) > 2.0 and muonID > 0.5",
            True,
            path=path,
        )
        ma.reconstructDecay("B-:LeptonicUntagged_0 -> e-:LeptonicUntagged", "", 1, path=path)
        ma.reconstructDecay("B-:LeptonicUntagged_1 -> mu-:LeptonicUntagged", "", 2, path=path)
        ma.applyCuts("B-:LeptonicUntagged_0", "nTracks>=3", path=path)
        ma.applyCuts("B-:LeptonicUntagged_1", "nTracks>=3", path=path)
        lepList = ["B-:LeptonicUntagged_0", "B-:LeptonicUntagged_1"]
        self.SkimLists = lepList


@fancy_skim_header
class dilepton(BaseSkim):
    """
    Reconstructed decays
        * :math:`BBar \\to l^+l^-`
        * :math:`BBar \\to l^+l^+`
        * :math:`BBar \\to l^-l^-`
    """
    __authors__ = ["Alessandro Gaz, Chiara La Licata"]
    __contact__ = __liaison__
    __description__ = (
        "Inclusive dilepton skim"
    )
    __category__ = "physics, leptonic"

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)

    def build_lists(self, path):
        ma.cutAndCopyList(
            "e+:pid",
            "e+:all",
            "abs(d0) < 1 and abs(z0) < 4 and p > 1.2 and electronID > 0.5",
            True,
            path=path,
        )
        ma.cutAndCopyList(
            "mu+:pid",
            "mu+:all",
            "abs(d0) < 1 and abs(z0) < 4 and p > 1.2 and muonID > 0.5",
            True,
            path=path,
        )

        ma.buildEventShape(
            inputListNames=[],
            default_cleanup=True,
            allMoments=False,
            cleoCones=True,
            collisionAxis=True,
            foxWolfram=True,
            harmonicMoments=True,
            jets=True,
            sphericity=True,
            thrust=True,
            checkForDuplicates=False,
            path=path)

        ma.applyEventCuts(cut='foxWolframR2 < 0.5 and nTracks > 3', path=path)

        ma.reconstructDecay('Upsilon(4S):ee   -> e+:pid e-:pid',   'M < 15', path=path)
        ma.reconstructDecay('Upsilon(4S):emu  -> e+:pid mu-:pid',  'M < 15', path=path)
        ma.reconstructDecay('Upsilon(4S):mumu -> mu+:pid mu-:pid', 'M < 15', path=path)

        ma.reconstructDecay('Delta++:ee   -> e+:pid e+:pid',   'M < 15', path=path)
        ma.reconstructDecay('Delta++:emu  -> e+:pid mu+:pid',  'M < 15', path=path)
        ma.reconstructDecay('Delta++:mumu -> mu+:pid mu+:pid', 'M < 15', path=path)

        ma.copyLists(outputListName='Upsilon(4S):ll',
                     inputListNames=['Upsilon(4S):ee', 'Upsilon(4S):emu', 'Upsilon(4S):mumu'],
                     path=path)

        ma.copyLists(outputListName='Delta++:ll',
                     inputListNames=['Delta++:ee', 'Delta++:emu', 'Delta++:mumu'],
                     path=path)

        dileptonList = ["Upsilon(4S):ll", "Delta++:ll"]
        self.SkimLists = dileptonList

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
from variables import variables as vm
from validation_tools.metadata import create_validation_histograms


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

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all"],
            "stdMu": ["all"]
        }
    }

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

    def validation_histograms(self, path):
        ma.cutAndCopyLists("B-:LeptonicUntagged", ["B-:LeptonicUntagged_0", "B-:LeptonicUntagged_1"], "", path=path)

        ma.buildRestOfEvent("B-:LeptonicUntagged", path=path)
        ma.appendROEMask(
            "B-:LeptonicUntagged",
            "basic",
            "pt>0.05 and -2<dr<2 and -4.0<dz<4.0",
            "E>0.05",
            path=path,
        )
        vm.addAlias("d0_p", "daughter(0,p)")
        vm.addAlias("d0_electronID", "daughter(0,electronID)")
        vm.addAlias("d0_muonID", "daughter(0,muonID)")
        vm.addAlias("MissP", "weMissP(basic,0)")

        histogramFilename = "LeptonicUntagged_Validation.root"
        contact = "Phil Grace <philip.grace@adelaide.edu.au>"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist="B-:LeptonicUntagged",
            variables_1d=[
                ("Mbc", 100, 4.0, 5.3, "Mbc", contact, "", ""),
                ("d0_p", 100, 0, 5.2, "Signal-side lepton momentum", contact, "", ""),
                ("d0_electronID", 100, 0, 1, "electronID of signal-side lepton",
                 contact, "", ""),
                ("d0_muonID", 100, 0, 1, "electronID of signal-side lepton", contact,
                 "", ""),
                ("R2", 100, 0, 1, "R2", contact, "", ""),
                ("MissP", 100, 0, 5.3, "Missing momentum of event (CMS frame)", contact,
                 "", ""),
            ],
            variables_2d=[
                ("deltaE", 100, -5, 5, "Mbc", 100, 4.0, 5.3, "Mbc vs deltaE", contact,
                 "", "")
            ],
            path=path,
        )

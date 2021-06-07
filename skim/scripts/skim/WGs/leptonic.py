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
from skim import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdMu
from variables import variables as vm

__liaison__ = "Shanette De La Motte <shanette.delamotte@adelaide.edu.au>"
_VALIDATION_SAMPLE = "mdst14.root"


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

    validation_sample = _VALIDATION_SAMPLE

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

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

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

        histogramFilename = f"{self}_Validation.root"
        contact = __liaison__

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

        path = self.skim_event_cuts('foxWolframR2 < 0.5 and nTracks > 3', path=path)

        ma.reconstructDecay('Upsilon(4S):ee   -> e+:pid e-:pid', 'M < 15', path=path)
        ma.reconstructDecay('Upsilon(4S):emu  -> e+:pid mu-:pid', 'M < 15', path=path)
        ma.reconstructDecay('Upsilon(4S):mumu -> mu+:pid mu-:pid', 'M < 15', path=path)

        ma.reconstructDecay('Delta++:ee   -> e+:pid e+:pid', 'M < 15', path=path)
        ma.reconstructDecay('Delta++:emu  -> e+:pid mu+:pid', 'M < 15', path=path)
        ma.reconstructDecay('Delta++:mumu -> mu+:pid mu+:pid', 'M < 15', path=path)

        ma.copyLists(outputListName='Upsilon(4S):ll',
                     inputListNames=['Upsilon(4S):ee', 'Upsilon(4S):emu', 'Upsilon(4S):mumu'],
                     path=path)

        ma.copyLists(outputListName='Delta++:ll',
                     inputListNames=['Delta++:ee', 'Delta++:emu', 'Delta++:mumu'],
                     path=path)

        dileptonList = ["Upsilon(4S):ll", "Delta++:ll"]
        self.SkimLists = dileptonList

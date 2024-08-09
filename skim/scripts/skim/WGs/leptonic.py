#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""(Semi-)Leptonic Working Group Skims for leptonic analyses.
"""

import modularAnalysis as ma
from skim import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdMu
from variables import variables as vm
import basf2 as b2
from ROOT import Belle2

__liaison__ = "Cameron Harris <cameron.harris@adelaide.edu.au>, Tommy Martinov <tommy.martinov@desy.de>"
_VALIDATION_SAMPLE = "mdst14.root"


@fancy_skim_header
class LeptonicUntagged(BaseSkim):
    """
    Reconstructed decays
        * :math:`B^- \\to e^-`
        * :math:`B^- \\to \\mu^-`

    Cuts applied
        * :math:`p_{\\ell}^{*} > 1.8\\,\\text{GeV}` in CMS Frame
        * :math:`\\text{pidChargedBDTScore_e} > 0.9`
        * :math:`\\text{muonID_noSVD} > 0.9`
        * :math:`n_{\\text{tracks}} \\geq 3`
    """

    __authors__ = ["Daniel Jacobi"]
    __contact__ = __liaison__
    __description__ = (
        "Skim for leptonic analyses, "
        ":math:`B_{\\text{sig}}^-\\to\\ell\\nu`, where :math:`\\ell=e,\\mu`"
    )
    __category__ = "physics, leptonic"

    validation_sample = _VALIDATION_SAMPLE
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)

    def build_lists(self, path):
        b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())
        ma.applyChargedPidMVA(
            particleLists=['e-:all'],
            path=path,
            trainingMode=Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_Multiclass)
        ma.cutAndCopyList(
            "e-:LeptonicUntagged",
            "e-:all",
            "useCMSFrame(p) > 1.8 and pidChargedBDTScore(11, ALL) > 0.9",
            True,
            path=path,
        )
        ma.cutAndCopyList(
            "mu-:LeptonicUntagged",
            "mu-:all",
            "useCMSFrame(p) > 1.8 and muonID_noSVD > 0.9",
            True,
            path=path,
        )

        ma.reconstructDecay("B-:LeptonicUntagged_0 -> e-:LeptonicUntagged", "", 1, path=path)
        ma.reconstructDecay("B-:LeptonicUntagged_1 -> mu-:LeptonicUntagged", "", 2, path=path)
        ma.applyCuts("B-:LeptonicUntagged_0", "nTracks>=3", path=path)
        ma.applyCuts("B-:LeptonicUntagged_1", "nTracks>=3", path=path)
        return ["B-:LeptonicUntagged_0", "B-:LeptonicUntagged_1"]

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
        vm.addAlias("d0_pidChargedBDTScore_e", "daughter(0,pidChargedBDTScore(11, ALL))")
        vm.addAlias("d0_muonID_noSVD", "daughter(0,muonID_noSVD)")
        vm.addAlias("MissP", "weMissP(basic,0)")

        histogramFilename = f"{self}_Validation.root"
        contact = __liaison__

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist="B-:LeptonicUntagged",
            variables_1d=[
                ("Mbc", 100, 4.0, 5.3, "Mbc", contact, "", ""),
                ("d0_p", 100, 0, 5.2, "Signal-side lepton momentum", contact, "", ""),
                ("d0_pidChargedBDTScore_e", 100, 0, 1, "pidChargedBDTScore_e of signal-side lepton",
                 contact, "", ""),
                ("d0_muonID_noSVD", 100, 0, 1, "muonID_noSVD of signal-side lepton", contact,
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

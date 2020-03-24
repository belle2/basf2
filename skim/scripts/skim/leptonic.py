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
from skimExpertFunctions import BaseSkim


def LeptonicList(path):
    """
    Note:
        * **Skim description**: Skim for leptonic analyses,
          :math:`B_{\\text{sig}}^-\\to\\ell\\nu`, where :math:`\\ell=e,\\mu`.
        * **Skim LFN code**: 11130300
        * **Working Group**: (Semi-)Leptonic and Missing Energy
          Working Group (WG1)

    Build leptonic untagged skim lists, and supply the name of the
    lists. Uses the standard electron and muon particle lists, so
    these must be added to the path first.

    Example usage:

    >>> from stdCharged import stdE, stdMu
    >>> from skim.leptonic import LeptonicList
    >>> stdE('all', path=path)
    >>> stdMu('all', path=path)
    >>> LeptonicList(path)
    ['B-:L0', 'B-:L1']

    Reconstructed decays
        * :math:`B^- \\to e^-`
        * :math:`B^- \\to \\mu^-`

    Cuts applied
        * :math:`p_{\\ell}^{*} > 2\\,\\text{GeV}` in CMS Frame
        * :math:`\\text{electronID} > 0.5`
        * :math:`\\text{muonID} > 0.5`
        * :math:`n_{\\text{tracks}} \geq 3`

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        ``lepList``, a Python list containing the strings
        :code:`'B-:L0'` and :code:`'B-:L1'`, the names of the particle
        lists for leptonic :math:`B^-` skim candidates.
    """
    __authors__ = [
        "Phillip Urquijo"
    ]

    ma.cutAndCopyList('e-:highP', 'e-:all', 'useCMSFrame(p) > 2.0 and electronID > 0.5', True, path=path)
    ma.cutAndCopyList('mu-:highP', 'mu-:all', 'useCMSFrame(p) > 2.0 and muonID > 0.5', True, path=path)
    ma.reconstructDecay('B-:L0 -> e-:highP', '', 1, path=path)
    ma.reconstructDecay('B-:L1 -> mu-:highP', '', 2, path=path)
    ma.applyCuts('B-:L0', 'nTracks>=3', path=path)
    ma.applyCuts('B-:L1', 'nTracks>=3', path=path)
    lepList = ['B-:L0', 'B-:L1']
    return lepList


class LeptonicUntagged(BaseSkim):
    """
    Note:
        * **Skim description**: Skim for leptonic analyses,
          :math:`B_{\\text{sig}}^-\\to\\ell\\nu`, where :math:`\\ell=e,\\mu`.
        * **Skim LFN code**: 11130300
        * **Working Group**: (Semi-)Leptonic and Missing Energy
          Working Group (WG1)

    Reconstructed decays
        * :math:`B^- \\to e^-`
        * :math:`B^- \\to \\mu^-`

    Cuts applied
        * :math:`p_{\\ell}^{*} > 2\\,\\text{GeV}` in CMS Frame
        * :math:`\\text{electronID} > 0.5`
        * :math:`\\text{muonID} > 0.5`
        * :math:`n_{\\text{tracks}} \\geq 3`
    """
    lists = ["B-:L0", "B-:L1"]

    def setup(self, path):
        from stdCharged import stdE, stdMu

        stdE("all", path=path)
        stdMu("all", path=path)

    def build_lists(self, path):
        ma.cutAndCopyList(
            "e-:highP",
            "e-:all",
            "useCMSFrame(p) > 2.0 and electronID > 0.5",
            True,
            path=path,
        )
        ma.cutAndCopyList(
            "mu-:highP",
            "mu-:all",
            "useCMSFrame(p) > 2.0 and muonID > 0.5",
            True,
            path=path,
        )
        ma.reconstructDecay("B-:L0 -> e-:highP", "", 1, path=path)
        ma.reconstructDecay("B-:L1 -> mu-:highP", "", 2, path=path)
        ma.applyCuts("B-:L0", "nTracks>=3", path=path)
        ma.applyCuts("B-:L1", "nTracks>=3", path=path)
        lepList = ["B-:L0", "B-:L1"]
        self.lists = lepList

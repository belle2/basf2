#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""(Semi-)Leptonic Working Group Skims for leptonic analyses.
"""

__authors__ = [
    "Sophie Hollit",
    "Racha Cheaib",
    "Hannah Wakeling"
]

from basf2 import *
from modularAnalysis import *


def LeptonicList(path):
    """
    Note:
        * (Semi-)Leptonic Working Group Skim list building functions for leptonic analyses.
        * To be used initially for B leptonic decays (B to l v) (l=electron, muon)
        * Skim code: 11130300

    **Decay Modes**:

    * B- -> e-
    * B- -> mu-

    **Cuts applied**:

    * lepton momentum in CMS Frame > 2 GeV
    * electronID > 0.5
    * muonID > 0.5
    * nTracks > 4
    """
    __authors__ = [
        "Phillip Urquijo"
    ]

    cutAndCopyList('e-:highP', 'e-:all', 'useCMSFrame(p) > 2.0 and electronID > 0.5', True, path=path)
    cutAndCopyList('mu-:highP', 'mu-:all', 'useCMSFrame(p) > 2.0 and muonID > 0.5', True, path=path)
    reconstructDecay('B-:L0 -> e-:highP', '', 1, path=path)
    reconstructDecay('B-:L1 -> mu-:highP', '', 2, path=path)
    applyCuts('B-:L0', 'nTracks>4', path=path)
    applyCuts('B-:L1', 'nTracks>4', path=path)
    lepList = ['B-:L0', 'B-:L1']
    return lepList

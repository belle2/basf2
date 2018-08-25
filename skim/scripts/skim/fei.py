#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Skims for missing energy modes that use the `FullEventInterpretation` (FEI) algorithm.
"""

__authors__ = [
    "Sophie Hollit",
    "Racha Cheaib"
]

from basf2 import *
from modularAnalysis import *


from variables import *
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')


def B0hadronic():
    """FEI Hadronic B0 skim list"""
    # Reconstruct tag side
    # Apply cuts
    applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')

    B0hadronicList = ['B0:generic']
    return B0hadronicList

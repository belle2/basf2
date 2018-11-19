#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""(Semi-)Leptonic Working Group Skim list building functions for semi-leptonic analyses.
"""

__authors__ = [
    "Racha Cheaib",
    "Hannah Wakeling"
]

from basf2 import *
from modularAnalysis import *


def SemileptonicList(path):
    """
    Note:
        * (Semi-)Leptonic Working Group skim for semi-leptonic analysis.
        * To be used initially for for B semileptonic decays (B to D l v) (l= electron, muon)
        * Skim code: 11160200
        * Uses D:all lists

    **Decay Modes**:
        *B+ -> D0 e+
        *B+ -> D0 mu+
        *B+ -> D*0 e+
        *B+ -> D*0 mu+
        *B0 ->  D+ e-
        *B0 ->  D+ mu-
        *B0 ->  D*+ e-
        *B0 ->  D*+ mu-

    **Cuts applied**:

    * lepton momentum > 0.35 GeV
    * 5.24 < B_Mbc < 5.29
    * | deltaE | < 0.5
    * nTracks > 4
    """

    __authors__ = [
        "Phillip Urquijo",
        "Racha Cheaib"
    ]

    cutAndCopyList('e-:SLB', 'e-:all', 'p>0.35', True, path=path)
    cutAndCopyList('mu-:SLB', 'mu-:all', 'p>0.35', True, path=path)
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    BplusChannels = ['D0:all e+:SLB',
                     'D0:all mu+:SLB',
                     'D*0:all e+:SLB',
                     'D*0:all mu+:SLB'
                     ]

    B0Channels = ['D+:all e-:SLB',
                  'D+:all mu-:SLB',
                  'D*+:all e-:SLB',
                  'D*+:all mu-:SLB'
                  ]

    bplusList = []
    for chID, channel in enumerate(BplusChannels):
        reconstructDecay('B+:SL' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        applyCuts('B+:SL' + str(chID), 'nTracks>4', path=path)
        bplusList.append('B+:SL' + str(chID))

    b0List = []
    for chID, channel in enumerate(B0Channels):
        reconstructDecay('B0:SL' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        applyCuts('B+:SL' + str(chID), 'nTracks>4', path=path)
        b0List.append('B0:SL' + str(chID))

    allLists = b0List + bplusList
    return allLists


def PRList(path):
    """
    Note:
        * (Semi-)Leptonic Working Group skim for partial reconstruction analysis.
        * To be used initially for measuring B to D* l v using partial reconstruction.
        * Skim code: 11110100

    **Decay Modes**:

    * B0:L1 ->  pi-:PR1 e+:PR1
    * B0:L2 ->  pi-:PR1 mu+:PR1

    **Cuts applied**:

    * electronID>0.5
    * muonID>0.5
    * lepton Momentum>1.5
    * R2EventLevel<0.5
    * nTracks>4
    """

    __authors__ = [
        "Lucien Cremaldi",
        "Racha Cheaib",
        "Romulus Godang"
    ]

    cutAndCopyList('e+:PR1', 'e+:all', 'useCMSFrame(p) > 1.50 and electronID > 0.5', path=path)
    cutAndCopyList('mu+:PR1', 'mu+:all', 'useCMSFrame(p) > 1.50 and muonID > 0.5', path=path)
    cutAndCopyList('pi-:PR1', 'pi-:all', 'pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.220', path=path)

    cutAndCopyList('e+:PR2', 'e+:all', '0.600 < useCMSFrame(p) <= 1.50 and electronID > 0.5', path=path)
    cutAndCopyList('mu+:PR2', 'mu+:all', '0.350 < useCMSFrame(p) <= 1.50 and muonID > 0.5', path=path)
    cutAndCopyList('pi-:PR2', 'pi-:all', 'pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.160', path=path)

    reconstructDecay('B0:L1 ->  pi-:PR1 e+:PR1', 'useCMSFrame(daughterAngle(0,1))<0.00', 1, path=path)
    applyCuts('B0:L1', 'R2EventLevel<0.5 and nTracks>4', path=path)

    reconstructDecay('B0:L2 ->  pi-:PR1 mu+:PR1', 'useCMSFrame(daughterAngle(0,1))<0.00', 2, path=path)
    applyCuts('B0:L2', 'R2EventLevel<0.5 and nTracks>4', path=path)

    reconstructDecay('B0:L3 ->  pi-:PR2 e+:PR2', 'useCMSFrame(daughterAngle(0,1))<1.00', 3, path=path)
    applyCuts('B0:L3', 'R2EventLevel<0.5 and nTracks>4', path=path)

    reconstructDecay('B0:L4 ->  pi-:PR2 mu+:PR2', 'useCMSFrame(daughterAngle(0,1))<1.00', 4, path=path)
    applyCuts('B0:L4', 'R2EventLevel<0.5 and nTracks>4', path=path)

    PRList = ['B0:L1', 'B0:L2']

    return PRList
